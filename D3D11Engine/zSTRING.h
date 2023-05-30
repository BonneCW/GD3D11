#pragma once
#include "pch.h"
#include "GothicMemoryLocations.h"

class zSTRING {
public:
    zSTRING() {
        reinterpret_cast<void( __fastcall* )( zSTRING*, int, const char* )>( GothicMemoryLocations::zSTRING::ConstructorCharPtr )( this, 0, "" );
    }
    zSTRING( const char* str ) {
        reinterpret_cast<void( __fastcall* )( zSTRING*, int, const char* )>( GothicMemoryLocations::zSTRING::ConstructorCharPtr )( this, 0, str );
    }

    void Delete() {
        reinterpret_cast<void( __fastcall* )( zSTRING* )>( GothicMemoryLocations::zSTRING::DestructorCharPtr )( this );
    }

    const char* ToChar() const {
        const char* str = *reinterpret_cast<const char**>(reinterpret_cast<DWORD>(this) + GothicMemoryLocations::zSTRING::ToChar);
        return (str ? str : "");
    }

    int Length() const
    {
        return *reinterpret_cast<int*>(reinterpret_cast<DWORD>(this) + 0x0C);
    }

    char data[20];
};
