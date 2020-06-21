#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "zCPolygon.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCVob.h"
#include "zViewTypes.h"

class oCNPC : public zCVob {
public:
	oCNPC();
	~oCNPC();

	/** Hooks the functions of this Class */
	static void Hook() {
#ifdef BUILD_GOTHIC_1_08k
		// Only G1 needs this, because newly added NPCs only get enabled, but not re-added to the world like in G2
		//XHook(HookedFunctions::OriginalFunctions.original_oCNPCEnable, GothicMemoryLocations::oCNPC::Enable, oCNPC::hooked_oCNPCEnable);
#endif

		XHook( HookedFunctions::OriginalFunctions.original_oCNPCEnable, GothicMemoryLocations::oCNPC::Enable, oCNPC::hooked_oCNPCEnable );
		XHook( HookedFunctions::OriginalFunctions.original_oCNPCDisable, GothicMemoryLocations::oCNPC::Disable, oCNPC::hooked_oCNPCDisable );

		XHook( HookedFunctions::OriginalFunctions.original_oCNPCInitModel, GothicMemoryLocations::oCNPC::InitModel, oCNPC::hooked_oCNPCInitModel );
	}

	static void __fastcall hooked_oCNPCInitModel( void* thisptr, void* unknwn ) {
		hook_infunc
			HookedFunctions::OriginalFunctions.original_oCNPCInitModel( thisptr );

		if (/*((zCVob *)thisptr)->GetVisual() || */Engine::GAPI->GetSkeletalVobByVob( (zCVob*)thisptr ) ) {
			// This may causes the vob to be added and removed multiple times, but makes sure we get all changes of armor
			Engine::GAPI->OnRemovedVob( (zCVob*)thisptr, ((zCVob*)thisptr)->GetHomeWorld() );
			Engine::GAPI->OnAddVob( (zCVob*)thisptr, ((zCVob*)thisptr)->GetHomeWorld() );
		}
		hook_outfunc
	}

	/** Reads config stuff */
	static void __fastcall hooked_oCNPCEnable( void* thisptr, void* unknwn, DirectX::XMFLOAT3& position ) {
		hook_infunc
			HookedFunctions::OriginalFunctions.original_oCNPCEnable( thisptr, position );

		// Re-Add if needed
		Engine::GAPI->OnRemovedVob( (zCVob*)thisptr, ((zCVob*)thisptr)->GetHomeWorld() );
		Engine::GAPI->OnAddVob( (zCVob*)thisptr, ((zCVob*)thisptr)->GetHomeWorld() );
		hook_outfunc
	}

	static void __fastcall hooked_oCNPCDisable( void* thisptr, void* unknwn ) {
		hook_infunc

			// Remove vob from world
			if ( !((oCNPC*)thisptr)->IsAPlayer() ) // Never disable the player vob
				Engine::GAPI->OnRemovedVob( (zCVob*)thisptr, ((zCVob*)thisptr)->GetHomeWorld() );

		HookedFunctions::OriginalFunctions.original_oCNPCDisable( thisptr );

		hook_outfunc
	}

	void ResetPos( const DirectX::XMFLOAT3& pos ) {
		XCALL( GothicMemoryLocations::oCNPC::ResetPos );
	}

	int IsAPlayer() {
		XCALL( GothicMemoryLocations::oCNPC::IsAPlayer );
	}
	zSTRING GetName( int i = 0 ) {
		XCALL( GothicMemoryLocations::oCNPC::GetName );
	}
#ifndef BUILD_GOTHIC_1_08k 
	int HasFlag( int ) { XCALL( 0x007309E0 ); }
#endif
};

