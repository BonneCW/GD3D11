#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
//#include "zCWorld.h"
#include "zCObject.h"

class zCSkyPlanet
{
public:
	int vtbl;
	void * mesh; // 0
	DirectX::SimpleMath::Vector4 color0; // 4 
	DirectX::SimpleMath::Vector4 color1; // 20
	float		size; // 36
	DirectX::SimpleMath::Vector3 pos; // 40
	DirectX::SimpleMath::Vector3 rotAxis; // 52


};

enum zESkyLayerMode
{
	zSKY_LAYER_MODE_POLY,
	zSKY_LAYER_MODE_BOX
};

class zCSkyLayerData
{
public:
	zESkyLayerMode SkyMode;
	zCTexture * Tex;
	char zSTring_TexName[20];

	float TexAlpha;
	float TexScale;
	DirectX::SimpleMath::Vector2 TexSpeed;
};


class zCSkyState
{
public:
	float Time;
	DirectX::SimpleMath::Vector3 PolyColor;
	DirectX::SimpleMath::Vector3 FogColor;
	DirectX::SimpleMath::Vector3 DomeColor1;
	DirectX::SimpleMath::Vector3 DomeColor0;
	float FogDist;
	int	SunOn;
	int	CloudShadowOn;
	zCSkyLayerData	Layer[2];
};

enum zTWeather
{
	zTWEATHER_SNOW,
	zTWEATHER_RAIN
};

class zCSkyLayer;

typedef void (__thiscall* zCSkyControllerRenderSkyPre)(void *);
typedef void (__thiscall* zCSkyControllerRenderSkyPost)(void *, int);
class zCSkyController : public zCObject
{
public:
	void RenderSkyPre()
	{
		int * vtbl = (int *)((int *)this)[0];

		zCSkyControllerRenderSkyPre fn = (zCSkyControllerRenderSkyPre)vtbl[GothicMemoryLocations::zCSkyController::VTBL_RenderSkyPre];
		fn(this);
	}

	void RenderSkyPost()
	{
		int * vtbl = (int *)((int *)this)[0];

		zCSkyControllerRenderSkyPost fn = (zCSkyControllerRenderSkyPost)vtbl[GothicMemoryLocations::zCSkyController::VTBL_RenderSkyPost];
		fn(this, 1);
	}

	DWORD* PolyLightCLUTPtr;
	float cloudShadowScale;
	BOOL ColorChanged;
	zTWeather Weather;
};

// Controler - Typo in Gothic
class zCSkyController_Outdoor : public zCSkyController
{
public:
	/** Hooks the functions of this Class */
	static void Hook()
	{
		// Overwrite the rain-renderfunction and particle-updates
		DWORD dwProtect;
		VirtualProtect((void *)GothicMemoryLocations::zCSkyController_Outdoor::LOC_ProcessRainFXNOPStart,
			GothicMemoryLocations::zCSkyController_Outdoor::LOC_ProcessRainFXNOPEnd
			- GothicMemoryLocations::zCSkyController_Outdoor::LOC_ProcessRainFXNOPStart,
			PAGE_EXECUTE_READWRITE, &dwProtect);

		REPLACE_RANGE(GothicMemoryLocations::zCSkyController_Outdoor::LOC_ProcessRainFXNOPStart, GothicMemoryLocations::zCSkyController_Outdoor::LOC_ProcessRainFXNOPEnd - 1, INST_NOP);

		// Replace the check for the lensflare with nops

		VirtualProtect((void *)GothicMemoryLocations::zCSkyController_Outdoor::LOC_SunVisibleStart,
			GothicMemoryLocations::zCSkyController_Outdoor::LOC_SunVisibleEnd - GothicMemoryLocations::zCSkyController_Outdoor::LOC_SunVisibleStart,
			PAGE_EXECUTE_READWRITE, &dwProtect);

		REPLACE_RANGE(GothicMemoryLocations::zCSkyController_Outdoor::LOC_SunVisibleStart, GothicMemoryLocations::zCSkyController_Outdoor::LOC_SunVisibleEnd - 1, INST_NOP);

	}

