#include "pch.h"
#include "D3D11PFX_SMAA.h"
#include "Logger.h"
#include "Engine.h"
#include "RenderToTextureBuffer.h"
#include "D3D11GraphicsEngine.h"
#include "D3D11PfxRenderer.h"
#include "D3D11ShaderManager.h"
#include "D3D11VShader.h"
#include "GothicAPI.h"
#include "D3D11PShader.h"
#include <d3dcompiler.h>
#include <DDSTextureLoader.h>

using namespace DirectX;

D3D11PFX_SMAA::D3D11PFX_SMAA( D3D11PfxRenderer* rnd ) : D3D11PFX_Effect( rnd ) {
	EdgesTex = nullptr;
	BlendTex = nullptr;
	SMAAShader = nullptr;
	AreaTextureSRV = nullptr;
	SearchTextureSRV = nullptr;

	Init();
}


D3D11PFX_SMAA::~D3D11PFX_SMAA() {
	delete EdgesTex;
	delete BlendTex;

	if ( AreaTextureSRV )AreaTextureSRV->Release();
	if ( SearchTextureSRV )SearchTextureSRV->Release();
	if ( SMAAShader )SMAAShader->Release();
}

HRESULT D3DX11CreateEffectFromFile_RES(
	LPCTSTR pFileName,
	CONST D3D_SHADER_MACRO* pDefines,
	LPCSTR pProfile,
	UINT HLSLFlags,
	UINT FXFlags,
	ID3D11Device* pDevice,
	void* pPump,
	ID3DX11Effect** ppEffect,
	HRESULT* pHResult
) {
	Microsoft::WRL::ComPtr<ID3DBlob> ErrorsBuffer;

	HRESULT hr = D3DX11CompileEffectFromFile( Toolbox::ToWideChar( pFileName ).c_str(), pDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, HLSLFlags, FXFlags, pDevice, ppEffect, &ErrorsBuffer );

	char* Errors;
	if ( ErrorsBuffer.Get() ) {
		Errors = (char*)ErrorsBuffer->GetBufferPointer();
		if ( SUCCEEDED( hr ) ) {
			LogWarn() << Errors;
		} else {
			LogError() << Errors;
		}

		return hr;
	}

	return S_OK;
}

