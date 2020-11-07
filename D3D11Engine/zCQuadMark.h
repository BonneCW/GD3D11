#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zSTRING.h"

class zCQuadMark {
public:

	/** Hooks the functions of this Class */
	static void Hook() {
		XHook( HookedFunctions::OriginalFunctions.original_zCQuadMarkCreateQuadMark, GothicMemoryLocations::zCQuadMark::CreateQuadMark, zCQuadMark::Hooked_CreateQuadMark );
		//XHook(HookedFunctions::OriginalFunctions.original_zCQuadMarkConstructor, GothicMemoryLocations::zCQuadMark::Constructor, zCQuadMark::Hooked_Constructor);
		XHook( HookedFunctions::OriginalFunctions.original_zCQuadMarkDestructor, GothicMemoryLocations::zCQuadMark::Destructor, zCQuadMark::Hooked_Destructor );
	}

	static void __fastcall Hooked_CreateQuadMark( void* thisptr, void* unknwn, zCPolygon* poly, const float3& position, const float2& size, struct zTEffectParams* params ) {
		hook_infunc

			if ( ((zCQuadMark*)thisptr)->GetDontRepositionConnectedVob() )
				return; // Don't create quad-marks for particle-effects because it's kinda slow at the moment
						// And even for the original game using some emitters? (L'Hiver Light, Swampdragon)

		HookedFunctions::OriginalFunctions.original_zCQuadMarkCreateQuadMark( thisptr, poly, position, size, params );

		QuadMarkInfo* info = Engine::GAPI->GetQuadMarkInfo( (zCQuadMark*)thisptr );

		WorldConverter::UpdateQuadMarkInfo( info, (zCQuadMark*)thisptr, position );

		if ( !info->Mesh )
			Engine::GAPI->RemoveQuadMark( (zCQuadMark*)thisptr );

		hook_outfunc
	}

	static void __fastcall Hooked_Constructor( void* thisptr, void* unknwn ) {
		hook_infunc

			HookedFunctions::OriginalFunctions.original_zCQuadMarkConstructor( thisptr );

		hook_outfunc
	}

	static void __fastcall Hooked_Destructor( void* thisptr, void* unknwn ) {
		hook_infunc

			HookedFunctions::OriginalFunctions.original_zCQuadMarkDestructor( thisptr );

		Engine::GAPI->RemoveQuadMark( (zCQuadMark*)thisptr );

		hook_outfunc
	}

	zCMesh* GetQuadMesh() {
		return *(zCMesh**)THISPTR_OFFSET( GothicMemoryLocations::zCQuadMark::Offset_QuadMesh );
	}

	zCMaterial* GetMaterial() {
		return *(zCMaterial**)THISPTR_OFFSET( GothicMemoryLocations::zCQuadMark::Offset_Material );
	}

	zCVob* GetConnectedVob() {
		return *(zCVob**)THISPTR_OFFSET( GothicMemoryLocations::zCQuadMark::Offset_ConnectedVob );
	}

	/** This gets only set for quad-marks created by a particle-effect. */
	int GetDontRepositionConnectedVob() {
		return *(int*)THISPTR_OFFSET( GothicMemoryLocations::zCQuadMark::Offset_DontRepositionConnectedVob );
	}
};