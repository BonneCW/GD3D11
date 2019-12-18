#pragma once
#include "pch.h"
#include <d3d11.h>
#include <D3DX10math.h>

struct SimpleVertexStruct
{
	DirectX::SimpleMath::Vector3 Position;
	DirectX::SimpleMath::Vector2 TexCoord;
};

//This can draw a full screen quad
class D3D11FullscreenQuad
{
public:
	D3D11FullscreenQuad();
	virtual ~D3D11FullscreenQuad();

	//Fills the VertexBuffer
	HRESULT CreateQuad(ID3D11Device* device);

	ID3D11Buffer* GetBuffer(){return QuadVB;}

private:
	ID3D11Buffer* QuadVB; //Vertex buffer for the quad
};

