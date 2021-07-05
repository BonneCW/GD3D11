#pragma once
#include "pch.h"
#include <d3d11_1.h>
#include <DirectXMath.h>

struct SimpleVertexStruct {
    DirectX::XMFLOAT3 Position;
    DirectX::XMFLOAT2 TexCoord;
};

//This can draw a full screen quad
class D3D11FullscreenQuad {
public:
    D3D11FullscreenQuad();
    virtual ~D3D11FullscreenQuad();

    //Fills the VertexBuffer
    HRESULT CreateQuad( Microsoft::WRL::ComPtr<ID3D11Device1> device );

    Microsoft::WRL::ComPtr<ID3D11Buffer>& GetBuffer() { return QuadVB; }

private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> QuadVB; //Vertex buffer for the quad
};

