#pragma once
#include <d3dx11effect.h>
#include "pch.h"
#include "d3d11pfx_effect.h"

struct RenderToTextureBuffer;
class D3D11PFX_SMAA :
    public D3D11PFX_Effect {
public:
    D3D11PFX_SMAA( D3D11PfxRenderer* rnd );
    ~D3D11PFX_SMAA();

    /** Creates needed resources */
    bool Init();

    /** Called on resize */
    void OnResize( const INT2& size );

    /** Renders the PostFX */
    void RenderPostFX( const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& renderTargetSRV );

    /** Draws this effect to the given buffer */
    XRESULT Render( RenderToTextureBuffer* fxbuffer ) { return XR_SUCCESS; };

private:
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> AreaTextureSRV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SearchTextureSRV;

    RenderToTextureBuffer* EdgesTex;
    RenderToTextureBuffer* BlendTex;

    Microsoft::WRL::ComPtr<ID3DX11Effect> SMAAShader;
    ID3DX11EffectTechnique* LumaEdgeDetection;
    ID3DX11EffectTechnique* BlendingWeightCalculation;
    ID3DX11EffectTechnique* NeighborhoodBlending;
};

