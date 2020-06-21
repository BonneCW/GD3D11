#include "pch.h"
#include "BaseLineRenderer.h"


BaseLineRenderer::BaseLineRenderer() {}


BaseLineRenderer::~BaseLineRenderer() {}

/** Plots a vector of floats */
void BaseLineRenderer::PlotNumbers( const std::vector<float>& values, const DirectX::XMFLOAT3& location, const DirectX::XMFLOAT3& direction, float distance, float heightScale, const DirectX::XMFLOAT4& color ) {
	for ( unsigned int i = 1; i < values.size(); i++ ) {
		XMFLOAT3 v1;
		XMFLOAT3 v2;
		XMStoreFloat3( &v1, XMLoadFloat3( &location ) + (XMLoadFloat3( &direction ) * (float)(i - 1) * distance) + XMVectorSet( 0, 0, values[i - 1] * heightScale, 0 ) );
		XMStoreFloat3( &v2, XMLoadFloat3( &location ) + (XMLoadFloat3( &direction ) * (float)i * distance) + XMVectorSet( 0, 0, values[i] * heightScale, 0 ) );
		AddLine( LineVertex( v1, color ), LineVertex( v2, color ) );
	}
}

/** Adds a triangle to the renderlist */
void BaseLineRenderer::AddTriangle( const DirectX::XMFLOAT3& t0, const DirectX::XMFLOAT3& t1, const DirectX::XMFLOAT3& t2, const DirectX::XMFLOAT4& color ) {
	AddLine( LineVertex( t0, color ), LineVertex( t1, color ) );
	AddLine( LineVertex( t0, color ), LineVertex( t2, color ) );
	AddLine( LineVertex( t1, color ), LineVertex( t2, color ) );
}

/** Adds a point locator to the renderlist */
void BaseLineRenderer::AddPointLocator( const DirectX::XMFLOAT3& location, float size, const DirectX::XMFLOAT4& color ) {
	DirectX::XMFLOAT3 u = location; u.z += size;
	DirectX::XMFLOAT3 d = location; d.z -= size;

	DirectX::XMFLOAT3 r = location; r.x += size;
	DirectX::XMFLOAT3 l = location; l.x -= size;

	DirectX::XMFLOAT3 b = location; b.y += size;
	DirectX::XMFLOAT3 f = location; f.y -= size;

	AddLine( LineVertex( u, color ), LineVertex( d, color ) );
	AddLine( LineVertex( r, color ), LineVertex( l, color ) );
	AddLine( LineVertex( f, color ), LineVertex( b, color ) );
}

/** Adds a plane to the renderlist */
void BaseLineRenderer::AddPlane( const DirectX::XMFLOAT4& plane, const DirectX::XMFLOAT3& origin, float size, const DirectX::XMFLOAT4& color ) {
	XMVECTOR DebugPlaneP1 = DirectX::XMVectorSet( 1, 1, ((-plane.x - plane.y) / plane.z), 0.0f );

	DebugPlaneP1 = DirectX::XMVector3NormalizeEst( DebugPlaneP1 );

	XMVECTOR pNormal = XMLoadFloat4( &plane );
	XMVECTOR DebugPlaneP2 = DirectX::XMVector3Cross( pNormal, DebugPlaneP1 );

	//DebugPlaneP2 += SlidingPlaneOrigin;
	XMVECTOR O = XMLoadFloat3( &origin );

	DirectX::XMFLOAT3 from;  DirectX::XMFLOAT3 to;
	XMStoreFloat3( &from, (O - DebugPlaneP1) - DebugPlaneP2 );
	XMStoreFloat3( &to, (O - DebugPlaneP1) + DebugPlaneP2 );
	AddLine( LineVertex( from ), LineVertex( to ) );

	XMStoreFloat3( &from, (O - DebugPlaneP1) + DebugPlaneP2 );
	XMStoreFloat3( &to, (O + DebugPlaneP1) + DebugPlaneP2 );
	AddLine( LineVertex( from ), LineVertex( to ) );

	XMStoreFloat3( &from, (O + DebugPlaneP1) + DebugPlaneP2 );
	XMStoreFloat3( &to, (O + DebugPlaneP1) - DebugPlaneP2 );
	AddLine( LineVertex( from ), LineVertex( to ) );

	XMStoreFloat3( &from, (O + DebugPlaneP1) - DebugPlaneP2 );
	XMStoreFloat3( &to, (O - DebugPlaneP1) - DebugPlaneP2 );
	AddLine( LineVertex( from ), LineVertex( to ) );
}

