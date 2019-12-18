#pragma once
#include "pch.h"

enum zTCam_ClipType {
	ZTCAM_CLIPTYPE_IN, 
	ZTCAM_CLIPTYPE_OUT, 
	ZTCAM_CLIPTYPE_CROSSING 
};

enum zTCam_ClipFlags {
	CLIP_FLAGS_FULL		= 63, 
	CLIP_FLAGS_NO_FAR	= 15 
};

#pragma pack (push, 1)	
struct zTBBox3D {
	DirectX::SimpleMath::Vector3	Min;
	DirectX::SimpleMath::Vector3	Max;

	enum zTPlaneClass {
		zPLANE_INFRONT, 
		zPLANE_BEHIND, 
		zPLANE_ONPLANE, 
		zPLANE_SPANNING 
	};

	int ClassifyToPlane (float planeDist, int axis) const {
		if (planeDist>=((float *)&Max)[axis])	
			return zPLANE_BEHIND;	
		else if (planeDist<=((float *)&Max)[axis])	
			return zPLANE_INFRONT;	
		else return zPLANE_SPANNING;
	}
};

struct zTPlane {
	float Distance;
	DirectX::SimpleMath::Vector3 Normal;
};
#pragma pack (pop)

class zCWorld;
class zCVob;
class zCCamera;

struct zTRenderContext {
	int ClipFlags;
	zCVob * Vob;
	zCWorld * world;
	zCCamera * cam;
	float distVobToCam;
	
	// More not needed stuff here
};

struct zCRenderLight {
	int	LightType;            
	DirectX::SimpleMath::Vector3 ColorDiffuse;			
	DirectX::SimpleMath::Vector3 Position;				
	DirectX::SimpleMath::Vector3 Direction;				
	float Range;						
	float RangeInv;
	DirectX::SimpleMath::Vector3 PositionLS;				
	DirectX::SimpleMath::Vector3 DirectionLS;			
	float Dir_approxFalloff;			
};

struct zCRenderLightContainer {
	zCRenderLight			LightList[8];
	int						NumLights;

private:
	int	DoPrelight;
	int	DoSmoothPrelit;
	float PreLightDist;
	DirectX::SimpleMath::Matrix MatObjToCam;
};

enum zTRnd_AlphaBlendFunc {	
	zRND_ALPHA_FUNC_MAT_DEFAULT = 0,	
	zRND_ALPHA_FUNC_NONE = 1,				
	zRND_ALPHA_FUNC_BLEND = 2,				
	zRND_ALPHA_FUNC_ADD = 3,				
	zRND_ALPHA_FUNC_SUB = 4,				
	zRND_ALPHA_FUNC_MUL = 5,				
	zRND_ALPHA_FUNC_MUL2 = 6,				
	zRND_ALPHA_FUNC_TEST = 7,				
	zRND_ALPHA_FUNC_BLEND_TEST = 8			
};
