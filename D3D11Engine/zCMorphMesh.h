#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCModelTexAniState.h"


class zCMorphMesh {
public:
    zCProgMeshProto* GetMorphMesh() {
        return *(zCProgMeshProto**)THISPTR_OFFSET( GothicMemoryLocations::zCMorphMesh::Offset_MorphMesh );
    }

    zCModelTexAniState* GetTexAniState() {
        return (zCModelTexAniState*)THISPTR_OFFSET( GothicMemoryLocations::zCMorphMesh::Offset_TexAniState );
    }

    void CalcVertexPositions() {
        reinterpret_cast<void( __fastcall* )( zCMorphMesh* )>( GothicMemoryLocations::zCMorphMesh::CalcVertexPositions )( this );
    }

    void AdvanceAnis() {
        reinterpret_cast<void( __fastcall* )( zCMorphMesh* )>( GothicMemoryLocations::zCMorphMesh::AdvanceAnis )( this );
    }

    void UpdateBuffer( D3D11VertexBuffer* buffer ) {
    }
};
