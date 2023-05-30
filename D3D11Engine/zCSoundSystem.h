#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"

class zCSoundSystem {
public:

    void SetGlobalReverbPreset( int preset, float weight ) {
#ifndef BUILD_GOTHIC_1_08k
        // Get vtable-entry
        DWORD* vtbl = reinterpret_cast<DWORD*>(*reinterpret_cast<DWORD*>(this));

        typedef void( __thiscall* pFun )(void*, int, float);

        pFun fn = reinterpret_cast<pFun>(vtbl[GothicMemoryLocations::zCSoundSystem::VTBL_SetGlobalReverbPreset]);
        fn( this, preset, weight );
#endif
    }

#ifndef BUILD_GOTHIC_1_08k
    static zCSoundSystem* GetSoundSystem() { return *reinterpret_cast<zCSoundSystem**>(GothicMemoryLocations::GlobalObjects::zSound); }
#else
    static zCSoundSystem* GetSoundSystem() { return nullptr; }
#endif
};
