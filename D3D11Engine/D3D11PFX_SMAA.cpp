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

D3D11PFX_SMAA::D3D11PFX_SMAA( D3D11PfxRenderer* rnd ) : D3D11PFX_Effect( rnd ) {
	EdgesTex = nullptr;
	BlendTex = nullptr;

	Init();
}

D3D11PFX_SMAA::~D3D11PFX_SMAA() {
	delete EdgesTex;
	delete BlendTex;
}

HRESULT D3DX11CreateEffectFromFile_RES(
	LPCTSTR pFileName,
	CONST D3D_SHADER_MACRO* pDefines,
	LPCSTR pProfile,
	UINT HLSLFlags,
	UINT FXFlags,
	const Microsoft::WRL::ComPtr<ID3D11Device1>& pDevice,
	void* pPump,
	ID3DX11Effect** ppEffect,
	HRESULT* pHResult
) {
	Microsoft::WRL::ComPtr<ID3DBlob> ErrorsBuffer;

	HRESULT hr = D3DX11CompileEffectFromFile( Toolbox::ToWideChar( pFileName ).c_str(), pDefines, D3D_COMPILE_STANDARD_FILE_INCLUDE, HLSLFlags, FXFlags, pDevice.Get(), ppEffect, ErrorsBuffer.GetAddressOf() );

	char* Errors;
	if ( ErrorsBuffer.Get() ) {
		Errors = reinterpret_cast<char*>(ErrorsBuffer->GetBufferPointer());
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

	D3D11GraphicsEngine* engine = reinterpret_cast<D3D11GraphicsEngine*>(Engine::GraphicsEngine);

	LE( D3DX11CreateEffectFromFile_RES( "System\\GD3D11\\Shaders\\SMAA.fx", nullptr, "fx_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, engine->GetDevice().Get(), nullptr, SMAAShader.GetAddressOf(), nullptr ) );

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
	hr = CreateDDSTextureFromFile( engine->GetDevice().Get(), L"system\\GD3D11\\Textures\\SMAA_AreaTexDX10.dds", nullptr, AreaTextureSRV.ReleaseAndGetAddressOf() );
	LE( hr );

	hr = CreateDDSTextureFromFile( engine->GetDevice().Get(), L"system\\GD3D11\\Textures\\SMAA_SearchTex.dds", nullptr, SearchTextureSRV.ReleaseAndGetAddressOf() );
	LE( hr );

	SMAAShader->GetVariableByName( "areaTex" )->AsShaderResource()->SetResource( AreaTextureSRV.Get() );
	SMAAShader->GetVariableByName( "searchTex" )->AsShaderResource()->SetResource( SearchTextureSRV.Get() );

	LumaEdgeDetection = SMAAShader->GetTechniqueByName( "LumaEdgeDetection" );
	BlendingWeightCalculation = SMAAShader->GetTechniqueByName( "BlendingWeightCalculation" );
	NeighborhoodBlending = SMAAShader->GetTechniqueByName( "NeighborhoodBlending" );

	return true;
}

/** Renders the PostFX */
void D3D11PFX_SMAA::RenderPostFX(const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& renderTargetSRV ) {
	D3D11GraphicsEngine* engine = reinterpret_cast<D3D11GraphicsEngine*>(Engine::GraphicsEngine);
	engine->SetDefaultStates();
	engine->UpdateRenderStates();

	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.Width = static_cast<float>(FxRenderer->GetTempBuffer().GetSizeX());
	vp.Height = static_cast<float>(FxRenderer->GetTempBuffer().GetSizeY());

	engine->GetShaderManager().GetVShader( "VS_PFX" )->Apply(); // Apply vertexlayout for PP-Effects

	RenderToTextureBuffer& TempRTV = FxRenderer->GetTempBuffer();

	if ( !EdgesTex ) {
		OnResize( INT2( engine->GetResolution().x, engine->GetResolution().y ) );
	}

	engine->GetContext()->ClearRenderTargetView( EdgesTex->GetRenderTargetView().Get(), reinterpret_cast<float*>(&float4( 0, 0, 0, 0 )) );
	engine->GetContext()->ClearRenderTargetView( BlendTex->GetRenderTargetView().Get(), reinterpret_cast<float*>(&float4( 0, 0, 0, 0 )) );

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> OldRTV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> OldDSV;
	ID3D11ShaderResourceView* const NoSRV[3] = { nullptr, nullptr, nullptr };

	engine->GetContext()->OMGetRenderTargets( 1, OldRTV.GetAddressOf(), OldDSV.GetAddressOf() );
	engine->GetContext()->ClearDepthStencilView( OldDSV.Get(), D3D11_CLEAR_STENCIL, 0, 0 );

	/** First pass - Edge detection */
	engine->GetContext()->OMSetRenderTargets( 1, EdgesTex->GetRenderTargetView().GetAddressOf(), OldDSV.Get() );

	SMAAShader->GetVariableByName( "colorTexGamma" )->AsShaderResource()->SetResource( renderTargetSRV.Get() );

	LumaEdgeDetection->GetPassByIndex( 0 )->Apply( 0, engine->GetContext().Get() );
	FxRenderer->DrawFullScreenQuad();

	//FxRenderer->CopyTextureToRTV(renderTargetSRV, RTV, INT2(0, 0), true);

	engine->GetContext()->PSSetShaderResources( 0, 3, NoSRV );

	/** Second pass - BlendingWeightCalculation */
	engine->GetContext()->OMSetRenderTargets( 1, BlendTex->GetRenderTargetView().GetAddressOf(), OldDSV.Get() );

	SMAAShader->GetVariableByName( "edgesTex" )->AsShaderResource()->SetResource( EdgesTex->GetShaderResView().Get() );

	BlendingWeightCalculation->GetPassByIndex( 0 )->Apply( 0, engine->GetContext().Get() );
	FxRenderer->DrawFullScreenQuad();

    engine->GetContext()->PSSetShaderResources( 0, 3, NoSRV );

	/** Third pass - NeighborhoodBlending */
	engine->GetContext()->OMSetRenderTargets( 1, TempRTV.GetRenderTargetView().GetAddressOf(), OldDSV.Get() );

	SMAAShader->GetVariableByName( "colorTex" )->AsShaderResource()->SetResource( renderTargetSRV.Get() );
	SMAAShader->GetVariableByName( "blendTex" )->AsShaderResource()->SetResource( BlendTex->GetShaderResView().Get() );

	NeighborhoodBlending->GetPassByIndex( 0 )->Apply( 0, engine->GetContext().Get() );
	FxRenderer->DrawFullScreenQuad();

    engine->GetContext()->PSSetShaderResources( 0, 3, NoSRV );

	/** Copy back to main RTV */
	engine->GetContext()->OMSetRenderTargets( 1, OldRTV.GetAddressOf(), nullptr );
    engine->GetContext()->PSSetShaderResources( 0, 1, TempRTV.GetShaderResView().GetAddressOf() );

	if ( Engine::GAPI->GetRendererState().RendererSettings.SharpenFactor > 0.0f ) {
		auto sharpenPS = engine->GetShaderManager().GetPShader( "PS_PFX_Sharpen" );
		sharpenPS->Apply();

		GammaCorrectConstantBuffer gcb;
		gcb.G_Gamma = Engine::GAPI->GetGammaValue();
		gcb.G_Brightness = Engine::GAPI->GetBrightnessValue();
		gcb.G_TextureSize = engine->GetResolution();
		gcb.G_SharpenStrength = Engine::GAPI->GetRendererState().RendererSettings.SharpenFactor;

		sharpenPS->GetConstantBuffer()[0]->UpdateBuffer( &gcb );
		sharpenPS->GetConstantBuffer()[0]->BindToPixelShader( 0 );

        FxRenderer->CopyTextureToRTV( TempRTV.GetShaderResView(), OldRTV, INT2( 0, 0 ), true );
	} else {
        FxRenderer->CopyTextureToRTV( TempRTV.GetShaderResView(), OldRTV );
	}

	engine->GetContext()->PSSetShaderResources( 0, 3, NoSRV );
	engine->GetContext()->OMSetRenderTargets( 1, OldRTV.GetAddressOf(), OldDSV.Get() );

    engine->SetDefaultStates( true );
}

/** Called on resize */
void D3D11PFX_SMAA::OnResize( const INT2& size ) {
	D3D11GraphicsEngine* engine = reinterpret_cast<D3D11GraphicsEngine*>(Engine::GraphicsEngine);

	delete EdgesTex;
	delete BlendTex;

	HRESULT hr = S_OK;

	// Create Edges- and Blend-Textures
	EdgesTex = new RenderToTextureBuffer( engine->GetDevice().Get(), size.x, size.y, DXGI_FORMAT_R8G8B8A8_UNORM, &hr );
	LE( hr );

	BlendTex = new RenderToTextureBuffer( engine->GetDevice().Get(), size.x, size.y, DXGI_FORMAT_R8G8B8A8_UNORM, &hr );
	LE( hr );

	std::vector<D3D_SHADER_MACRO> Makros;

	char ResStr[256];
	sprintf( ResStr, "float2(1.0f/%d, 1.0f/%d)", size.x, size.y );
	D3D_SHADER_MACRO PixelSize = { "SMAA_PIXEL_SIZE", ResStr };
	Makros.push_back( PixelSize );

	D3D_SHADER_MACRO HLSL = { "SMAA_HLSL_4_1", "1" };
	Makros.push_back( HLSL );

	int QUALITY = 2;
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

	LE( D3DX11CreateEffectFromFile_RES( "system\\GD3D11\\shaders\\SMAA.fx", &Makros[0], "fx_5_0", D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, engine->GetDevice().Get(), nullptr, SMAAShader.ReleaseAndGetAddressOf(), nullptr ) );

	// Load the textures

	hr = CreateDDSTextureFromFile( engine->GetDevice().Get(), L"system\\GD3D11\\Textures\\SMAA_AreaTexDX10.dds", nullptr, AreaTextureSRV.ReleaseAndGetAddressOf() );
	LE( hr );

	hr = CreateDDSTextureFromFile( engine->GetDevice().Get(), L"system\\GD3D11\\Textures\\SMAA_SearchTex.dds", nullptr, SearchTextureSRV.ReleaseAndGetAddressOf() );
	LE( hr );

	SMAAShader->GetVariableByName( "areaTex" )->AsShaderResource()->SetResource( AreaTextureSRV.Get() );
	SMAAShader->GetVariableByName( "searchTex" )->AsShaderResource()->SetResource( SearchTextureSRV.Get() );

	LumaEdgeDetection = SMAAShader->GetTechniqueByName( "LumaEdgeDetection" );
	BlendingWeightCalculation = SMAAShader->GetTechniqueByName( "BlendingWeightCalculation" );
	NeighborhoodBlending = SMAAShader->GetTechniqueByName( "NeighborhoodBlending" );
}
