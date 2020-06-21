#pragma once
#include "pch.h"
#include "detours.h"
#include "GothicMemoryLocations.h"
#include "zTypes.h"
#include "HookExceptionFilter.h"

/** This file stores the original versions of the hooked functions and the function declerations */

class zCFileBIN;
class zCCamera;
class zCVob;
class zSTRING;
class zCBspBase;
class oCNPC;
class zCPolygon;
class zCTexture;

template <class T>
class zCTree;

class zCVisual;

typedef int( __thiscall* zCBspTreeLoadBIN )(void*, zCFileBIN&, int);
typedef void( __thiscall* zCWorldRender )(void*, zCCamera&);
typedef void( __thiscall* zCWorldVobAddedToWorld )(void*, zCVob*);
typedef void( __thiscall* oCNPCEnable )(void*, DirectX::XMFLOAT3&);
typedef void( __thiscall* oCWorldInsertVobInWorld )(void*, zCVob*);
typedef void( __thiscall* zCBspTreeAddVob )(void*, zCVob*);
typedef void( __thiscall* zCWorldLoadWorld )(void*, const zSTRING& fileName, const int loadMode);
typedef void( __thiscall* oCGameEnterWorld )(void*, oCNPC* playerVob, int changePlayerPos, const zSTRING& startpoint);
typedef void( __thiscall* oCGameUpdatePlayerStatus )(void*);
typedef void( __thiscall* zCWorldVobRemovedFromWorld )(void*, zCVob*);
typedef XMFLOAT4X4( __cdecl* Alg_Rotation3DNRad )(const DirectX::XMFLOAT3& axis, const float angle);
typedef int( __cdecl* vidGetFPSRate )();
typedef void( __thiscall* GenericDestructor )(void*);
typedef void( __thiscall* GenericThiscall )(void*);
typedef void( __thiscall* zCMaterialConstruktor )(void*);
typedef void( __thiscall* zCMaterialInitValues )(void*);
typedef void( __fastcall* zCBspNodeRenderIndoor )(void*, int);
typedef void( __fastcall* zCBspNodeRenderOutdoor )(void*, zCBspBase*, zTBBox3D, int, int);

typedef int( __fastcall* zCBspBaseCollectPolysInBBox3D )(void*, const zTBBox3D&, zCPolygon**&, int&);

typedef int( __fastcall* zCBspBaseCheckRayAgainstPolys )(void*, const DirectX::XMFLOAT3&, const DirectX::XMFLOAT3&, DirectX::XMFLOAT3&);

typedef int( __thiscall* zFILEOpen )(void*, zSTRING&, bool);
typedef void( __thiscall* zCRnd_D3DVid_SetScreenMode )(void*, int);
typedef void( __thiscall* zCRnd_D3D_DrawPoly )(void*, zCPolygon*);
typedef void( __thiscall* zCRnd_D3D_DrawPolySimple )(void*, zCTexture*, void*, int);
typedef int( __thiscall* zCOptionReadInt )(void*, zSTRING const&, char const*, int);
typedef int( __thiscall* zCOptionReadBool )(void*, zSTRING const&, char const*, int);
typedef unsigned long( __thiscall* zCOptionReadDWORD )(void*, zSTRING const&, char const*, unsigned long);
typedef void( __cdecl* zCViewSetMode )(int, int, int, HWND*);
typedef int( __thiscall* zCViewFontSize )(void*, const zSTRING&);
typedef void( __thiscall* zCViewBlitText )(void*);
typedef void( __thiscall* zCViewPrint )(void*, int, int, const zSTRING&);
typedef int( __stdcall* HandledWinMain )(HINSTANCE, HINSTANCE, LPSTR, int);
typedef int( __thiscall* CGameManagerExitGame )(void*);
typedef const zSTRING* (__thiscall* zCVisualGetFileExtension)(void*, int);
typedef long( __stdcall* zCExceptionHandlerUnhandledExceptionFilter )(void*);
typedef void( __thiscall* zCWorldDisposeVobs )(void*, zCTree<zCVob>*);
typedef void( __thiscall* oCSpawnManagerSpawnNpc )(void*, oCNPC*, const DirectX::XMFLOAT3&, float);
typedef void( __thiscall* oCSpawnManagerInsertNpc )(void*, oCNPC*, const DirectX::XMFLOAT3&);
typedef int( __thiscall* oCSpawnManagerCheckRemoveNpc )(void*, oCNPC*);
typedef void( __thiscall* oCSpawnManagerCheckInsertNpc )(void*);
typedef void( __thiscall* zCVobSetVisual )(void*, zCVisual*);