/** Adds an AABB-Box to the renderlist */
void BaseLineRenderer::AddAABB( const DirectX::XMFLOAT3& location, float halfSize, const DirectX::XMFLOAT4& color ) {
	// Bottom -x -y -z to +x -y -z
	AddLine( LineVertex( DirectX::XMFLOAT3( location.x - halfSize, location.y - halfSize, location.z - halfSize ), color ), LineVertex( DirectX::XMFLOAT3( location.x + halfSize, location.y - halfSize, location.z - halfSize ), color ) );

	AddLine( LineVertex( DirectX::XMFLOAT3( location.x + halfSize, location.y - halfSize, location.z - halfSize ), color ), LineVertex( DirectX::XMFLOAT3( location.x + halfSize, location.y + halfSize, location.z - halfSize ), color ) );

	AddLine( LineVertex( DirectX::XMFLOAT3( location.x + halfSize, location.y + halfSize, location.z - halfSize ), color ), LineVertex( DirectX::XMFLOAT3( location.x - halfSize, location.y + halfSize, location.z - halfSize ), color ) );

	AddLine( LineVertex( DirectX::XMFLOAT3( location.x - halfSize, location.y + halfSize, location.z - halfSize ), color ), LineVertex( DirectX::XMFLOAT3( location.x - halfSize, location.y - halfSize, location.z - halfSize ), color ) );

	// Top
	AddLine( LineVertex( DirectX::XMFLOAT3( location.x - halfSize, location.y - halfSize, location.z + halfSize ), color ), LineVertex( DirectX::XMFLOAT3( location.x + halfSize, location.y - halfSize, location.z + halfSize ), color ) );

	AddLine( LineVertex( DirectX::XMFLOAT3( location.x + halfSize, location.y - halfSize, location.z + halfSize ), color ), LineVertex( DirectX::XMFLOAT3( location.x + halfSize, location.y + halfSize, location.z + halfSize ), color ) );

	AddLine( LineVertex( DirectX::XMFLOAT3( location.x + halfSize, location.y + halfSize, location.z + halfSize ), color ), LineVertex( DirectX::XMFLOAT3( location.x - halfSize, location.y + halfSize, location.z + halfSize ), color ) );

	AddLine( LineVertex( DirectX::XMFLOAT3( location.x - halfSize, location.y + halfSize, location.z + halfSize ), color ), LineVertex( DirectX::XMFLOAT3( location.x - halfSize, location.y - halfSize, location.z + halfSize ), color ) );

	// Sides
	AddLine( LineVertex( DirectX::XMFLOAT3( location.x - halfSize, location.y - halfSize, location.z + halfSize ), color ), LineVertex( DirectX::XMFLOAT3( location.x - halfSize, location.y - halfSize, location.z - halfSize ), color ) );

	AddLine( LineVertex( DirectX::XMFLOAT3( location.x + halfSize, location.y - halfSize, location.z + halfSize ), color ), LineVertex( DirectX::XMFLOAT3( location.x + halfSize, location.y - halfSize, location.z - halfSize ), color ) );

	AddLine( LineVertex( DirectX::XMFLOAT3( location.x + halfSize, location.y + halfSize, location.z + halfSize ), color ), LineVertex( DirectX::XMFLOAT3( location.x + halfSize, location.y + halfSize, location.z - halfSize ), color ) );

	AddLine( LineVertex( DirectX::XMFLOAT3( location.x - halfSize, location.y + halfSize, location.z + halfSize ), color ), LineVertex( DirectX::XMFLOAT3( location.x - halfSize, location.y + halfSize, location.z - halfSize ), color ) );

}

/** Adds an AABB-Box to the renderlist */
void BaseLineRenderer::AddAABB( const DirectX::XMFLOAT3& location, const DirectX::XMFLOAT3& halfSize, const DirectX::XMFLOAT4& color ) {
	AddAABBMinMax( DirectX::XMFLOAT3( location.x - halfSize.x,
		location.y - halfSize.y,
		location.z - halfSize.z ), DirectX::XMFLOAT3( location.x + halfSize.x,
			location.y + halfSize.y,
			location.z + halfSize.z ), color );
}



