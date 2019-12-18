#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"

class zCLightmap
{
public:

	DirectX::SimpleMath::Vector2 GetLightmapUV(const DirectX::SimpleMath::Vector3 & worldPos)
	{
		DirectX::SimpleMath::Vector3 q = worldPos - LightmapOrigin;
		return DirectX::SimpleMath::Vector2(q.Dot(LightmapUVRight), q.Dot(LightmapUVUp));
	}


	char data[0x24];

	DirectX::SimpleMath::Vector3	LightmapOrigin;
	DirectX::SimpleMath::Vector3	LightmapUVUp;
	DirectX::SimpleMath::Vector3	LightmapUVRight;

    zCTexture * Texture;
};