#pragma once
#include "pch.h"
#include "HookedFunctions.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "BaseGraphicsEngine.h"
#include "zCTree.h"
#include "zCCamera.h"
#include "zCVob.h"
#include "zCCamera.h"
#include "zCSkyController_Outdoor.h"

class zCCamera;
class zCSkyController_Outdoor;
class zCSkyController;

class zCWorld {
public:
	/** Hooks the functions of this Class */
	static void Hook() {
		XHook( HookedFunctions::OriginalFunctions.original_zCWorldRender, GothicMemoryLocations::zCWorld::Render, zCWorld::hooked_Render );
		XHook( HookedFunctions::OriginalFunctions.original_zCWorldVobAddedToWorld, GothicMemoryLocations::zCWorld::VobAddedToWorld, zCWorld::hooked_VobAddedToWorld );

		XHook( HookedFunctions::OriginalFunctions.original_zCWorldLoadWorld, GothicMemoryLocations::zCWorld::LoadWorld, zCWorld::hooked_LoadWorld );
		XHook( HookedFunctions::OriginalFunctions.original_zCWorldVobRemovedFromWorld, GothicMemoryLocations::zCWorld::VobRemovedFromWorld, zCWorld::hooked_zCWorldVobRemovedFromWorld );
		//XHook(HookedFunctions::OriginalFunctions.original_zCWorldDisposeWorld, GothicMemoryLocations::zCWorld::DisposeWorld, zCWorld::hooked_zCWorldDisposeWorld);
		XHook( HookedFunctions::OriginalFunctions.original_zCWorldDisposeVobs, GothicMemoryLocations::zCWorld::DisposeVobs, zCWorld::hooked_zCWorldDisposeVobs );

#ifdef BUILD_GOTHIC_1_08k
		XHook( HookedFunctions::OriginalFunctions.original_oCWorldInsertVobInWorld, GothicMemoryLocations::oCWorld::InsertVobInWorld, zCWorld::hooked_InsertVobInWorld );
#endif

		XHook( HookedFunctions::OriginalFunctions.original_oCWorldRemoveFromLists, GothicMemoryLocations::oCWorld::RemoveFromLists, zCWorld::hooked_oCWorldRemoveFromLists );
		XHook( HookedFunctions::OriginalFunctions.original_oCWorldEnableVob, GothicMemoryLocations::oCWorld::EnableVob, zCWorld::hooked_oCWorldEnableVob );
		XHook( HookedFunctions::OriginalFunctions.original_oCWorldDisableVob, GothicMemoryLocations::oCWorld::DisableVob, zCWorld::hooked_oCWorldDisableVob );
		XHook( HookedFunctions::OriginalFunctions.original_oCWorldRemoveVob, GothicMemoryLocations::oCWorld::RemoveVob, zCWorld::hooked_oCWorldRemoveVob );
	}

	static void __fastcall hooked_oCWorldEnableVob( void* thisptr, void* unknwn, zCVob* vob, zCVob* parent ) {
		hook_infunc

			HookedFunctions::OriginalFunctions.original_oCWorldEnableVob( thisptr, vob, parent );

		// Re-Add it
		Engine::GAPI->OnAddVob( vob, (zCWorld*)thisptr );

		hook_outfunc
	}

	static void __fastcall hooked_oCWorldDisableVob( void* thisptr, void* unknwn, zCVob* vob ) {
		hook_infunc

			// Remove it
			Engine::GAPI->OnRemovedVob( vob, (zCWorld*)thisptr );

		HookedFunctions::OriginalFunctions.original_oCWorldDisableVob( thisptr, vob );
		hook_outfunc
	}

	static void __fastcall hooked_oCWorldRemoveVob( void* thisptr, void* unknwn, zCVob* vob ) {
		hook_infunc
			//Engine::GAPI->SetCanClearVobsByVisual(); // TODO: #8
			HookedFunctions::OriginalFunctions.original_oCWorldRemoveVob( thisptr, vob );
		//Engine::GAPI->SetCanClearVobsByVisual(false); // TODO: #8
		hook_outfunc
	}

	static void __fastcall hooked_oCWorldRemoveFromLists( void* thisptr, zCVob* vob ) {
		hook_infunc

			// Remove it
			Engine::GAPI->OnRemovedVob( vob, (zCWorld*)thisptr );

		HookedFunctions::OriginalFunctions.original_oCWorldRemoveFromLists( thisptr, vob );
		hook_outfunc
	}

	static void __fastcall hooked_zCWorldDisposeWorld( void* thisptr, void* unknwn ) {
		//Engine::GAPI->ResetWorld();
		HookedFunctions::OriginalFunctions.original_zCWorldDisposeWorld( thisptr );
	}

	static void __fastcall hooked_zCWorldDisposeVobs( void* thisptr, void* unknwn, zCTree<zCVob>* tree ) {
		// Reset only if this is the main world, inventory worlds are handled differently
		if ( (zCWorld*)thisptr == Engine::GAPI->GetLoadedWorldInfo()->MainWorld )
			Engine::GAPI->ResetVobs();

		HookedFunctions::OriginalFunctions.original_zCWorldDisposeVobs( thisptr, tree );
	}

	static void __fastcall hooked_zCWorldVobRemovedFromWorld( void* thisptr, void* unknwn, zCVob* vob ) {
		hook_infunc
			// Remove it first, before it becomes invalid
			Engine::GAPI->OnRemovedVob( vob, (zCWorld*)thisptr );

		HookedFunctions::OriginalFunctions.original_zCWorldVobRemovedFromWorld( thisptr, vob );
		hook_outfunc
	}

