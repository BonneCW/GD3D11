#include "pch.h"
#include "GSpriteCloud.h"

GSpriteCloud::GSpriteCloud() {}

GSpriteCloud::~GSpriteCloud() {}

struct CloudBB {
    void MakeRandom( const XMFLOAT3& center, const XMFLOAT3& minSize, const XMFLOAT3& maxSize ) {
        XMFLOAT3 d;
        XMStoreFloat3( &d, XMLoadFloat3( &maxSize ) - XMLoadFloat3( &minSize ) );

        // Random Box size
        XMFLOAT3 sr = XMFLOAT3( minSize.x + Toolbox::frand() * d.x,
            minSize.y + Toolbox::frand() * d.y,
            minSize.z + Toolbox::frand() * d.z );
        XMVECTOR XMV_sr = XMLoadFloat3( &sr );
        XMV_sr /= 2.0f;
        XMStoreFloat3( &Size, XMV_sr );

        Center = center;
    }

    XMFLOAT3 GetRandomPointInBox() {
        XMFLOAT3 r = XMFLOAT3( (Toolbox::frand() * Size.x * 2) - Size.x,
            (Toolbox::frand() * Size.y * 2) - Size.y,
            (Toolbox::frand() * Size.z * 2) - Size.z );

        return r;
    }

    XMFLOAT3 Center;
    XMFLOAT3 Size;
};

/** Initializes this cloud */
void GSpriteCloud::CreateCloud( const XMFLOAT3& size, int numSprites ) {
    CloudBB c;
    XMFLOAT3 Size_dived;
    XMStoreFloat3( &Size_dived, XMLoadFloat3( &size ) / 2.0f );
    c.MakeRandom( XMFLOAT3( 0, 0, 0 ), Size_dived, size );

    // Fill the bb with sprites
    for ( int i = 0; i < numSprites; i++ ) {
        XMFLOAT3 rnd = c.GetRandomPointInBox();
        Sprites.push_back( rnd );

        XMFLOAT4X4 m;
        XMMATRIX XMM_m = XMMatrixTranslation( rnd.x, rnd.y, rnd.z );
        XMStoreFloat4x4( &m, XMM_m );
        SpriteWorldMatrices.push_back( m );
    }
}
