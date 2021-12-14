#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCVob.h"
#include "zViewTypes.h"

enum oCNPCFlags : int
{
    NPC_FLAG_FRIEND = (1 << 0),
    NPC_FLAG_IMMORTAL = (1 << 1),
    NPC_FLAG_GHOST = (1 << 2)
};

class oCNPC : public zCVob {
public:
    static const zCClassDef* GetStaticClassDef() {
        return reinterpret_cast<const zCClassDef*>(GothicMemoryLocations::zCClassDef::oCNpc);
    }

    /** Hooks the functions of this Class */
    static void Hook() {
#ifdef BUILD_GOTHIC_1_08k
        // Only G1 needs this, because newly added NPCs only get enabled, but not re-added to the world like in G2
        //XHook(HookedFunctions::OriginalFunctions.original_oCNPCEnable, GothicMemoryLocations::oCNPC::Enable, oCNPC::hooked_oCNPCEnable);
#endif

        XHook( HookedFunctions::OriginalFunctions.original_oCNPCEnable, GothicMemoryLocations::oCNPC::Enable, oCNPC::hooked_oCNPCEnable );
        XHook( HookedFunctions::OriginalFunctions.original_oCNPCDisable, GothicMemoryLocations::oCNPC::Disable, oCNPC::hooked_oCNPCDisable );

        XHook( HookedFunctions::OriginalFunctions.original_oCNPCInitModel, GothicMemoryLocations::oCNPC::InitModel, oCNPC::hooked_oCNPCInitModel );
    }

    static void __fastcall hooked_oCNPCInitModel( void* thisptr, void* unknwn ) {
        hook_infunc
            HookedFunctions::OriginalFunctions.original_oCNPCInitModel( thisptr );

        if (/*((zCVob *)thisptr)->GetVisual() || */Engine::GAPI->GetSkeletalVobByVob( (zCVob*)thisptr ) ) {
            // This may causes the vob to be added and removed multiple times, but makes sure we get all changes of armor
            Engine::GAPI->OnRemovedVob( (zCVob*)thisptr, ((zCVob*)thisptr)->GetHomeWorld() );
            Engine::GAPI->OnAddVob( (zCVob*)thisptr, ((zCVob*)thisptr)->GetHomeWorld() );
        }
        hook_outfunc
    }

    /** Reads config stuff */
    static void __fastcall hooked_oCNPCEnable( void* thisptr, void* unknwn, DirectX::XMFLOAT3& position ) {
        hook_infunc
            HookedFunctions::OriginalFunctions.original_oCNPCEnable( thisptr, position );

        // Re-Add if needed
        Engine::GAPI->OnRemovedVob( (zCVob*)thisptr, ((zCVob*)thisptr)->GetHomeWorld() );
        Engine::GAPI->OnAddVob( (zCVob*)thisptr, ((zCVob*)thisptr)->GetHomeWorld() );
        hook_outfunc
    }

    static void __fastcall hooked_oCNPCDisable( void* thisptr, void* unknwn ) {
        hook_infunc

            // Remove vob from world
            if ( !((oCNPC*)thisptr)->IsAPlayer() ) // Never disable the player vob
                Engine::GAPI->OnRemovedVob( (zCVob*)thisptr, ((zCVob*)thisptr)->GetHomeWorld() );

        HookedFunctions::OriginalFunctions.original_oCNPCDisable( thisptr );

        hook_outfunc
    }

    void ResetPos( const DirectX::XMFLOAT3& pos ) {
        reinterpret_cast<void( __fastcall* )( oCNPC*, int, const DirectX::XMFLOAT3& )>( GothicMemoryLocations::oCNPC::ResetPos )( this, 0, pos );
    }

    int IsAPlayer() {
        return (this == oCGame::GetPlayer());
    }
    zSTRING GetName( int i = 0 ) {
        zSTRING str;
        reinterpret_cast<void( __fastcall* )( oCNPC*, int, zSTRING&, int )>( GothicMemoryLocations::oCNPC::GetName )( this, 0, str, i );
        return str;
    }
#ifndef BUILD_SPACER
    bool HasFlag( oCNPCFlags flag ) {
        return reinterpret_cast<bool( __fastcall* )( oCNPC*, int, oCNPCFlags )>( GothicMemoryLocations::oCNPC::HasFlag )( this, 0, flag );
    }
#else
    bool HasFlag( oCNPCFlags ) {
        return false;
    }
#endif
};

