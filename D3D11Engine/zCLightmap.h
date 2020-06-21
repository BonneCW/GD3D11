#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"


class zCLightmap {
public:

	DirectX::XMFLOAT2 GetLightmapUV( const DirectX::XMFLOAT3& worldPos ) {
		XMVECTOR q = XMLoadFloat3( &worldPos ) - XMLoadFloat3( &LightmapOrigin );

		XMFLOAT2 lightmap;
		lightmap.x = XMVectorGetX( DirectX::XMVector3Dot( q, XMLoadFloat3( &LightmapUVRight ) ) );
		lightmap.y = XMVectorGetY( DirectX::XMVector3Dot( q, XMLoadFloat3( &LightmapUVUp ) ) );
		return lightmap;
	}


	char data[0x24];

	DirectX::XMFLOAT3 LightmapOrigin;
	DirectX::XMFLOAT3 LightmapUVUp;
	DirectX::XMFLOAT3 LightmapUVRight;

	zCTexture* Texture;
};