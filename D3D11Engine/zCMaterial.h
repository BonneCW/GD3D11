#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCTexture.h"

const int zMAT_GROUP_WATER = 5;
const int zMAT_GROUP_SNOW = 6;

const int zMAT_ALPHA_FUNC_MAT_DEFAULT = 0;
const int zMAT_ALPHA_FUNC_NONE = 1;
const int zMAT_ALPHA_FUNC_BLEND = 2;
const int zMAT_ALPHA_FUNC_ADD = 3;
const int zMAT_ALPHA_FUNC_SUB = 4;
const int zMAT_ALPHA_FUNC_MUL = 5;
const int zMAT_ALPHA_FUNC_MUL2 = 6;
const int zMAT_ALPHA_FUNC_TEST = 7;
const int zMAT_ALPHA_FUNC_BLEND_TEST = 8;

class zCTexAniCtrl {
private:
    int	AniChannel;
    float ActFrame;
    float AniFPS;
    DWORD FrameCtr;
    int	IsOneShotAni;
};

class zCMaterial {
public:
    /** Hooks the functions of this Class */
    static void Hook() {
        XHook( HookedFunctions::OriginalFunctions.original_zCMaterialDestructor, GothicMemoryLocations::zCMaterial::Destructor, zCMaterial::Hooked_Destructor );
        //XHook(HookedFunctions::OriginalFunctions.original_zCMaterialConstruktor, GothicMemoryLocations::zCMaterial::Constructor, zCMaterial::Hooked_Constructor);
        XHook( HookedFunctions::OriginalFunctions.original_zCMaterialInitValues, GothicMemoryLocations::zCMaterial::InitValues, zCMaterial::Hooked_InitValues );

    }

    static void __fastcall Hooked_Destructor( void* thisptr, void* unknwn ) {
        hook_infunc

            // Notify the world
            if ( Engine::GAPI )
                Engine::GAPI->OnMaterialDeleted( (zCMaterial*)thisptr );

        HookedFunctions::OriginalFunctions.original_zCMaterialDestructor( thisptr );

        hook_outfunc
    }

    static void __fastcall Hooked_Constructor( void* thisptr, void* unknwn ) {
        hook_infunc
            // Notify the world
            //Engine::GAPI->OnMaterialCreated((zCMaterial *)thisptr);

            HookedFunctions::OriginalFunctions.original_zCMaterialConstruktor( thisptr );

        hook_outfunc
    }

    static void __fastcall Hooked_InitValues( void* thisptr, void* unknwn ) {
        hook_infunc

            // Notify the world
            if ( Engine::GAPI )
                Engine::GAPI->OnMaterialCreated( (zCMaterial*)thisptr );

        HookedFunctions::OriginalFunctions.original_zCMaterialInitValues( thisptr );

        hook_outfunc
    }

    zCTexAniCtrl* GetTexAniCtrl() {
        return (zCTexAniCtrl*)(((char*)this) + GothicMemoryLocations::zCMaterial::Offset_TexAniCtrl);
    }

    /** Returns AniTexture - single animation channel */
    zCTexture* GetTexture() {
        zCTexture* texture = GetTextureSingle();
        if ( texture ) {
            unsigned char flags = *(unsigned char*)(((char*)texture) + GothicMemoryLocations::zCTexture::Offset_Flags);
            if ( flags & GothicMemoryLocations::zCTexture::Mask_FlagIsAnimated ) {
                reinterpret_cast<void( __fastcall* )(zCTexAniCtrl*, int, zCTexture* )>
                    ( GothicMemoryLocations::zCMaterial::AdvanceAni )( GetTexAniCtrl(), 0, texture );
                return GetCurrentTexture();
            }
        }
        return texture;
    }

    /** Returns the color-mod of this material */
    DWORD GetColor() {
        return *(DWORD*)THISPTR_OFFSET( GothicMemoryLocations::zCMaterial::Offset_Color );
    }

    /** Returns single texture, because not all seem to be animated and returned by GetAniTexture? */
    zCTexture* GetTextureSingle() {
        return *(zCTexture**)(((char*)this) + GothicMemoryLocations::zCMaterial::Offset_Texture);
    }

    /** Returns the current texture - single animation channel */
    zCTexture* GetCurrentTexture() {
        zCTexture* texture = GetTextureSingle();
        if ( texture ) {
            unsigned char flags = *(unsigned char*)(((char*)texture) + GothicMemoryLocations::zCTexture::Offset_Flags);
            if ( flags & GothicMemoryLocations::zCTexture::Mask_FlagIsAnimated ) {
                int animationChannel = *(int*)(((char*)this) + GothicMemoryLocations::zCMaterial::Offset_TexAniCtrl);
                int animationFrames = ((int*)(((char*)texture) + GothicMemoryLocations::zCTexture::Offset_AniFrames))[animationChannel];
                if ( animationFrames <= 0 )
                    return texture;

                zCTexture* tex = texture;
                int activeAnimationFrame = ((int*)(((char*)texture) + GothicMemoryLocations::zCTexture::Offset_ActAniFrame))[animationChannel];
                for ( int i = 0; i < activeAnimationFrame; ++i ) {
                    zCTexture* activeAnimationFrame = ((zCTexture**)(((char*)tex) + GothicMemoryLocations::zCTexture::Offset_NextFrame))[animationChannel];
                    if ( !activeAnimationFrame )
                        return tex;

                    tex = activeAnimationFrame;
                }
                return tex;
            }
        }
        return texture;
    }

    /** Returns the current texture from GetAniTexture */
    zCTexture* GetAniTexture() {
        return reinterpret_cast<zCTexture*( __fastcall* )( zCMaterial* )>( GothicMemoryLocations::zCMaterial::GetAniTexture )( this );
    }

    void BindTexture( int slot ) {
        if ( zCTexture* texture = GetAniTexture() ) {
            // Bind it
            if ( texture->CacheIn( 0.6f ) == zRES_CACHED_IN )
                texture->Bind( slot );
        }
    }

    void BindTextureSingle( int slot ) {
        if ( zCTexture* texture = GetTextureSingle() ) {
            // Bind it
            if ( texture->CacheIn( 0.6f ) == zRES_CACHED_IN )
                texture->Bind( slot );
        }
    }

    int GetAlphaFunc() {
        return static_cast<int>(*(unsigned char*)THISPTR_OFFSET( GothicMemoryLocations::zCMaterial::Offset_AlphaFunc ));
    }

    void SetAlphaFunc( int func ) {
        *(unsigned char*)THISPTR_OFFSET( GothicMemoryLocations::zCMaterial::Offset_AlphaFunc ) = static_cast<unsigned char>(func);
    }

    int GetMatGroup() {
        return *(int*)THISPTR_OFFSET( GothicMemoryLocations::zCMaterial::Offset_MatGroup );
    }

    bool HasAlphaTest() {
        int f = GetAlphaFunc();
        return f == zMAT_ALPHA_FUNC_TEST || f == zMAT_ALPHA_FUNC_BLEND_TEST;
    }

    bool HasTexAniMap() {
        return (*(unsigned char*)THISPTR_OFFSET( GothicMemoryLocations::zCMaterial::Offset_AlphaFunc )) & GothicMemoryLocations::zCMaterial::Mask_FlagTexAniMap;
    }

    DirectX::XMFLOAT2 GetTexAniMapDelta() {
        return *(DirectX::XMFLOAT2*)THISPTR_OFFSET( GothicMemoryLocations::zCMaterial::Offset_TexAniMapDelta );
    }
};

