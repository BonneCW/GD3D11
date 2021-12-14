#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"

class zCModelTexAniState {
public:
    void UpdateTexList() {
        reinterpret_cast<void( __fastcall* )( zCModelTexAniState* )>( GothicMemoryLocations::zCModelTexAniState::UpdateTexList )( this );
    }

    int	NumNodeTex;
    zCTexture** NodeTexList;
    int	ActAniFrames[8];
};
