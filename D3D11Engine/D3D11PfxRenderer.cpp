#include "pch.h"
#include "D3D11PfxRenderer.h"
#include "RenderToTextureBuffer.h"
#include "Engine.h"
#include "D3D11GraphicsEngine.h"
#include "D3D11FullscreenQuad.h"
#include "D3D11ShaderManager.h"
#include "D3D11PShader.h"
#include "D3D11VShader.h"
#include "D3D11PFX_Blur.h"
#include "D3D11PFX_HeightFog.h"
#include "D3D11PFX_DistanceBlur.h"
#include "D3D11PFX_HDR.h"
#include "D3D11NVHBAO.h"
#include "D3D11PFX_SMAA.h"
#include "D3D11PFX_GodRays.h"

D3D11PfxRenderer::D3D11PfxRenderer() {
    D3D11GraphicsEngine* engine = (D3D11GraphicsEngine*)Engine::GraphicsEngine;
    ScreenQuad = std::make_unique<D3D11FullscreenQuad>();
    ScreenQuad->CreateQuad( engine->GetDevice().Get() );

    FX_Blur = std::make_unique<D3D11PFX_Blur>( this );
    FX_HeightFog = std::make_unique<D3D11PFX_HeightFog>( this );
    //FX_DistanceBlur = new D3D11PFX_DistanceBlur(this);
    FX_HDR = std::make_unique<D3D11PFX_HDR>( this );
    FX_SMAA = std::make_unique<D3D11PFX_SMAA>( this );

    FX_GodRays = std::make_unique<D3D11PFX_GodRays>( this );

    NvHBAO = std::make_unique<D3D11NVHBAO>();
    NvHBAO->Init();
}


D3D11PfxRenderer::~D3D11PfxRenderer() {
    //delete FX_DistanceBlur;
}

/** Renders the distance blur effect */
XRESULT D3D11PfxRenderer::RenderDistanceBlur() {
    FX_DistanceBlur->Render( nullptr );
    return XR_SUCCESS;
}

/** Blurs the given texture */
XRESULT D3D11PfxRenderer::BlurTexture( RenderToTextureBuffer* texture, bool leaveResultInD4_2, float scale, const DirectX::XMFLOAT4& colorMod, const std::string& finalCopyShader ) {
    FX_Blur->RenderBlur( texture, leaveResultInD4_2, 0.0f, scale, colorMod, finalCopyShader );
    return XR_SUCCESS;
}

/** Renders the heightfog */
XRESULT D3D11PfxRenderer::RenderHeightfog() {
    return FX_HeightFog->Render( nullptr );
}

/** Renders the godrays-Effect */
XRESULT D3D11PfxRenderer::RenderGodRays() {
    return FX_GodRays->Render( nullptr );
}

/** Renders the HDR-Effect */
XRESULT D3D11PfxRenderer::RenderHDR() {
    return FX_HDR->Render( nullptr );
}

/** Renders the SMAA-Effect */
XRESULT D3D11PfxRenderer::RenderSMAA() {
    D3D11GraphicsEngine* engine = (D3D11GraphicsEngine*)Engine::GraphicsEngine;
    FX_SMAA->RenderPostFX( engine->GetHDRBackBuffer().GetShaderResView() );

    return XR_SUCCESS;
}

/** Draws a fullscreenquad */
XRESULT D3D11PfxRenderer::DrawFullScreenQuad() {
    D3D11GraphicsEngine* engine = (D3D11GraphicsEngine*)Engine::GraphicsEngine;
    engine->UpdateRenderStates();

    engine->GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

    UINT offset = 0;
    UINT uStride = sizeof( SimpleVertexStruct );
    engine->GetContext()->IASetVertexBuffers( 0, 1, ScreenQuad->GetBuffer().GetAddressOf(), &uStride, &offset );

    //Microsoft::WRL::ComPtr<ID3D11Buffer> cb;
    //engine->GetContext()->VSSetConstantBuffers(0, 1, cb.GetAddressOf());

    //Draw the mesh
    engine->GetContext()->Draw( 6, 0 );

    return XR_SUCCESS;
}

