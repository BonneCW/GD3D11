#pragma once
#include "D2DSubView.h"

const float SV_CHKBOX_BOX_SIZE_XY = 13;
class SV_Checkbox;
typedef void( __cdecl* SV_CheckboxCheckedChangedCallback )(SV_Checkbox*, void*);

class SV_Checkbox : public D2DSubView {
public:
    SV_Checkbox( D2DView* view, D2DSubView* parent );
    ~SV_Checkbox();

    /** Draws this sub-view */
    virtual void Draw( const D2D1_RECT_F& clientRectAbs, float deltaTime );

    /** Sets the position and size of this sub-view */
    virtual void SetRect( const D2D1_RECT_F& rect );

    /** Sets the state */
    virtual void SetChecked( bool checked );

    /** Returns the state */
    bool GetChecked() const;

    /** Sets this buttons caption */
    void SetCaption( const std::wstring& caption );

    /** Sets the data location to update with this checkbox */
    void SetDataToUpdate( bool* data );

    /** Sets the callback */
    void SetCheckedChangedCallback( SV_CheckboxCheckedChangedCallback cb, void* userdata );

    /** Processes a window-message. Return false to stop the message from going to children */
    virtual bool OnWindowMessage( HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam, const D2D1_RECT_F& clientRectAbs );

protected:
    /** Draws the cross */
    void DrawCross( const D2D1_RECT_F& r );

    /** Wether this button is currently pressed or not */
    bool IsChecked;
    bool* DataToUpdate;

    /** Current Caption */
    std::wstring Caption;

    /** Text layout */
    IDWriteTextLayout* CaptionLayout;

    SV_CheckboxCheckedChangedCallback CheckedChangedCallback;
    void* CheckedChangedUserdata;
};
