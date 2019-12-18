#include "pch.h"
#include "BaseLineRenderer.h"
using namespace DirectX;
using namespace DirectX::SimpleMath;

BaseLineRenderer::BaseLineRenderer()
{
}


BaseLineRenderer::~BaseLineRenderer()
{
}

/** Plots a vector of floats */
void BaseLineRenderer::PlotNumbers(const std::vector<float> & values, const DirectX::SimpleMath::Vector3 & location, const DirectX::SimpleMath::Vector3 & direction, float distance, float heightScale, const DirectX::SimpleMath::Vector4 & color)
{
	for(unsigned int i=1;i<values.size();i++)
	{
		AddLine(LineVertex(location + (direction * (float)(i-1) * distance) + DirectX::SimpleMath::Vector3(0, 0,values[i-1]*heightScale), color),
				LineVertex(location + (direction * (float)i * distance) + DirectX::SimpleMath::Vector3(0, 0,values[i]*heightScale), color));
	}
}

/** Adds a triangle to the renderlist */
void BaseLineRenderer::AddTriangle(const DirectX::SimpleMath::Vector3 & t0, const DirectX::SimpleMath::Vector3 & t1, const DirectX::SimpleMath::Vector3 & t2, const DirectX::SimpleMath::Vector4 & color)
{
	AddLine(LineVertex(t0, color), LineVertex(t1, color));
	AddLine(LineVertex(t0, color), LineVertex(t2, color));
	AddLine(LineVertex(t1, color), LineVertex(t2, color));
}

/** Adds a point locator to the renderlist */
void BaseLineRenderer::AddPointLocator(const DirectX::SimpleMath::Vector3 & location, float size, const DirectX::SimpleMath::Vector4 & color)
{
	DirectX::SimpleMath::Vector3 u = location; u.z += size;
	DirectX::SimpleMath::Vector3 d = location; d.z -= size;

	DirectX::SimpleMath::Vector3 r = location; r.x += size;
	DirectX::SimpleMath::Vector3 l = location; l.x -= size;

	DirectX::SimpleMath::Vector3 b = location; b.y += size;
	DirectX::SimpleMath::Vector3 f = location; f.y -= size;

	AddLine(LineVertex(u, color), LineVertex(d, color));
	AddLine(LineVertex(r, color), LineVertex(l, color));
	AddLine(LineVertex(f, color), LineVertex(b, color));
}

/** Adds a plane to the renderlist */
void BaseLineRenderer::AddPlane(const DirectX::SimpleMath::Vector4 & plane, const DirectX::SimpleMath::Vector3 & origin, float size, const DirectX::SimpleMath::Vector4 & color)
{
	DirectX::SimpleMath::Vector3 pNormal = DirectX::SimpleMath::Vector3(plane);

	DirectX::SimpleMath::Vector3 DebugPlaneP1;
	DebugPlaneP1.x = 1;
	DebugPlaneP1.y = 1;
	DebugPlaneP1.z = ((-plane.x - plane.y) / plane.z);

	DebugPlaneP1.Normalize();
	
	DirectX::SimpleMath::Vector3 DebugPlaneP2 = DirectX::XMVector3Cross(pNormal, DebugPlaneP1);

	//DebugPlaneP2 += SlidingPlaneOrigin;
	DirectX::SimpleMath::Vector3 & p1 = DebugPlaneP1;
	DirectX::SimpleMath::Vector3 & p2 = DebugPlaneP2;
	DirectX::SimpleMath::Vector3 O = origin;

	DirectX::SimpleMath::Vector3 from; DirectX::SimpleMath::Vector3 to;
	from = (O - p1) - p2; 
	to = (O - p1) + p2; 
	AddLine(LineVertex(from), LineVertex(to));

	from = (O - p1) + p2; 
	to = (O + p1) + p2; 
	AddLine(LineVertex(from), LineVertex(to));

	from = (O + p1) + p2; 
	to = (O + p1) - p2; 
	AddLine(LineVertex(from), LineVertex(to));

	from = (O + p1) - p2; 
	to = (O - p1) - p2; 
	AddLine(LineVertex(from), LineVertex(to));
}