/** Unbinds texturesamplers from the pixel-shader */
XRESULT D3D11PfxRenderer::UnbindPSResources( int num ) {
    ID3D11ShaderResourceView** srv = new ID3D11ShaderResourceView * [num];
    ZeroMemory( srv, sizeof( ID3D11ShaderResourceView* ) * num );

    D3D11GraphicsEngine* engine = (D3D11GraphicsEngine*)Engine::GraphicsEngine;
    engine->GetContext()->PSSetShaderResources( 0, num, srv );

    delete[] srv;

    return XR_SUCCESS;
}

/** Copies the given texture to the given RTV */
XRESULT D3D11PfxRenderer::CopyTextureToRTV( const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& texture, const Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& rtv, INT2 targetResolution, bool useCustomPS, INT2 offset ) {
    D3D11GraphicsEngine* engine = (D3D11GraphicsEngine*)Engine::GraphicsEngine;

    D3D11_VIEWPORT oldVP;
    if ( targetResolution.x != 0 && targetResolution.y != 0 ) {
        UINT n = 1;
        engine->GetContext()->RSGetViewports( &n, &oldVP );

        D3D11_VIEWPORT vp;
        vp.TopLeftX = (float)offset.x;
        vp.TopLeftY = (float)offset.y;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 0.0f;
        vp.Width = (float)targetResolution.x;
        vp.Height = (float)targetResolution.y;

        engine->GetContext()->RSSetViewports( 1, &vp );
    }

    // Save old rendertargets
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> oldRTV;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> oldDSV;
    engine->GetContext()->OMGetRenderTargets( 1, oldRTV.GetAddressOf(), oldDSV.GetAddressOf() );

    // Bind shaders
    if ( !useCustomPS ) {
        auto simplePS = engine->GetShaderManager().GetPShader( "PS_PFX_Simple" );
        simplePS->Apply();
    }

    engine->GetShaderManager().GetVShader( "VS_PFX" )->Apply();

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    engine->GetContext()->PSSetShaderResources( 0, 1, srv.GetAddressOf() );

    engine->GetContext()->OMSetRenderTargets( 1, rtv.GetAddressOf(), nullptr );

    if ( texture.Get() )
        engine->GetContext()->PSSetShaderResources( 0, 1, texture.GetAddressOf() );

    DrawFullScreenQuad();

    engine->GetContext()->PSSetShaderResources( 0, 1, srv.GetAddressOf() );
    engine->GetContext()->OMSetRenderTargets( 1, oldRTV.GetAddressOf(), oldDSV.Get() );

    if ( targetResolution.x != 0 && targetResolution.y != 0 ) {
        engine->GetContext()->RSSetViewports( 1, &oldVP );
    }

    return XR_SUCCESS;
}

/** Called on resize */
XRESULT D3D11PfxRenderer::OnResize( const INT2& newResolution ) {
    D3D11GraphicsEngine* engine = (D3D11GraphicsEngine*)Engine::GraphicsEngine;

    // Create temp-buffer
    TempBuffer.reset( new RenderToTextureBuffer( engine->GetDevice(), newResolution.x, newResolution.y, DXGI_FORMAT_R16G16B16A16_FLOAT, nullptr ) );
    TempBufferDS4_1.reset( new RenderToTextureBuffer( engine->GetDevice(), newResolution.x / 4, newResolution.y / 4, DXGI_FORMAT_R16G16B16A16_FLOAT, nullptr ) );
    TempBufferDS4_2.reset( new RenderToTextureBuffer( engine->GetDevice(), newResolution.x / 4, newResolution.y / 4, DXGI_FORMAT_R16G16B16A16_FLOAT, nullptr ) );

    FX_SMAA->OnResize( newResolution );

    return XR_SUCCESS;
}

/** Draws the HBAO-Effect to the given buffer */
XRESULT D3D11PfxRenderer::DrawHBAO( const Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& rtv ) {
    return NvHBAO->Render( rtv.Get() );
}