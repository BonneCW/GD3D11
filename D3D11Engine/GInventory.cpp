#include "pch.h"
#include "GInventory.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCMaterial.h"

GInventory::GInventory() {}

GInventory::~GInventory() {}

/** Called when a VOB got added to the BSP-Tree or the world */
void GInventory::OnAddVob( VobInfo* vob, zCWorld* world ) {
    auto it = InventoryVobs.find( world );
    if ( it != InventoryVobs.end() ) {
        it->second.reset( vob );
    } else {
        InventoryVobs.emplace( world, vob );
    }
}

/** Called when a VOB got removed from the world */
bool GInventory::OnRemovedVob( zCVob* vob, zCWorld* world ) {
    auto it = InventoryVobs.find( world );
    if ( it != InventoryVobs.end() ) {
        InventoryVobs.erase( it );
        return true;
    }

    return false;
}

/** Draws the inventory for the given world */
void GInventory::DrawInventory( zCWorld* world, zCCamera& camera ) {
    auto it = InventoryVobs.find( world );
    if ( it != InventoryVobs.end() ) {
        Engine::GraphicsEngine->DrawVobSingle( it->second.get(), camera);
    }
}
