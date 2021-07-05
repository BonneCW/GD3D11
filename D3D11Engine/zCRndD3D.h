#pragma once
#include "pch.h"
#include "zTypes.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"

class zCRndD3D {
public:

    /** Hooks the functions of this Class */
    static void Hook() {
        XHook( GothicMemoryLocations::zCRndD3D::DrawLineZ, hooked_zCRndD3DDrawLineZ );
        XHook( GothicMemoryLocations::zCRndD3D::DrawLine, hooked_zCRndD3DDrawLine );

        XHook( HookedFunctions::OriginalFunctions.original_zCRnd_D3D_DrawPoly, GothicMemoryLocations::zCRndD3D::DrawPoly, hooked_zCRndD3DDrawPoly );
        XHook( HookedFunctions::OriginalFunctions.original_zCRnd_D3D_DrawPolySimple, GothicMemoryLocations::zCRndD3D::DrawPolySimple, hooked_zCRndD3DDrawPolySimple );
    }

    /** Draws a straight line from xyz1 to xyz2 */
    static void __fastcall hooked_zCRndD3DDrawLineZ( void* thisptr, void* unknwn, float x1, float y1, float z1, float x2, float y2, float z2, zColor color ) {
        // TODO: Implement occlusion culling for the lines.
        // TODO: Find out why lines are flickering

        auto lineRenderer = Engine::GraphicsEngine->GetLineRenderer();
        if ( lineRenderer ) {
            lineRenderer->AddLineDeferred( ScreenSpaceLine( XMFLOAT3( x1, y1, z1 ), color.dword ), ScreenSpaceLine( XMFLOAT3( x2, y2, z2 ), color.dword ) );
        }
    }

    static void __fastcall hooked_zCRndD3DDrawLine( void* thisptr, void* unknwn, float x1, float y1, float x2, float y2, zColor color ) {
        // TODO: Find out why lines are flickering

        if ( color.bgra.alpha == 0 ) {
            color.bgra.alpha = 255;
        }
        auto lineRenderer = Engine::GraphicsEngine->GetLineRenderer();
        if ( lineRenderer ) {
            lineRenderer->AddLineDeferred( ScreenSpaceLine( XMFLOAT3( x1, y1, 0 ), color.dword ), ScreenSpaceLine( XMFLOAT3( x2, y2, 0 ), color.dword ) );
        }
    }

    static void __fastcall hooked_zCRndD3DDrawPoly( void* thisptr, void* unknwn, zCPolygon* poly ) {
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
        if ( _ReturnAddress() != polyStripReturnPointer ) {
            HookedFunctions::OriginalFunctions.original_zCRnd_D3D_DrawPoly( thisptr, poly );
        }

        hook_outfunc
    }

    static void __fastcall hooked_zCRndD3DDrawPolySimple( void* thisptr, void* unknwn, zCTexture* texture, zTRndSimpleVertex* zTRndSimpleVertex, int iVal ) {

        hook_infunc

            HookedFunctions::OriginalFunctions.original_zCRnd_D3D_DrawPolySimple( thisptr, texture, zTRndSimpleVertex, iVal );

        hook_outfunc
    }

    void ResetRenderState() {
        // Set render state values to some absurd high value so that they will be changed by engine for sure
        *(DWORD*)THISPTR_OFFSET( GothicMemoryLocations::zCRndD3D::Offset_BoundTexture + ( /*TEX0*/0 * 4) ) = 0x00000000;
        *(DWORD*)THISPTR_OFFSET( GothicMemoryLocations::zCRndD3D::Offset_RenderState + ( /*D3DRENDERSTATE_ALPHABLENDENABLE*/27 * 4 ) ) = 0xFFFFFFFF;
        *(DWORD*)THISPTR_OFFSET( GothicMemoryLocations::zCRndD3D::Offset_RenderState + ( /*D3DRENDERSTATE_SRCBLEND*/19 * 4 ) ) = 0xFFFFFFFF;
        *(DWORD*)THISPTR_OFFSET( GothicMemoryLocations::zCRndD3D::Offset_RenderState + ( /*D3DRENDERSTATE_DESTBLEND*/20 * 4 ) ) = 0xFFFFFFFF;
    }

    /*float GetGammaValue()
    {
        XCALL(GothicMemoryLocations::zCRndD3D::Vid_GetGammaCorrection);
    }*/

    static zCRndD3D* GetRenderer() {
        return *(zCRndD3D**)GothicMemoryLocations::GlobalObjects::zRenderer;
    }
};