	static void __fastcall hooked_LoadWorld( void* thisptr, void* unknwn, const zSTRING& fileName, const int loadMode ) {
		//hook_infunc
		//LogInfo() << "Loading: " << fileName.ToChar();

		//if (loadMode != 1)
		//	Engine::GAPI->ResetWorld();

		Engine::GAPI->OnLoadWorld( fileName.ToChar(), loadMode );

		HookedFunctions::OriginalFunctions.original_zCWorldLoadWorld( thisptr, fileName, loadMode );

		Engine::GAPI->GetLoadedWorldInfo()->MainWorld = (zCWorld*)thisptr;

		//LogInfo() << "Loaded world: " << fileName.ToChar();

		//Engine::GAPI->OnWorldLoaded();
		//hook_outfunc
	}

	static void __fastcall hooked_InsertVobInWorld( void* thisptr, void* unknwn, zCVob* vob ) {
		hook_infunc
			HookedFunctions::OriginalFunctions.original_oCWorldInsertVobInWorld( thisptr, vob );
		if ( vob->GetVisual() ) {
			//LogInfo() << vob->GetVisual()->GetFileExtension(0);
			//Engine::GAPI->OnAddVob(vob); 
			Engine::GAPI->OnAddVob( vob, (zCWorld*)thisptr );
		}
		hook_outfunc
	}

	static void __fastcall hooked_VobAddedToWorld( void* thisptr, void* unknwn, zCVob* vob ) {
		hook_infunc

			HookedFunctions::OriginalFunctions.original_zCWorldVobAddedToWorld( thisptr, vob );

		if ( vob->GetVisual() ) {
			//LogInfo() << vob->GetVisual()->GetFileExtension(0);
			Engine::GAPI->OnAddVob( vob, (zCWorld*)thisptr );
		}
		hook_outfunc
	}

	// Get around C2712
	static void Do_hooked_Render( void* thisptr, zCCamera& camera ) {
		Engine::GAPI->SetTextureTestBindMode( false, "" );

		//HookedFunctions::OriginalFunctions.original_zCWorldRender(thisptr, camera);
		if ( thisptr == Engine::GAPI->GetLoadedWorldInfo()->MainWorld ) {
			Engine::GAPI->OnWorldUpdate();

			// Main world
			if ( Engine::GAPI->GetRendererState().RendererSettings.AtmosphericScattering ) {
				HookedFunctions::OriginalFunctions.original_zCWorldRender( thisptr, camera );
				//zCWorld::fake_zCWorldRender(thisptr, camera);
			} else {
				camera.SetFarPlane( 25000.0f );
				HookedFunctions::OriginalFunctions.original_zCWorldRender( thisptr, camera );
			}

			/*zCWorld* w = (zCWorld *)thisptr;
			zCSkyController* sky = w->GetActiveSkyController();
			sky->RenderSkyPre();*/
		} else {
			// Bind matrices
			//camera.UpdateViewport();
			//camera.Activate();

			// This needs to be called to init the camera and everything for the inventory vobs
			// The PresentPending-Guard will stop the renderer from rendering the world into one of the cells here
			// TODO: This can be implemented better.
			HookedFunctions::OriginalFunctions.original_zCWorldRender( thisptr, camera );

			// Inventory
			Engine::GAPI->DrawInventory( (zCWorld*)thisptr, camera );
		}
	}

	static void __fastcall hooked_Render( void* thisptr, void* unknwn, zCCamera& camera ) {
		hook_infunc
			Do_hooked_Render( thisptr, camera );
		hook_outfunc
	}

	static void fake_zCWorldRender( void* thisptr, zCCamera& camera ) {
		zCWorld* world = (zCWorld*)thisptr;

		unsigned char originalzCWorldRender[0x255];
		unsigned char* worldRender = (unsigned char*)GothicMemoryLocations::zCWorld::Render;

		memcpy( originalzCWorldRender, worldRender, 0x255 );

		// zCBspTree::Render()
		//REPLACE_CALL(GothicMemoryLocations::zCWorld::Call_Render_zCBspTreeRender, INST_NOP);

		// Increase farplane 
		//camera.SetFarPlane(80000.0f); // Does this even do something

		HookedFunctions::OriginalFunctions.original_zCWorldRender( thisptr, camera );

		/*D3DXVECTOR4 cc = D3DXVECTOR4(Engine::GAPI->GetRendererState().GraphicsState.FF_FogColor.x,
			Engine::GAPI->GetRendererState().GraphicsState.FF_FogColor.y,
			Engine::GAPI->GetRendererState().GraphicsState.FF_FogColor.z, 0);

		Engine::GraphicsEngine->Clear(*(float4 *)&cc);*/

		memcpy( worldRender, originalzCWorldRender, 0x255 );
	}

	zCTree<zCVob>* GetGlobalVobTree() {
		return (zCTree<zCVob>*)THISPTR_OFFSET( GothicMemoryLocations::zCWorld::Offset_GlobalVobTree );
	}

	void Render( zCCamera& camera ) {
		XCALL( GothicMemoryLocations::zCWorld::Render );
	}

	/*zCSkyController* GetActiveSkyController()
	{
		XCALL(GothicMemoryLocations::zCWorld::GetActiveSkyController);
	}*/

	zCSkyController_Outdoor* GetSkyControllerOutdoor() {
		return *(zCSkyController_Outdoor**)(((char*)this) + GothicMemoryLocations::zCWorld::Offset_SkyControllerOutdoor);
	}

	zCBspTree* GetBspTree() {
		return (zCBspTree*)THISPTR_OFFSET( GothicMemoryLocations::zCWorld::Offset_BspTree );
	}

	void RemoveVob(zCVob* vob) {
		XCALL(GothicMemoryLocations::zCWorld::RemoveVob);
	}
};