typedef int( __thiscall* zCTex_D3DXTEX_BuildSurfaces )(void*, int);
typedef int( __thiscall* zCTextureLoadResourceData )(void*);
typedef int( __thiscall* zCThreadSuspendThread )(void*);
typedef void( __thiscall* zCResourceManagerCacheOut )(void*, class zCResource*);
typedef void( __thiscall* zCQuadMarkCreateQuadMark )(void*, zCPolygon*, const float3&, const float2&, struct zTEffectParams*);
typedef void( __thiscall* oCWorldEnableVob )(void*, zCVob*, zCVob*);
typedef void( __thiscall* oCWorldRemoveVob )(void*, zCVob*);
typedef void( __thiscall* oCWorldDisableVob )(void*, zCVob*);
typedef void( __fastcall* oCWorldRemoveFromLists )(void*, zCVob*);
typedef int( __thiscall* zCBinkPlayerOpenVideo )(void*, class zSTRING);
typedef int( __thiscall* zCModelPrototypeLoadModelASC )(void*, class zSTRING const&);
typedef int( __thiscall* zCModelPrototypeReadMeshAndTreeMSB )(void*, int&, class zCFileBIN&);

typedef void( __thiscall* zCActiveSndAutoCalcObstruction )(void*, int);

#ifdef BUILD_GOTHIC_1_08k
typedef void( __thiscall* zCVobEndMovement )(void*);
#else
typedef void( __thiscall* zCVobEndMovement )(void*, int);
#endif
struct zTRndSurfaceDesc;
struct HookedFunctionInfo {

	/** Init all hooks here */
	void InitHooks();

