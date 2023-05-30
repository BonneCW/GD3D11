#pragma once
#include "pch.h"

class GSpriteCloud {
public:
    GSpriteCloud();
    ~GSpriteCloud();

    /** Initializes this cloud */
    void CreateCloud( const XMFLOAT3& size, int numSprites = 10 );

    /** Returns the sprite world matrices */
    const std::vector<XMFLOAT4X4>& GetWorldMatrices();

protected:
    /** World matrices for the sprites */
    std::vector<XMFLOAT3> Sprites;

    /** Sprite positions as world matrices */
    std::vector<XMFLOAT4X4> SpriteWorldMatrices;
};

