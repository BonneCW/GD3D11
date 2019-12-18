#pragma once
#include "pch.h"

#pragma pack (push, 1)	
struct LineVertex
{
	LineVertex(){}
	LineVertex(const DirectX::SimpleMath::Vector3 & position, DWORD color = 0xFFFFFFFF)
	{
		Position = position;
		Color = color;
	}

	LineVertex(const DirectX::SimpleMath::Vector3 & position, const DirectX::SimpleMath::Vector4 & color, float zScale = 1.0f)
	{
		Position = position;
		Position.w = zScale;
		Color = color;
	}

	LineVertex(const DirectX::SimpleMath::Vector3 & position, const float4 & color, float zScale = 1.0f)
	{
		Position = position;
		Position.w = zScale;
		Color = color;
	}

	float4 Position;
	float4 Color;
};
#pragma pack (pop)	

class BaseLineRenderer
{
public:
	BaseLineRenderer();
	virtual ~BaseLineRenderer();

	/** Adds a line to the list */
	virtual XRESULT AddLine(const LineVertex& v1, const LineVertex& v2) = 0;

	/** Flushes the cached lines */
	virtual XRESULT Flush() = 0;

	/** Clears the line cache */
	virtual XRESULT ClearCache() = 0;

	/** Adds a point locator to the renderlist */
	void AddPointLocator(const DirectX::SimpleMath::Vector3 & location, float size=1, const DirectX::SimpleMath::Vector4 & color = DirectX::SimpleMath::Vector4::One);

	/** Adds a plane to the renderlist */
	void AddPlane(const DirectX::SimpleMath::Vector4 & plane, const DirectX::SimpleMath::Vector3 & origin, float size=1, const DirectX::SimpleMath::Vector4 & color = DirectX::SimpleMath::Vector4::One);

	/** Adds a ring to the renderlist */
	void AddRingZ(const DirectX::SimpleMath::Vector3 & location, float size=1.0f, const DirectX::SimpleMath::Vector4 & color = DirectX::SimpleMath::Vector4::One, int res=32);

	/** Adds an AABB-Box to the renderlist */
	void AddAABB(const DirectX::SimpleMath::Vector3 & location, float halfSize, const DirectX::SimpleMath::Vector4 & color = DirectX::SimpleMath::Vector4::One);
	void AddAABB(const DirectX::SimpleMath::Vector3 & location, const DirectX::SimpleMath::Vector3 & halfSize, const DirectX::SimpleMath::Vector4 & color = DirectX::SimpleMath::Vector4::One);
	void AddAABBMinMax(const DirectX::SimpleMath::Vector3 & min, const DirectX::SimpleMath::Vector3 & max, const DirectX::SimpleMath::Vector4 & color = DirectX::SimpleMath::Vector4::One);

	/** Adds a triangle to the renderlist */
	void AddTriangle(const DirectX::SimpleMath::Vector3 & t0, const DirectX::SimpleMath::Vector3 & t1, const DirectX::SimpleMath::Vector3 & t2, const DirectX::SimpleMath::Vector4 & color = DirectX::SimpleMath::Vector4::One);

	/** Plots a vector of floats */
	void PlotNumbers(const std::vector<float> & values, const DirectX::SimpleMath::Vector3 & location, const DirectX::SimpleMath::Vector3 & direction, float distance, float heightScale, const DirectX::SimpleMath::Vector4 & color = DirectX::SimpleMath::Vector4::One);

	/** Draws a wireframe mesh */
	void AddWireframeMesh(const std::vector<ExVertexStruct> & vertices, const std::vector<VERTEX_INDEX> & indices, const DirectX::SimpleMath::Vector4 & color = DirectX::SimpleMath::Vector4::One, const DirectX::SimpleMath::Matrix* world = nullptr);
};