void BaseLineRenderer::AddAABBMinMax( const DirectX::XMFLOAT3& min, const DirectX::XMFLOAT3& max, const DirectX::XMFLOAT4& color ) {
	AddLine( LineVertex( DirectX::XMFLOAT3( min.x, min.y, min.z ), color ), LineVertex( DirectX::XMFLOAT3( max.x, min.y, min.z ), color ) );
	AddLine( LineVertex( DirectX::XMFLOAT3( max.x, min.y, min.z ), color ), LineVertex( DirectX::XMFLOAT3( max.x, max.y, min.z ), color ) );
	AddLine( LineVertex( DirectX::XMFLOAT3( max.x, max.y, min.z ), color ), LineVertex( DirectX::XMFLOAT3( min.x, max.y, min.z ), color ) );
	AddLine( LineVertex( DirectX::XMFLOAT3( min.x, max.y, min.z ), color ), LineVertex( DirectX::XMFLOAT3( min.x, min.y, min.z ), color ) );

	AddLine( LineVertex( DirectX::XMFLOAT3( min.x, min.y, max.z ), color ), LineVertex( DirectX::XMFLOAT3( max.x, min.y, max.z ), color ) );
	AddLine( LineVertex( DirectX::XMFLOAT3( max.x, min.y, max.z ), color ), LineVertex( DirectX::XMFLOAT3( max.x, max.y, max.z ), color ) );
	AddLine( LineVertex( DirectX::XMFLOAT3( max.x, max.y, max.z ), color ), LineVertex( DirectX::XMFLOAT3( min.x, max.y, max.z ), color ) );
	AddLine( LineVertex( DirectX::XMFLOAT3( min.x, max.y, max.z ), color ), LineVertex( DirectX::XMFLOAT3( min.x, min.y, max.z ), color ) );

	AddLine( LineVertex( DirectX::XMFLOAT3( min.x, min.y, min.z ), color ), LineVertex( DirectX::XMFLOAT3( min.x, min.y, max.z ), color ) );
	AddLine( LineVertex( DirectX::XMFLOAT3( max.x, min.y, min.z ), color ), LineVertex( DirectX::XMFLOAT3( max.x, min.y, max.z ), color ) );
	AddLine( LineVertex( DirectX::XMFLOAT3( max.x, max.y, min.z ), color ), LineVertex( DirectX::XMFLOAT3( max.x, max.y, max.z ), color ) );
	AddLine( LineVertex( DirectX::XMFLOAT3( min.x, max.y, min.z ), color ), LineVertex( DirectX::XMFLOAT3( min.x, max.y, max.z ), color ) );
}

/** Adds a ring to the renderlist */
void BaseLineRenderer::AddRingZ( const DirectX::XMFLOAT3& location, float size, const DirectX::XMFLOAT4& color, int res ) {
	std::vector<DirectX::XMFLOAT3> points;
	float step = DirectX::XM_2PI / (float)res;

	for ( int i = 0; i < res; i++ ) {
		points.push_back( DirectX::XMFLOAT3( size * sinf( step * i ) + location.x, size * cosf( step * i ) + location.y, location.z ) );
	}

	for ( unsigned int i = 0; i < points.size() - 1; i++ ) {
		AddLine( LineVertex( points[i], color ), LineVertex( points[i + 1], color ) );
	}

	AddLine( LineVertex( points[points.size() - 1], color ), LineVertex( points[0], color ) );
}

/** Draws a wireframe mesh */
void BaseLineRenderer::AddWireframeMesh( const std::vector<ExVertexStruct>& vertices, const std::vector<VERTEX_INDEX>& indices, const DirectX::XMFLOAT4& color, const DirectX::XMFLOAT4X4* world ) {
	for ( size_t i = 0; i < indices.size(); i += 3 ) {
		DirectX::XMFLOAT3 vx[3];
		for ( int v = 0; v < 3; v++ ) {
			if ( world ) {
				XMStoreFloat3( &vx[v], DirectX::XMVector3TransformCoord( XMVectorSet( vertices[indices[i + v]].Position.x, vertices[indices[i + v]].Position.y, vertices[indices[i + v]].Position.z, 0.0f ), XMLoadFloat4x4( world ) ) );
			} else
				vx[v] = *vertices[indices[i + v]].Position.toXMFLOAT3();
		}

		AddTriangle( vx[0], vx[1], vx[2], color );
	}
}