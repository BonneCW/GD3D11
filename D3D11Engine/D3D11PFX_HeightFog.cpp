#include "pch.h"
#include "D3D11PFX_HeightFog.h"
#include "Engine.h"
#include "D3D11GraphicsEngine.h"
#include "D3D11PfxRenderer.h"
#include "RenderToTextureBuffer.h"
#include "D3D11ShaderManager.h"
#include "D3D11VShader.h"
#include "D3D11PShader.h"
#include "D3D11ConstantBuffer.h"
#include "ConstantBufferStructs.h"
#include "GothicAPI.h"
#include "GSky.h"

using namespace DirectX;

D3D11PFX_HeightFog::D3D11PFX_HeightFog( D3D11PfxRenderer* rnd ) : D3D11PFX_Effect( rnd ) {}


D3D11PFX_HeightFog::~D3D11PFX_HeightFog() {}

/** Draws this effect to the given buffer */
XRESULT D3D11PFX_HeightFog::Render( RenderToTextureBuffer* fxbuffer ) {
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine*)Engine::GraphicsEngine;

	// Save old rendertargets
	ID3D11RenderTargetView* oldRTV = nullptr;
	ID3D11DepthStencilView* oldDSV = nullptr;
	engine->GetContext()->OMGetRenderTargets( 1, &oldRTV, &oldDSV );

	D3D11VShader* vs = engine->GetShaderManager()->GetVShader( "VS_PFX" );
	D3D11PShader* hfPS = engine->GetShaderManager()->GetPShader( "PS_PFX_Heightfog" );

	hfPS->Apply();
	vs->Apply();

	HeightfogConstantBuffer cb;
	XMMATRIX xmInvProj;
	xmInvProj = XMLoadFloat4x4( &Engine::GAPI->GetProjectionMatrix() );
	xmInvProj = XMMatrixInverse( nullptr, xmInvProj );
	XMStoreFloat4x4( &cb.InvProj, xmInvProj );

	XMMATRIX xmInvView = XMMatrixInverse( nullptr, Engine::GAPI->GetViewMatrixXM() );
	XMStoreFloat4x4( &cb.InvView, xmInvView );

	cb.CameraPosition = Engine::GAPI->GetCameraPosition();
	float NearPlane = Engine::GAPI->GetRendererState()->RendererInfo.NearPlane;
	float FarPlane = Engine::GAPI->GetRendererState()->RendererInfo.FarPlane;

	cb.HF_GlobalDensity = Engine::GAPI->GetRendererState()->RendererSettings.FogGlobalDensity;
	cb.HF_HeightFalloff = Engine::GAPI->GetRendererState()->RendererSettings.FogHeightFalloff;

	float height = Engine::GAPI->GetRendererState()->RendererSettings.FogHeight;
	DirectX::XMFLOAT3 color = *Engine::GAPI->GetRendererState()->RendererSettings.FogColorMod.toXMFLOAT3();

	float fnear = 15000.0f;
	float ffar = 60000.0f;
	float secScale = Engine::GAPI->GetRendererState()->RendererSettings.SectionDrawRadius;

	cb.HF_WeightZNear = std::max( 0.0f, WORLD_SECTION_SIZE * ((secScale - 0.5f) * 0.7f) - (ffar - fnear) ); // Keep distance from original fog but scale the near-fog up to section draw distance
	cb.HF_WeightZFar = WORLD_SECTION_SIZE * ((secScale - 0.5f) * 0.8f);

	float atmoMax = 83200.0f; // TODO: Calculate!	
	float atmoMin = 27799.9922f;

	cb.HF_WeightZFar = std::min( cb.HF_WeightZFar, atmoMax );
	cb.HF_WeightZNear = std::min( cb.HF_WeightZNear, atmoMin );

	if ( Engine::GAPI->GetFogOverride() > 0.0f ) {
		// Make sure the camera is inside the fog when in fog zone
		height = Toolbox::lerp( height, Engine::GAPI->GetCameraPosition().y + 10000, Engine::GAPI->GetFogOverride() ); // TODO: Get this from the actual fog-distance in the fogzone!

		// Override fog color when in fog zone
		color = Engine::GAPI->GetFogColor();

		// Make it z-Fog
		cb.HF_HeightFalloff = Toolbox::lerp( cb.HF_HeightFalloff, 0.000001f, Engine::GAPI->GetFogOverride() );

		// Turn up density
		cb.HF_GlobalDensity = Toolbox::lerp( cb.HF_GlobalDensity, cb.HF_GlobalDensity * 2, Engine::GAPI->GetFogOverride() );

		// Use other fog-values for fog-zones
		float distNear = WORLD_SECTION_SIZE * ((ffar - fnear) / ffar);
		cb.HF_WeightZNear = Toolbox::lerp( cb.HF_WeightZNear, WORLD_SECTION_SIZE * 0.09f, Engine::GAPI->GetFogOverride() );
		cb.HF_WeightZFar = Toolbox::lerp( cb.HF_WeightZFar, WORLD_SECTION_SIZE * 0.8, Engine::GAPI->GetFogOverride() );
	}

	cb.HF_FogColorMod = color;//Engine::GAPI->GetRendererState()->RendererSettings.FogColorMod;

	cb.HF_FogHeight = height;

	//cb.HF_FogColorMod = Engine::GAPI->GetRendererState()->GraphicsState.FF_FogColor;
	cb.HF_ProjAB = float2( Engine::GAPI->GetProjectionMatrix()._33,
		Engine::GAPI->GetProjectionMatrix()._34 );


	// Modify fog when raining
	float rain = Engine::GAPI->GetRainFXWeight();

	// Color
	XMFLOAT3 FogColorMod;
	FogColorMod.x = cb.HF_FogColorMod.x;
	FogColorMod.y = cb.HF_FogColorMod.y;
	FogColorMod.z = cb.HF_FogColorMod.z;
	XMStoreFloat3( &FogColorMod, DirectX::XMVectorLerpV( XMLoadFloat3( &FogColorMod ), XMLoadFloat3( &Engine::GAPI->GetRendererState()->RendererSettings.RainFogColor ), XMVectorSet( std::min( 1.0f, rain * 2.0f ), std::min( 1.0f, rain * 2.0f ), std::min( 1.0f, rain * 2.0f ), 0 ) ) ); // Scale color faster here, so it looks better on light rain //check if it works
	cb.HF_FogColorMod = FogColorMod;
	// Raining Density, only when not in fogzone
	cb.HF_GlobalDensity = Toolbox::lerp( cb.HF_GlobalDensity, Engine::GAPI->GetRendererState()->RendererSettings.RainFogDensity, rain * (1.0f - Engine::GAPI->GetFogOverride()) );




	hfPS->GetConstantBuffer()[0]->UpdateBuffer( &cb );
	hfPS->GetConstantBuffer()[0]->BindToPixelShader( 0 );

	PFXVS_ConstantBuffer vcb = {};
	vs->GetConstantBuffer()[0]->UpdateBuffer( &vcb );
	vs->GetConstantBuffer()[0]->BindToVertexShader( 0 );

	GSky* sky = Engine::GAPI->GetSky();
	hfPS->GetConstantBuffer()[1]->UpdateBuffer( &sky->GetAtmosphereCB() );
	hfPS->GetConstantBuffer()[1]->BindToPixelShader( 1 );

	engine->GetContext()->OMSetRenderTargets( 1, &oldRTV, nullptr );

	// Bind depthbuffer
	engine->GetDepthBuffer()->BindToPixelShader( engine->GetContext(), 1 );

	Engine::GAPI->GetRendererState()->BlendState.SetDefault();
	//Engine::GAPI->GetRendererState()->BlendState.SetAdditiveBlending();
	Engine::GAPI->GetRendererState()->BlendState.BlendEnabled = true;
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	// Copy
	FxRenderer->DrawFullScreenQuad();


	// Restore rendertargets
	ID3D11ShaderResourceView* srv = nullptr;
	engine->GetContext()->PSSetShaderResources( 1, 1, &srv );

	engine->GetContext()->OMSetRenderTargets( 1, &oldRTV, oldDSV );
	if ( oldRTV )oldRTV->Release();
	if ( oldDSV )oldDSV->Release();

	return XR_SUCCESS;
}