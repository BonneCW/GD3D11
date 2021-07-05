#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCObject.h"

struct zCPolyStripInstance {
    zCMaterial* material;

    zCVertex* vertList;
    zCPolygon* polyList;

    int numPoly;
    int numVert;

    float3* centerPointList;
    float* alphaList;
    float width;
    zCVob* connectedVob;
    zTBBox3D bbox3D;
    int camAlign;
#ifdef BUILD_GOTHIC_2_6_fix
    int heightCheck;
    int everyFrameUpdate;
    float heightBound;
#endif
    int firstSeg;
    int lastSeg;
    int numSeg;

    float visLastFrac;
    float visFirstFrac;

    float alphaFadeSpeed;
    float newAlphaFadeSpeed;
    float newAlpha;
    int lastDirSeg;
    float3 lastDirNormal;

    unsigned char localFOR : 1;
};



class zCPolyStrip : public zCObject {
public:
    zCPolyStripInstance* GetInstanceData() {
        return (zCPolyStripInstance*)THISPTR_OFFSET( GothicMemoryLocations::zCPolyStrip::Offset_Material );
    }

    void SetVisibleSegments( float visibleFirst, float visibleLast ) {
        XCALL( GothicMemoryLocations::zCPolyStrip::SetVisibleSegments );
    }

    void AlighToCamera() {
        XCALL( GothicMemoryLocations::zCPolyStrip::AlignToCamera );
    }

    void Render( void* ) {
        XCALL( GothicMemoryLocations::zCPolyStrip::Render );
    }
};