/** Adds an AABB-Box to the renderlist */
void BaseLineRenderer::AddAABB(const DirectX::SimpleMath::Vector3 & location, float halfSize, const DirectX::SimpleMath::Vector4 & color)
{
	// Bottom -x -y -z to +x -y -z
	AddLine(LineVertex(DirectX::SimpleMath::Vector3(location.x - halfSize, location.y - halfSize, location.z - halfSize), color), LineVertex(DirectX::SimpleMath::Vector3(location.x + halfSize, location.y - halfSize, location.z - halfSize), color));

	AddLine(LineVertex(DirectX::SimpleMath::Vector3(location.x + halfSize, location.y - halfSize, location.z - halfSize), color), LineVertex(DirectX::SimpleMath::Vector3(location.x + halfSize, location.y + halfSize, location.z - halfSize), color));

	AddLine(LineVertex(DirectX::SimpleMath::Vector3(location.x + halfSize, location.y + halfSize, location.z - halfSize), color), LineVertex(DirectX::SimpleMath::Vector3(location.x - halfSize, location.y + halfSize, location.z - halfSize), color));

	AddLine(LineVertex(DirectX::SimpleMath::Vector3(location.x - halfSize, location.y + halfSize, location.z - halfSize), color), LineVertex(DirectX::SimpleMath::Vector3(location.x - halfSize, location.y - halfSize, location.z - halfSize), color));

	// Top
	AddLine(LineVertex(DirectX::SimpleMath::Vector3(location.x - halfSize, location.y - halfSize, location.z + halfSize), color), LineVertex(DirectX::SimpleMath::Vector3(location.x + halfSize, location.y - halfSize, location.z + halfSize), color));

	AddLine(LineVertex(DirectX::SimpleMath::Vector3(location.x + halfSize, location.y - halfSize, location.z + halfSize), color), LineVertex(DirectX::SimpleMath::Vector3(location.x + halfSize, location.y + halfSize, location.z + halfSize), color));

	AddLine(LineVertex(DirectX::SimpleMath::Vector3(location.x + halfSize, location.y + halfSize, location.z + halfSize), color), LineVertex(DirectX::SimpleMath::Vector3(location.x - halfSize, location.y + halfSize, location.z + halfSize), color));

	AddLine(LineVertex(DirectX::SimpleMath::Vector3(location.x - halfSize, location.y + halfSize, location.z + halfSize), color), LineVertex(DirectX::SimpleMath::Vector3(location.x - halfSize, location.y - halfSize, location.z + halfSize), color));

	// Sides
	AddLine(LineVertex(DirectX::SimpleMath::Vector3(location.x - halfSize, location.y - halfSize, location.z + halfSize), color), LineVertex(DirectX::SimpleMath::Vector3(location.x - halfSize, location.y - halfSize, location.z - halfSize), color));

	AddLine(LineVertex(DirectX::SimpleMath::Vector3(location.x + halfSize, location.y - halfSize, location.z + halfSize), color), LineVertex(DirectX::SimpleMath::Vector3(location.x + halfSize, location.y - halfSize, location.z - halfSize), color));

	AddLine(LineVertex(DirectX::SimpleMath::Vector3(location.x + halfSize, location.y + halfSize, location.z + halfSize), color), LineVertex(DirectX::SimpleMath::Vector3(location.x + halfSize, location.y + halfSize, location.z - halfSize), color));

	AddLine(LineVertex(DirectX::SimpleMath::Vector3(location.x - halfSize, location.y + halfSize, location.z + halfSize), color), LineVertex(DirectX::SimpleMath::Vector3(location.x - halfSize, location.y + halfSize, location.z - halfSize), color));

}

/** Adds an AABB-Box to the renderlist */
void BaseLineRenderer::AddAABB(const DirectX::SimpleMath::Vector3 & location, const DirectX::SimpleMath::Vector3 &  halfSize, const DirectX::SimpleMath::Vector4 & color)
{
	AddAABBMinMax(DirectX::SimpleMath::Vector3(	location.x - halfSize.x,
						location.y - halfSize.y, 
						location.z - halfSize.z), DirectX::SimpleMath::Vector3(	location.x + halfSize.x,
															location.y + halfSize.y, 
															location.z + halfSize.z), color);
}