	zCBspTreeLoadBIN original_zCBspTreeLoadBIN;
	zCWorldRender original_zCWorldRender;
	zCWorldVobAddedToWorld original_zCWorldVobAddedToWorld;
	zCBspTreeAddVob original_zCBspTreeAddVob;
	oCWorldInsertVobInWorld original_oCWorldInsertVobInWorld;
	zCWorldLoadWorld original_zCWorldLoadWorld;
	oCGameEnterWorld original_oCGameEnterWorld;
	oCGameUpdatePlayerStatus original_oCGameUpdatePlayerStatus;
	zCWorldVobRemovedFromWorld original_zCWorldVobRemovedFromWorld;
	Alg_Rotation3DNRad original_Alg_Rotation3DNRad;
	GenericDestructor original_zCMaterialDestructor;
	GenericDestructor original_zCParticleFXDestructor;
	GenericDestructor original_zCVisualDestructor;
	zCMaterialConstruktor original_zCMaterialConstruktor;
	zCMaterialInitValues original_zCMaterialInitValues;
	zFILEOpen original_zFILEOpen;
	zCRnd_D3DVid_SetScreenMode original_zCRnd_D3DVid_SetScreenMode;
	zCRnd_D3D_DrawPoly original_zCRnd_D3D_DrawPoly;
	zCRnd_D3D_DrawPolySimple original_zCRnd_D3D_DrawPolySimple;
	zCOptionReadInt original_zCOptionReadInt;
	zCOptionReadBool original_zCOptionReadBool;
	zCOptionReadDWORD original_zCOptionReadDWORD;
	zCViewSetMode original_zCViewSetMode;
	zCViewFontSize original_zCViewFontSize;
	zCViewBlitText original_zCViewBlitText;
	zCViewPrint original_zCViewPrint;
	HandledWinMain original_HandledWinMain;
	CGameManagerExitGame original_CGameManagerExitGame;
	zCExceptionHandlerUnhandledExceptionFilter original_zCExceptionHandler_UnhandledExceptionFilter;
	GenericThiscall original_zCWorldDisposeWorld;
	zCWorldDisposeVobs original_zCWorldDisposeVobs;
	oCSpawnManagerSpawnNpc original_oCSpawnManagerSpawnNpc;
	oCSpawnManagerCheckRemoveNpc original_oCSpawnManagerCheckRemoveNpc;
	oCSpawnManagerCheckInsertNpc original_oCSpawnManagerCheckInsertNpc;
	zCVobSetVisual original_zCVobSetVisual;
	GenericDestructor original_zCVobDestructor;
	zCTex_D3DXTEX_BuildSurfaces original_zCTex_D3DXTEX_BuildSurfaces;
	zCTextureLoadResourceData ofiginal_zCTextureLoadResourceData;
	zCThreadSuspendThread original_zCThreadSuspendThread;
	zCResourceManagerCacheOut original_zCResourceManagerCacheOut;
	zCQuadMarkCreateQuadMark original_zCQuadMarkCreateQuadMark;
	GenericDestructor original_zCQuadMarkDestructor;
	GenericThiscall original_zCQuadMarkConstructor;
	oCSpawnManagerInsertNpc original_oCSpawnManagerInsertNpc;
	oCNPCEnable original_oCNPCEnable;
	GenericThiscall original_oCNPCDisable;
	GenericThiscall original_oCNPCInitModel;
	oCWorldDisableVob original_oCWorldDisableVob;
	oCWorldEnableVob original_oCWorldEnableVob;
	oCWorldRemoveVob original_oCWorldRemoveVob;
	oCWorldRemoveFromLists original_oCWorldRemoveFromLists;
	zCBinkPlayerOpenVideo original_zCBinkPlayerOpenVideo;
	zCVobEndMovement original_zCVobEndMovement;
	zCBspBaseCollectPolysInBBox3D original_zCBspBaseCollectPolysInBBox3D;
	zCBspBaseCheckRayAgainstPolys original_zCBspBaseCheckRayAgainstPolys;
	zCBspBaseCheckRayAgainstPolys original_zCBspBaseCheckRayAgainstPolysCache;
	zCBspBaseCheckRayAgainstPolys original_zCBspBaseCheckRayAgainstPolysNearestHit;
	//zCActiveSndAutoCalcObstruction original_zCActiveSndAutoCalcObstruction;
	zCModelPrototypeLoadModelASC original_zCModelPrototypeLoadModelASC;
	zCModelPrototypeReadMeshAndTreeMSB original_zCModelPrototypeReadMeshAndTreeMSB;

	/** Function hooks */
	static int __stdcall hooked_HandledWinMain( HINSTANCE hInstance, HINSTANCE hPrev, LPSTR szCmdLine, int sw );
	static void __cdecl hooked_ExitGameFunc();

	/** Unhandled exception handler */
	static long __stdcall hooked_zCExceptionHandlerUnhandledExceptionFilter( EXCEPTION_POINTERS* exceptionPtrs );

	/** Single function for making the bink-player working again */
	/** Returns the pixelformat of a bink-surface */
	static long __fastcall hooked_zBinkPlayerGetPixelFormat( void* thisptr, void* unknwn, zTRndSurfaceDesc& desc );
	static int __fastcall hooked_zBinkPlayerOpenVideo( void* thisptr, void* unknwn, zSTRING str );
	static void __fastcall hooked_zCActiveSndAutoCalcObstruction( void* thisptr, void* unknwn, int i );
};

namespace HookedFunctions {
	/** Holds all the original functions */
	__declspec(selectany) HookedFunctionInfo OriginalFunctions;
};