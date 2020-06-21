#include "pch.h"
#include "GInventory.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCMaterial.h"

GInventory::GInventory() {}


GInventory::~GInventory() {}

/** Called when a VOB got added to the BSP-Tree or the world */
void GInventory::OnAddVob( VobInfo* vob, zCWorld* world ) {
	InventoryVobs[world].push_back( vob );
}

/** Called when a VOB got removed from the world */
bool GInventory::OnRemovedVob( zCVob* vob, zCWorld* world ) {
	for ( std::list<VobInfo*>::iterator it = InventoryVobs[world].begin(); it != InventoryVobs[world].end(); ++it ) {
		//if ((*it)->Vob == vob)
		// There is only one VOB inside this anyways and the pointers seem to change
		// TODO: Investigate!
		{
			delete* it;
			InventoryVobs[world].erase( it );
			return true;
		}
	}

	return false;
}

/** Draws the inventory for the given world */
void GInventory::DrawInventory( zCWorld* world, zCCamera& camera ) {
	Engine::GraphicsEngine->DrawVobsList( InventoryVobs[world], camera );
}
