#pragma once

#include "HookedFunctions.h"
#include "zCMaterial.h"
#include "zCVisual.h"

struct DecalSettings {
    zCMaterial* DecalMaterial;
    XMFLOAT2 DecalSize;
    XMFLOAT2 DecalOffset;
    BOOL DecalTwoSided;
    BOOL IgnoreDayLight;
    BOOL DecalOnTop;
};

class zCDecal : public zCVisual {
public:
    DecalSettings* GetDecalSettings() {
        return reinterpret_cast<DecalSettings*>(THISPTR_OFFSET( GothicMemoryLocations::zCDecal::Offset_DecalSettings ));
    }

    bool GetAlphaTestEnabled() {
        int alphaFunc = GetDecalSettings()->DecalMaterial->GetAlphaFunc();
        return alphaFunc == zMAT_ALPHA_FUNC_NONE || alphaFunc == zMAT_ALPHA_FUNC_TEST;
    }
};
