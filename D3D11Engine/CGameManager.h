#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"

class CGameManager {
public:

    /** Hooks the functions of this Class */
    static void Hook() {
        //XHook(HookedFunctions::OriginalFunctions.original_CGameManagerExitGame, GothicMemoryLocations::CGameManager::ExitGame, CGameManager::hooked_ExitGame);
    }

    static int __fastcall hooked_ExitGame( void* thisptr, void* unknwn ) {
        HookedFunctions::OriginalFunctions.original_CGameManagerExitGame( thisptr );

        Engine::OnShutDown();

        return 1;
    }

};
