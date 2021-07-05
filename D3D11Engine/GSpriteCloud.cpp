#include "pch.h"
#include "GSpriteCloud.h"


GSpriteCloud::GSpriteCloud() {}


GSpriteCloud::~GSpriteCloud() {}

struct CloudBB {
    void MakeRandom( const DirectX::XMFLOAT3& center, const DirectX::XMFLOAT3& minSize, const DirectX::XMFLOAT3& maxSize ) {
        DirectX::XMFLOAT3 d;
        XMStoreFloat3( &d, XMLoadFloat3( &maxSize ) - XMLoadFloat3( &minSize ) );

        // Random Box size
        DirectX::XMFLOAT3 sr = DirectX::XMFLOAT3( minSize.x + Toolbox::frand() * d.x,
            minSize.y + Toolbox::frand() * d.y,
            minSize.z + Toolbox::frand() * d.z );
        XMVECTOR XMV_sr = XMLoadFloat3( &sr );
        XMV_sr /= 2.0f;
        XMStoreFloat3( &Size, XMV_sr );

        Center = center;
    }

    DirectX::XMFLOAT3 GetRandomPointInBox() {
        DirectX::XMFLOAT3 r = DirectX::XMFLOAT3( (Toolbox::frand() * Size.x * 2) - Size.x,
            (Toolbox::frand() * Size.y * 2) - Size.y,
            (Toolbox::frand() * Size.z * 2) - Size.z );

        return r;
    }

    DirectX::XMFLOAT3 Center;
    DirectX::XMFLOAT3 Size;
};

/** Initializes this cloud */
void GSpriteCloud::CreateCloud( const DirectX::XMFLOAT3& size, int numSprites ) {
    CloudBB c;
    DirectX::XMFLOAT3 Size_dived;
    XMStoreFloat3( &Size_dived, XMLoadFloat3( &size ) / 2.0f );
    c.MakeRandom( DirectX::XMFLOAT3( 0, 0, 0 ), Size_dived, size );

    // Fill the bb with sprites
    for ( int i = 0; i < numSprites; i++ ) {
        DirectX::XMFLOAT3 rnd = c.GetRandomPointInBox();
        Sprites.push_back( rnd );

        DirectX::XMFLOAT4X4 m;
        XMMATRIX XMM_m = DirectX::XMMatrixTranslation( rnd.x, rnd.y, rnd.z );
        XMStoreFloat4x4( &m, XMM_m );
        SpriteWorldMatrices.push_back( m );
    }
}