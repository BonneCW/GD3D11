#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"

class zCRndD3D
{
public:

	/** Hooks the functions of this Class */
	static void Hook()
	{
		//HookedFunctions::OriginalFunctions.original_zCRnd_D3DVid_SetScreenMode = (zCRnd_D3DVid_SetScreenMode)DetourFunction((BYTE *)GothicMemoryLocations::zCRndD3D::VidSetScreenMode, (BYTE *)zCRndD3D::hooked_zCRndD3DVidSetScreenMode);
		DetourFunction((BYTE *)GothicMemoryLocations::zCRndD3D::DrawLineZ, (BYTE *)hooked_zCRndD3DDrawLineZ);
		HookedFunctions::OriginalFunctions.original_zCRnd_D3D_DrawPoly = (zCRnd_D3D_DrawPoly)DetourFunction((BYTE*)GothicMemoryLocations::zCRndD3D::DrawPoly, (BYTE*)hooked_zCRndD3DDrawPoly);
	}

	/** Overwritten to only accept windowed */
	static void __fastcall hooked_zCRndD3DVidSetScreenMode(void * thisptr, void * unknwn, int mode)
	{
		hook_infunc
		// Pass Windowed only.
		HookedFunctions::OriginalFunctions.original_zCRnd_D3DVid_SetScreenMode(thisptr, 1);

		hook_outfunc
	}

	/** Overwritten to only accept windowed */
	static void __fastcall hooked_zCRndD3DDrawLineZ(void * thisptr, void * unknwn, float x1, float y1, float z1, float x2, float y2, float z2, DWORD color)
	{
		// Do nothing here yet.
		// TODO: Implement!
	}

	static void __fastcall hooked_zCRndD3DDrawPoly(void* thisptr, void* unknwn, zCPolygon* poly) {
		// Check if it's a call from zCPolyStrip render(), if so
		// prevent original function from execution, since we only need
		// zCPolyStrip render() to do pre-render computations
		// Relevant assembly parts (G2):

		// DrawPoly call inside zCPolyStrip render():
		// .text:005BE18C                 push    edi
		// .text:005BE18D                 call    dword ptr[eax + 10h]
		// .text:005BE190

		// DrawPoly function:
		// .text:0064B260 ; void __thiscall zCRnd_D3D::DrawPoly(zCRnd_D3D *this, struct zCPolygon *)
		// .text:0064B260 ? DrawPoly@zCRnd_D3D@@UAEXPAVzCPolygon@@@Z proc near

		hook_infunc
			//LogInfo() << "Draw Poly Return Adress: " << _ReturnAddress();		
			void* polyStripReturnPointer = (void*)GothicMemoryLocations::zCPolyStrip::RenderDrawPolyReturn;
		if (_ReturnAddress() != polyStripReturnPointer) {
			HookedFunctions::OriginalFunctions.original_zCRnd_D3D_DrawPoly(thisptr, poly);
		}

		hook_outfunc
	}

	/*float GetGammaValue()
	{
		XCALL(GothicMemoryLocations::zCRndD3D::Vid_GetGammaCorrection);
	}*/

	//static zCRndD3D* GetRenderer(){return *(zCRndD3D**)GothicMemoryLocations::GlobalObjects::zRenderer;}
};