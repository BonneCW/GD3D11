#pragma once
#include "../pch.h"
#include <ddraw.h>

class MyClipper : public IDirectDrawClipper {
public:
	MyClipper() {
		refCount = 1;
	}

	/*** IUnknown methods ***/
	HRESULT __stdcall QueryInterface( THIS_ REFIID riid, LPVOID FAR* ppvObj ) {
		return S_OK;
	}

	ULONG __stdcall AddRef() {
		return ++refCount;
	}

	ULONG __stdcall Release() {
		if ( --refCount == 0 ) {
			delete this;
			return 0;
		}

		return refCount;
	}

	/*** IDirectDrawClipper methods ***/
	HRESULT __stdcall GetClipList( THIS_ LPRECT x, LPRGNDATA y, LPDWORD z ) {
		return S_OK;
	}

	HRESULT __stdcall GetHWnd( HWND* handle ) {
		*handle = hWnd;
		return S_OK;
	}

	HRESULT __stdcall Initialize( THIS_ LPDIRECTDRAW x, DWORD y ) {
		return S_OK;
	}

	HRESULT __stdcall IsClipListChanged( THIS_ BOOL FAR* x ) {
		return S_OK;
	}

	HRESULT __stdcall SetClipList( THIS_ LPRGNDATA x, DWORD y ) {
		return S_OK;
	}

	HRESULT __stdcall SetHWnd( THIS_ DWORD x, HWND handle ) {
		hWnd = handle;
		return S_OK;
	}

private:
	HWND hWnd;
	int refCount;
};
