#pragma once
#include "pch.h"
#include "zTypes.h"
#include "HookedFunctions.h"
#include "oCGame.h"
#include "zViewTypes.h"


class zCViewDraw {
public:
    static void Hook() {
        XHook( HookedFunctions::OriginalFunctions.original_zCViewDrawGetScreen, GothicMemoryLocations::zCViewDraw::GetScreen, GetScreen );
    }
    
    static zCViewDraw& GetScreen() {
        hook_infunc
            zCViewDraw& viewDraw = HookedFunctions::OriginalFunctions.original_zCViewDrawGetScreen();
            return viewDraw;
        hook_outfunc
    }

    void __fastcall SetVirtualSize(POINT& pt) {
        XCALL( GothicMemoryLocations::zCViewDraw::SetVirtualSize );
    }
};

class zCView {
public:

    /** Hooks the functions of this Class */
    static void Hook() {
        zCViewDraw::Hook();
        DWORD dwProtect;

        HookedFunctions::OriginalFunctions.original_zCViewSetMode = (zCViewSetMode)GothicMemoryLocations::zCView::SetMode;

        VirtualProtect( (void*)GothicMemoryLocations::zCView::SetMode, 0x1B9, PAGE_EXECUTE_READWRITE, &dwProtect ); // zCView::SetMode

        // Replace the actual mode-change in zCView::SetMode. Only do the UI-Changes.
        REPLACE_RANGE( GothicMemoryLocations::zCView::REPL_SetMode_ModechangeStart, GothicMemoryLocations::zCView::REPL_SetMode_ModechangeEnd - 1, INST_NOP );

#if BUILD_GOTHIC_2_6_fix
        // .text:007A62A0; void __thiscall zCView::BlitText(zCView * __hidden this)
        XHook( HookedFunctions::OriginalFunctions.original_zCViewBlitText, GothicMemoryLocations::zCView::BlitText, hooked_BlitText );
        XHook( HookedFunctions::OriginalFunctions.original_zCViewPrint, GothicMemoryLocations::zCView::Print, hooked_Print );
        //XHook( HookedFunctions::OriginalFunctions.original_zCViewBlit, GothicMemoryLocations::zCView::Blit, hooked_Blit );
#endif
    }

    static void SetMode( int x, int y, int bpp, HWND* window = nullptr ) {
        hook_infunc
            HookedFunctions::OriginalFunctions.original_zCViewSetMode( x, y, bpp, window );
        hook_outfunc
    }

#if BUILD_GOTHIC_2_6_fix
    /*
    static void __fastcall hooked_Blit(_zCView* thisptr, void* unknwn) {

        if (true || !Engine::GAPI->GetRendererState().RendererSettings.EnableCustomFontRendering) {
            HookedFunctions::OriginalFunctions.original_zCViewBlit(thisptr);
            return;
        }

        if (thisptr->viewID == 1 /* VIEW_VIEWPORT *\/) return;
        if (thisptr == GetScreen()) return;
        hook_infunc
            auto oldDepthState = Engine::GAPI->GetRendererState().DepthState.Clone();

            if (!thisptr->m_bFillZ) Engine::GAPI->GetRendererState().DepthState.DepthWriteEnabled = false;
            else Engine::GAPI->GetRendererState().DepthState.DepthWriteEnabled = true;

            Engine::GAPI->GetRendererState().DepthState.DepthBufferCompareFunc = GothicDepthBufferStateInfo::CF_COMPARISON_ALWAYS;
            Engine::GAPI->GetRendererState().DepthState.SetDirty();

            Engine::GraphicsEngine->UpdateRenderStates();

            HookedFunctions::OriginalFunctions.original_zCViewBlit(thisptr);

            oldDepthState.ApplyTo(Engine::GAPI->GetRendererState().DepthState);
            Engine::GraphicsEngine->UpdateRenderStates();
        hook_outfunc
    }
    */

    static void __fastcall hooked_Print( _zCView* thisptr, void* unknwn, int x, int y, const zSTRING& s ) {
        if ( !Engine::GAPI->GetRendererState().RendererSettings.EnableCustomFontRendering ) {
            HookedFunctions::OriginalFunctions.original_zCViewPrint( thisptr, x, y, s );
            return;
        }
        if ( !thisptr->font ) return;
        thisptr->scrollTimer = 0;

        // Instantly blit Viewport/global-screen
        if ( (thisptr->viewID == 1)
            || (thisptr == GetScreen()) ) {
            Engine::GraphicsEngine->DrawString(
                s.ToChar(),
                thisptr->pposx + thisptr->nax( x ), thisptr->pposy + thisptr->nay( y ),
                thisptr->font, thisptr->fontColor );
        } else {
            // create a textview for later blitting
            thisptr->CreateText( x, y, s );
        }
    }

    static void __fastcall hooked_BlitText( _zCView* thisptr, void* unknwn ) {
        if ( !Engine::GAPI->GetRendererState().RendererSettings.EnableCustomFontRendering ) {
            HookedFunctions::OriginalFunctions.original_zCViewBlitText( thisptr );
            return;
        }

        thisptr->CheckAutoScroll();
        thisptr->CheckTimedText();

        if ( !thisptr->isOpen ) return;

        float x, y;

        zCList <zCViewText>* textNode = thisptr->textLines.next;
        zCViewText* text = nullptr;
        zColor fontColor = thisptr->fontColor;
        while ( textNode ) {

            text = textNode->data;
            textNode = textNode->next;

            if ( text->colored ) { fontColor = text->color; }
            //else                 { fontColor = thisptr->fontColor;}

            x = thisptr->pposx + thisptr->nax( text->posx );
            // TODO: Remove additional addition if we get the correct char positioning
            y = thisptr->pposy + thisptr->nay( text->posy );

            if ( !thisptr->font ) continue;

            Engine::GraphicsEngine->DrawString( text->text.ToChar(), x, y, text->font, fontColor );
        }
    }
    static _zCView* GetScreen() { return *(_zCView**)GothicMemoryLocations::GlobalObjects::screen; }

#endif

    /** Prints a message to the screen */
    void PrintTimed( int posX, int posY, const zSTRING& strMessage, float time = 3000.0f, DWORD* col = nullptr ) {
        XCALL( GothicMemoryLocations::zCView::PrintTimed );
    }
};
