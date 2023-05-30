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
        return reinterpret_cast<zCPolyStripInstance*>(THISPTR_OFFSET( GothicMemoryLocations::zCPolyStrip::Offset_Material ));
    }

    void SetVisibleSegments( float visibleFirst, float visibleLast ) {
        reinterpret_cast<void( __fastcall* )( zCPolyStrip*, int, float, float )>
            ( GothicMemoryLocations::zCPolyStrip::SetVisibleSegments )( this, 0, visibleFirst, visibleLast );
    }

    void AlighToCamera() {
        reinterpret_cast<void( __fastcall* )( zCPolyStrip* )>( GothicMemoryLocations::zCPolyStrip::AlignToCamera )( this );
    }

    void Render( void* a ) {
        reinterpret_cast<void( __fastcall* )( zCPolyStrip*, int, void* )>( GothicMemoryLocations::zCPolyStrip::Render )( this, 0, a );
    }
};