void BaseLineRenderer::AddAABBMinMax(const DirectX::SimpleMath::Vector3 & min, const DirectX::SimpleMath::Vector3 & max, const DirectX::SimpleMath::Vector4 & color)
{
	AddLine(LineVertex(DirectX::SimpleMath::Vector3(min.x, min.y, min.z), color), LineVertex(DirectX::SimpleMath::Vector3(max.x, min.y, min.z), color));
	AddLine(LineVertex(DirectX::SimpleMath::Vector3(max.x, min.y, min.z), color), LineVertex(DirectX::SimpleMath::Vector3(max.x, max.y, min.z), color));
	AddLine(LineVertex(DirectX::SimpleMath::Vector3(max.x, max.y, min.z), color), LineVertex(DirectX::SimpleMath::Vector3(min.x, max.y, min.z), color));
	AddLine(LineVertex(DirectX::SimpleMath::Vector3(min.x, max.y, min.z), color), LineVertex(DirectX::SimpleMath::Vector3(min.x, min.y, min.z), color));
													
	AddLine(LineVertex(DirectX::SimpleMath::Vector3(min.x, min.y, max.z), color), LineVertex(DirectX::SimpleMath::Vector3(max.x, min.y, max.z), color));
	AddLine(LineVertex(DirectX::SimpleMath::Vector3(max.x, min.y, max.z), color), LineVertex(DirectX::SimpleMath::Vector3(max.x, max.y, max.z), color));
	AddLine(LineVertex(DirectX::SimpleMath::Vector3(max.x, max.y, max.z), color), LineVertex(DirectX::SimpleMath::Vector3(min.x, max.y, max.z), color));
	AddLine(LineVertex(DirectX::SimpleMath::Vector3(min.x, max.y, max.z), color), LineVertex(DirectX::SimpleMath::Vector3(min.x, min.y, max.z), color));
												
	AddLine(LineVertex(DirectX::SimpleMath::Vector3(min.x, min.y, min.z), color), LineVertex(DirectX::SimpleMath::Vector3(min.x, min.y, max.z), color));
	AddLine(LineVertex(DirectX::SimpleMath::Vector3(max.x, min.y, min.z), color), LineVertex(DirectX::SimpleMath::Vector3(max.x, min.y, max.z), color));
	AddLine(LineVertex(DirectX::SimpleMath::Vector3(max.x, max.y, min.z), color), LineVertex(DirectX::SimpleMath::Vector3(max.x, max.y, max.z), color));
	AddLine(LineVertex(DirectX::SimpleMath::Vector3(min.x, max.y, min.z), color), LineVertex(DirectX::SimpleMath::Vector3(min.x, max.y, max.z), color));
}

/** Adds a ring to the renderlist */
void BaseLineRenderer::AddRingZ(const DirectX::SimpleMath::Vector3 & location, float size, const DirectX::SimpleMath::Vector4 & color, int res)
{
	std::vector<DirectX::SimpleMath::Vector3> points;
	float step = (float)(XM_PI * 2) / (float)res;

	for(int i=0;i<res;i++)
	{
		points.push_back(DirectX::SimpleMath::Vector3(size * sinf(step * i) + location.x, size * cosf(step * i) + location.y, location.z));
	}

	for(unsigned int i=0; i<points.size()-1;i++)
	{
		AddLine(LineVertex(points[i], color), LineVertex(points[i+1], color));
	}

	AddLine(LineVertex(points[points.size()-1], color), LineVertex(points[0], color));
}

/** Draws a wireframe mesh */
void BaseLineRenderer::AddWireframeMesh(const std::vector<ExVertexStruct> & vertices, const std::vector<VERTEX_INDEX> & indices, const DirectX::SimpleMath::Vector4 & color, const DirectX::SimpleMath::Matrix* world)
{
	for(size_t i=0;i<indices.size();i+=3)
	{
		DirectX::SimpleMath::Vector3 vx[3];
		for(int v=0;v<3;v++)
		{
			if (world)
				vx[v] = DirectX::XMVector3TransformCoord(*vertices[indices[i + v]].Position.toVector3(), *world);
			else
				vx[v] = *vertices[indices[i + v]].Position.toVector3();
		}

		AddTriangle(vx[0], vx[1], vx[2], color);
	}
}