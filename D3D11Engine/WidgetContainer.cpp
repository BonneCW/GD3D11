#include "WidgetContainer.h"

#include "Widget_TransRot.h"
#include "WorldObjects.h"

WidgetContainer::WidgetContainer() {
    Widgets.push_back( new Widget_TransRot( this ) );
}

WidgetContainer::~WidgetContainer() {
    Toolbox::DeleteElements( Widgets );
}

/** Renders the active widget */
void WidgetContainer::Render() {
    for ( auto it = Widgets.cbegin(); it != Widgets.cend(); ++it ) {
        (*it)->RenderWidget();
    }
}

/** Adds a selectiontarget */
void WidgetContainer::AddSelection( BaseVobInfo* vob ) {
    Selection.insert( vob );

    for ( auto it = Widgets.cbegin(); it != Widgets.cend(); ++it ) {
        (*it)->OnSelectionAdded( vob->Vob );
    }
}

/** Clears the selection */
void WidgetContainer::ClearSelection() {
    Selection.clear();
}

/** Removes a single object from the selection */
void WidgetContainer::RemoveSelection( BaseVobInfo* vob ) {
    Selection.erase( vob );
}

/** Returns the current selection-set */
std::set<BaseVobInfo*>& WidgetContainer::GetSelection() {
    return Selection;
}

/** Returns if one of the widgets is currently clicked */
bool WidgetContainer::IsWidgetClicked() const {
    for ( auto it = Widgets.cbegin(); it != Widgets.cend(); ++it ) {
        if ( (*it)->IsActive() )
            return true;
    }

    return false;
}

/** Called when the owning window got a message */
void WidgetContainer::OnWindowMessage( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
    for ( auto it = Widgets.cbegin(); it != Widgets.cend(); ++it ) {
        (*it)->OnWindowMessage( hWnd, msg, wParam, lParam );
    }
}
