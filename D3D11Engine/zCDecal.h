#pragma once

#include "HookedFunctions.h"
#include "zCMaterial.h"
#include "zCVisual.h"

struct DecalSettings {
    zCMaterial* DecalMaterial;
    DirectX::XMFLOAT2 DecalSize;
    DirectX::XMFLOAT2 DecalOffset;
    BOOL DecalTwoSided;
    BOOL IgnoreDayLight;
    BOOL DecalOnTop;
};

class zCDecal : public zCVisual {
public:
    DecalSettings* GetDecalSettings() {
        return (DecalSettings*)THISPTR_OFFSET( GothicMemoryLocations::zCDecal::Offset_DecalSettings );
    }

    bool GetAlphaTestEnabled() {
        int alphaFunc = GetDecalSettings()->DecalMaterial->GetAlphaFunc();
        return alphaFunc == zMAT_ALPHA_FUNC_NONE || alphaFunc == zMAT_ALPHA_FUNC_TEST;
    }
};