	/** Updates the rain-weight and sound-effects */
	void ProcessRainFX()
	{
		XCALL(GothicMemoryLocations::zCSkyController_Outdoor::ProcessRainFX);
	}

	/** Returns the rain-fx weight */
	float GetRainFXWeight()
	{
		return *(float *)THISPTR_OFFSET(GothicMemoryLocations::zCSkyController_Outdoor::Offset_OutdoorRainFXWeight);
	}

	/** Returns the currently active weather type */
	zTWeather GetWeatherType()
	{
#ifdef BUILD_GOTHIC_2_6_fix
		return *(zTWeather *)THISPTR_OFFSET(GothicMemoryLocations::zCSkyController_Outdoor::Offset_WeatherType);
#else
		return zTWeather::zTWEATHER_RAIN;
#endif
	}

	/** Returns the master-time wrapped between 0 and 1 */
	float GetMasterTime()
	{
		return *(float *)(((char *)this) + GothicMemoryLocations::zCSkyController_Outdoor::Offset_MasterTime);
	}

	int GetUnderwaterFX()
	{
		XCALL(GothicMemoryLocations::zCSkyController_Outdoor::GetUnderwaterFX);
	}

	DirectX::SimpleMath::Vector3 GetOverrideColor()
	{
#ifndef BUILD_GOTHIC_1_08k
		return *(DirectX::SimpleMath::Vector3 *)THISPTR_OFFSET(GothicMemoryLocations::zCSkyController_Outdoor::Offset_OverrideColor);
#else
		return DirectX::SimpleMath::Vector3::Zero;
#endif
	}

	bool GetOverrideFlag()
	{
#ifndef BUILD_GOTHIC_1_08k
		int f = *(int *)THISPTR_OFFSET(GothicMemoryLocations::zCSkyController_Outdoor::Offset_OverrideFlag);

		return f != 0;
#else
		return 0;
#endif
	}

	/** Returns the sun position in world coords */
	DirectX::SimpleMath::Vector3 GetSunWorldPosition(float timeScale = 1.0f) {

		//float angle = GetMasterTime() * 2.0f * (float)XM_PI; // Get mastertime into rad, 0 and 12 are now at the horizon, 18 is in the sky
		//angle += (float)XM_PI * 0.5f; // 12 is now in the sky, 18 horizon
		float angle = ((GetMasterTime() * timeScale - 0.3f) * 1.25f + 0.5f) * 2.0f * (float)DirectX::XM_PI;

		DirectX::SimpleMath::Vector3 sunPos = DirectX::SimpleMath::Vector3(-60, 0, 100);
		sunPos.Normalize();
		DirectX::SimpleMath::Vector3 rotAxis = DirectX::SimpleMath::Vector3::Right;

		DirectX::SimpleMath::Matrix r = HookedFunctions::OriginalFunctions.original_Alg_Rotation3DNRad(rotAxis, -angle);
		r = r.Transpose();

		DirectX::SimpleMath::Vector3 pos;

		pos = DirectX::XMVector3TransformNormal(sunPos, r);

		return pos;
	}

	void SetCameraLocationHint(int hint)
	{
		XCALL(GothicMemoryLocations::zCSkyController_Outdoor::SetCameraLocationHint);
	}

	zCSkyState* GetMasterState()
	{
		return (zCSkyState*)THISPTR_OFFSET(GothicMemoryLocations::zCSkyController_Outdoor::Offset_MasterState);
	}
};

class zCMesh;
class zCSkyLayer
{
public:
	zCMesh* SkyPolyMesh;
	zCPolygon* SkyPoly;
	DirectX::SimpleMath::Vector2 SkyTexOffs;
	zCMesh*	SkyDomeMesh;
	zESkyLayerMode SkyMode;
};
