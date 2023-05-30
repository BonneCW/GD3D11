#pragma once
#include "zCVob.h"

class zCVobLight : public zCVob {
public:

    DWORD GetLightColor() {
        return *reinterpret_cast<DWORD*>(THISPTR_OFFSET( GothicMemoryLocations::zCVobLight::Offset_LightColor ));
    }

    float GetLightRange() {
        return *reinterpret_cast<float*>(THISPTR_OFFSET( GothicMemoryLocations::zCVobLight::Offset_Range ));
    }

    bool IsEnabled() {
        return *reinterpret_cast<DWORD*>(THISPTR_OFFSET( GothicMemoryLocations::zCVobLight::Offset_LightInfo )) & GothicMemoryLocations::zCVobLight::Mask_LightEnabled;
    }

    void DoAnimation() {
        reinterpret_cast<void( __fastcall* )( zCVobLight* )>( GothicMemoryLocations::zCVobLight::DoAnimation )( this );
    }

    bool IsStatic() {
        int flags = *reinterpret_cast<int*>(THISPTR_OFFSET( GothicMemoryLocations::zCVobLight::Offset_LightInfo ));
        return (flags & 1) != 0;
    }

#ifndef PUBLIC_RELEASE
    byte data1[0x140];
    float flt[10];
#endif
};
