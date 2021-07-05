#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "BaseGraphicsEngine.h"
#include "oCGame.h"
#include "zCVob.h"
#include "oCNPC.h"

class oCSpawnManager {
public:

    /** Hooks the functions of this Class */
    static void Hook() {
        XHook( HookedFunctions::OriginalFunctions.original_oCSpawnManagerSpawnNpc, GothicMemoryLocations::oCSpawnManager::SpawnNpc, oCSpawnManager::hooked_oCSpawnManagerSpawnNpc );
        //XHook(HookedFunctions::OriginalFunctions.original_oCSpawnManagerCheckInsertNpc, GothicMemoryLocations::oCSpawnManager::CheckInsertNpc, oCSpawnManager::hooked_oCSpawnManagerCheckInsertNpc);

        // TODO: #8
        //XHook(HookedFunctions::OriginalFunctions.original_oCSpawnManagerCheckRemoveNpc, GothicMemoryLocations::oCSpawnManager::CheckRemoveNpc, oCSpawnManager::hooked_oCSpawnManagerCheckRemoveNpc);
    }

    /** Reads config stuff */
    static void __fastcall hooked_oCSpawnManagerSpawnNpc( void* thisptr, void* unknwn, oCNPC* npc, const DirectX::XMFLOAT3& position, float f ) {
        hook_infunc
            HookedFunctions::OriginalFunctions.original_oCSpawnManagerSpawnNpc( thisptr, npc, position, f );

        if ( npc->GetSleepingMode() != 0 || npc->IsAPlayer() ) {
            Engine::GAPI->OnRemovedVob( (zCVob*)npc, ((zCVob*)npc)->GetHomeWorld() );
            Engine::GAPI->OnAddVob( (zCVob*)npc, ((zCVob*)npc)->GetHomeWorld() );
        }
        hook_outfunc
    }

    static int __fastcall hooked_oCSpawnManagerCheckRemoveNpc( void* thisptr, void* unknwn, oCNPC* npc ) {
        hook_infunc
            Engine::GAPI->SetCanClearVobsByVisual();
        auto res = HookedFunctions::OriginalFunctions.original_oCSpawnManagerCheckRemoveNpc( thisptr, npc );
        Engine::GAPI->SetCanClearVobsByVisual( false );
        return res;
        hook_outfunc

            return 0;
    }

    /** Reads config stuff */
    static void __fastcall hooked_oCSpawnManagerCheckInsertNpc( void* thisptr, void* unknwn ) {
        hook_infunc
            HookedFunctions::OriginalFunctions.original_oCSpawnManagerCheckInsertNpc( thisptr );
        hook_outfunc
    }
};



