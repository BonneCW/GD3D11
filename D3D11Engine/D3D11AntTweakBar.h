#pragma once
#include "baseanttweakbar.h"

class D3D11AntTweakBar :
    public BaseAntTweakBar {
public:
    D3D11AntTweakBar();
    ~D3D11AntTweakBar();

    /** Creates the resources */
    virtual XRESULT Init();
};

