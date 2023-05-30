#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"

struct zCMeshData {
    int				numPoly;
    int				numVert;
    int             numFeat;

    zCVertex** vertList;
    zCPolygon** polyList;
    zCVertFeature** featList;

    zCVertex* vertArray;
    zCPolygon* polyArray;
    zCVertFeature* featArray;
};

class zCMesh {
public:
    zCPolygon** GetPolygons() {
        return *reinterpret_cast<zCPolygon***>(THISPTR_OFFSET( GothicMemoryLocations::zCMesh::Offset_Polygons ));
    }

    int GetNumPolygons() {
        return *reinterpret_cast<int*>(THISPTR_OFFSET( GothicMemoryLocations::zCMesh::Offset_NumPolygons ));
    }
};