/** Creates needed resources */
bool D3D11PFX_SMAA::Init() {
	HRESULT hr;

	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine*)Engine::GraphicsEngine;

	LE( D3DX11CreateEffectFromFile_RES( "System\\GD3D11\\Shaders\\SMAA.fx", nullptr, "fx_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, engine->GetDevice(), nullptr, &SMAAShader, nullptr ) );

	if ( AreaTextureSRV )AreaTextureSRV->Release();
	if ( SearchTextureSRV )SearchTextureSRV->Release();


	/*SMAAShader->AddCustomVariable("colorTex", CVT_SHADER_RES_VIEW, &ColorTexIdx);
	SMAAShader->AddCustomVariable("colorTexGamma", CVT_SHADER_RES_VIEW, &ColorTexGammaIdx);
	SMAAShader->AddCustomVariable("colorTexPrev", CVT_SHADER_RES_VIEW, &ColorTexPrevIdx);
	SMAAShader->AddCustomVariable("colorMSTex", CVT_SHADER_RES_VIEW, &ColorMSTexIdx);
	SMAAShader->AddCustomVariable("depthTex", CVT_SHADER_RES_VIEW, &DepthTexIdx);
	SMAAShader->AddCustomVariable("velocityTex", CVT_SHADER_RES_VIEW, &VelocityTexIdx);
	SMAAShader->AddCustomVariable("edgesTex", CVT_SHADER_RES_VIEW, &EdgesTexIdx);
	SMAAShader->AddCustomVariable("blendTex", CVT_SHADER_RES_VIEW, &BlendTexIdx);
	SMAAShader->AddCustomVariable("areaTex", CVT_SHADER_RES_VIEW, &areaTexIdx);
	SMAAShader->AddCustomVariable("searchTex", CVT_SHADER_RES_VIEW, &SearchTexIdx);*/

	// Load the textures
	hr = CreateDDSTextureFromFile( engine->GetDevice(), L"system\\GD3D11\\Textures\\SMAA_AreaTexDX10.dds", nullptr, &AreaTextureSRV );
	LE( hr );

	hr = CreateDDSTextureFromFile( engine->GetDevice(), L"system\\GD3D11\\Textures\\SMAA_SearchTex.dds", nullptr, &SearchTextureSRV );
	LE( hr );

	SMAAShader->GetVariableByName( "areaTex" )->AsShaderResource()->SetResource( AreaTextureSRV );
	SMAAShader->GetVariableByName( "searchTex" )->AsShaderResource()->SetResource( SearchTextureSRV );

	LumaEdgeDetection = SMAAShader->GetTechniqueByName( "LumaEdgeDetection" );
	BlendingWeightCalculation = SMAAShader->GetTechniqueByName( "BlendingWeightCalculation" );
	NeighborhoodBlending = SMAAShader->GetTechniqueByName( "NeighborhoodBlending" );

	return true;
}

/** Renders the PostFX */
void D3D11PFX_SMAA::RenderPostFX( ID3D11ShaderResourceView* renderTargetSRV ) {
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine*)Engine::GraphicsEngine;
	engine->SetDefaultStates();
	engine->UpdateRenderStates();

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 0.0f;
	vp.Width = (float)FxRenderer->GetTempBuffer()->GetSizeX();
	vp.Height = (float)FxRenderer->GetTempBuffer()->GetSizeY();

	engine->GetShaderManager()->GetVShader( "VS_PFX" )->Apply(); // Apply vertexlayout for PP-Effects

	RenderToTextureBuffer* TempRTV = FxRenderer->GetTempBuffer();

	if ( !EdgesTex ) {
		OnResize( INT2( engine->GetResolution().x, engine->GetResolution().y ) );
	}

	engine->GetContext()->ClearRenderTargetView( EdgesTex->GetRenderTargetView(), (float*)&float4( 0, 0, 0, 0 ) );
	engine->GetContext()->ClearRenderTargetView( BlendTex->GetRenderTargetView(), (float*)&float4( 0, 0, 0, 0 ) );

	ID3D11RenderTargetView* RTV = nullptr;
	ID3D11RenderTargetView* OldRTV = nullptr;
	ID3D11DepthStencilView* OldDSV = nullptr;
	ID3DX11EffectShaderResourceVariable* SRV = nullptr;
	ID3D11ShaderResourceView* const NoSRV[3] = { nullptr,nullptr, nullptr };

	engine->GetContext()->OMGetRenderTargets( 1, &OldRTV, &OldDSV );
	engine->GetContext()->ClearDepthStencilView( OldDSV, D3D11_CLEAR_STENCIL, 0, 0 );

	/** First pass - Edge detection */
	RTV = EdgesTex->GetRenderTargetView();
	engine->GetContext()->OMSetRenderTargets( 1, &RTV, OldDSV );

	SMAAShader->GetVariableByName( "colorTexGamma" )->AsShaderResource()->SetResource( renderTargetSRV );

	LumaEdgeDetection->GetPassByIndex( 0 )->Apply( 0, engine->GetContext() );
	FxRenderer->DrawFullScreenQuad();

	//FxRenderer->CopyTextureToRTV(renderTargetSRV, RTV, INT2(0, 0), true);

	SMAAShader->GetVariableByName( "colorTexGamma" )->AsShaderResource()->SetResource( nullptr );

	engine->GetContext()->PSSetShaderResources( 0, 3, NoSRV );

	/** Second pass - BlendingWeightCalculation */
	RTV = BlendTex->GetRenderTargetView();
	engine->GetContext()->OMSetRenderTargets( 1, &RTV, OldDSV );

	SMAAShader->GetVariableByName( "edgesTex" )->AsShaderResource()->SetResource( EdgesTex->GetShaderResView() );

	BlendingWeightCalculation->GetPassByIndex( 0 )->Apply( 0, engine->GetContext() );
	FxRenderer->DrawFullScreenQuad();

	/** Copy back to main RTV */
	/*DXUTGetD3D11DeviceContext()->OMSetRenderTargets(1, &OldRTV, nullptr);
	CopyShader->SetBackBufferVar(BlendTex->GetShaderResView());
	CmplxScreenQuad.SetShader(CopyShader);
	CmplxScreenQuad.Render(6);

	goto end;*/



	/** Third pass - NeighborhoodBlending */
	RTV = TempRTV->GetRenderTargetView();
	engine->GetContext()->OMSetRenderTargets( 1, &RTV, OldDSV );


	SMAAShader->GetVariableByName( "colorTex" )->AsShaderResource()->SetResource( renderTargetSRV );
	SMAAShader->GetVariableByName( "blendTex" )->AsShaderResource()->SetResource( BlendTex->GetShaderResView() );

	NeighborhoodBlending->GetPassByIndex( 0 )->Apply( 0, engine->GetContext() );
	FxRenderer->DrawFullScreenQuad();

	SMAAShader->GetVariableByName( "colorTex" )->AsShaderResource()->SetResource( nullptr );

	/** Copy back to main RTV */
	engine->GetContext()->OMSetRenderTargets( 1, &OldRTV, nullptr );
	/*engine->GetContext()->OMSetRenderTargets(1, &OldRTV, nullptr);
	engine->DrawSRVToBackbuffer(TempRTV->GetShaderResView());
	goto end;*/

	ID3D11ShaderResourceView* srv = TempRTV->GetShaderResView();
	engine->GetContext()->PSSetShaderResources( 0, 1, &srv );


	if ( Engine::GAPI->GetRendererState()->RendererSettings.SharpenFactor > 0.0f ) {
		D3D11PShader* sharpenPS = engine->GetShaderManager()->GetPShader( "PS_PFX_Sharpen" );
		sharpenPS->Apply();

		GammaCorrectConstantBuffer gcb;
		gcb.G_Gamma = Engine::GAPI->GetGammaValue();
		gcb.G_Brightness = Engine::GAPI->GetBrightnessValue();
		gcb.G_TextureSize = engine->GetResolution();
		gcb.G_SharpenStrength = Engine::GAPI->GetRendererState()->RendererSettings.SharpenFactor;

		sharpenPS->GetConstantBuffer()[0]->UpdateBuffer( &gcb );
		sharpenPS->GetConstantBuffer()[0]->BindToPixelShader( 0 );

		FxRenderer->CopyTextureToRTV( TempRTV->GetShaderResView(), OldRTV, INT2( 0, 0 ), true );
	} else {
		FxRenderer->CopyTextureToRTV( TempRTV->GetShaderResView(), OldRTV );
	}

	engine->GetContext()->PSSetShaderResources( 0, 3, NoSRV );

	engine->GetContext()->OMSetRenderTargets( 1, &OldRTV, OldDSV );
	if ( OldRTV )OldRTV->Release();
	if ( OldDSV )OldDSV->Release();

	engine->SetDefaultStates( true );
}

