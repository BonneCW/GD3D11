#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCArray.h"
#include "zCProgMeshProto.h"

#pragma pack (push, 1)
struct zTWeightEntry {
    float Weight;
    XMFLOAT3 VertexPosition;
    unsigned char NodeIndex;
};
#pragma pack (pop)

class zCOBBox3D;
class zCMeshSoftSkin : public zCProgMeshProto {
public:

    struct zTNodeWedgeNormal {
        XMFLOAT3 Normal;
        int				NodeIndex;
    };

    char* GetVertWeightStream() {
        return *reinterpret_cast<char**>(THISPTR_OFFSET( GothicMemoryLocations::zCMeshSoftSkin::Offset_VertWeightStream ));
    };
};