/** Called on resize */
void D3D11PFX_SMAA::OnResize( const INT2& size ) {
	D3D11GraphicsEngine* engine = (D3D11GraphicsEngine*)Engine::GraphicsEngine;

	delete EdgesTex;
	delete BlendTex;

	HRESULT hr = S_OK;

	// Create Edges- and Blend-Textures
	EdgesTex = new RenderToTextureBuffer( engine->GetDevice(), size.x, size.y, DXGI_FORMAT_R8G8B8A8_UNORM, &hr );
	LE( hr );

	BlendTex = new RenderToTextureBuffer( engine->GetDevice(), size.x, size.y, DXGI_FORMAT_R8G8B8A8_UNORM, &hr );
	LE( hr );

	std::vector<D3D_SHADER_MACRO> Makros;

	char ResStr[256];
	sprintf( ResStr, "float2(1.0f/%d, 1.0f/%d)", (int)(size.x), (int)(size.y) );
	D3D_SHADER_MACRO PixelSize = { "SMAA_PIXEL_SIZE", ResStr };
	Makros.push_back( PixelSize );

	D3D_SHADER_MACRO HLSL = { "SMAA_HLSL_4_1", "1" };
	Makros.push_back( HLSL );

	const int QUALITY = 2;
	switch ( QUALITY ) {
	case 0:
	{
		D3D_SHADER_MACRO Quality = { "SMAA_PRESET_LOW","1" };
		Makros.push_back( Quality );
	}
	break;

	case 1:
	{
		D3D_SHADER_MACRO Quality = { "SMAA_PRESET_MEDIUM","1" };
		Makros.push_back( Quality );
	}
	break;

	case 2:
	{
		D3D_SHADER_MACRO Quality = { "SMAA_PRESET_HIGH","1" };
		Makros.push_back( Quality );
	}
	break;

	case 3:
	{
		D3D_SHADER_MACRO Quality = { "SMAA_PRESET_ULTRA","1" };
		Makros.push_back( Quality );
	}
	break;
	}
	D3D_SHADER_MACRO Null = { nullptr, nullptr };
	Makros.push_back( Null );

	LE( D3DX11CreateEffectFromFile_RES( "system\\GD3D11\\shaders\\SMAA.fx", &Makros[0], "fx_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, engine->GetDevice(), nullptr, &SMAAShader, nullptr ) );

	if ( AreaTextureSRV )AreaTextureSRV->Release();
	if ( SearchTextureSRV )SearchTextureSRV->Release();

	// Load the textures

	hr = CreateDDSTextureFromFile( engine->GetDevice(), L"system\\GD3D11\\Textures\\SMAA_AreaTexDX10.dds", nullptr, &AreaTextureSRV );
	LE( hr );

	hr = CreateDDSTextureFromFile( engine->GetDevice(), L"system\\GD3D11\\Textures\\SMAA_SearchTex.dds", nullptr, &SearchTextureSRV );
	LE( hr );

	SMAAShader->GetVariableByName( "areaTex" )->AsShaderResource()->SetResource( AreaTextureSRV );
	SMAAShader->GetVariableByName( "searchTex" )->AsShaderResource()->SetResource( SearchTextureSRV );

	LumaEdgeDetection = SMAAShader->GetTechniqueByName( "LumaEdgeDetection" );
	BlendingWeightCalculation = SMAAShader->GetTechniqueByName( "BlendingWeightCalculation" );
	NeighborhoodBlending = SMAAShader->GetTechniqueByName( "NeighborhoodBlending" );
}