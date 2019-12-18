#pragma once
#include "pch.h"


class GMeshSimple;
class D3D11Texture;
class D3D11ConstantBuffer;
class D3D11VertexBuffer;
class zCTexture;
struct MeshInfo;



class GVegetationBox
{
public:
	GVegetationBox();
	virtual ~GVegetationBox();

	enum EShape
	{
		S_None,
		S_Box,
		S_Circle
	};

	/** Initializes the vegetationbox */
	XRESULT InitVegetationBox(	const DirectX::SimpleMath::Vector3 & min, 
								const DirectX::SimpleMath::Vector3 & max, 
								const std::string & vegetationMesh, 
								float density, 
								float maxSize,
								const std::string & restrictByTexture = "",
								EShape shape = S_Box);

	XRESULT InitVegetationBox( MeshInfo * mesh, 
								const std::string & vegetationMesh,
								float density, 
								float maxSize,
								zCTexture * meshTexture = nullptr);

	/** Draws this vegetation box */
	void RenderVegetation(const DirectX::SimpleMath::Vector3 & eye);

	/** Returns true if the given position is inside the box */
	bool PositionInsideBox(const DirectX::SimpleMath::Vector3 & p);

	/** Sets bounding box rendering */
	void SetRenderBoundingBox(bool value);

	/** Returns what mesh this is placed on */
	MeshInfo * GetWorldMeshPart(){return MeshPart;}

	/** Visualizes the grass-meshes */
	void VisualizeGrass(const DirectX::SimpleMath::Vector4 & color = DirectX::SimpleMath::Vector4::One);

	/** Returns the boundingbox of this */
	void GetBoundingBox(DirectX::SimpleMath::Vector3 * bbMin, DirectX::SimpleMath::Vector3 * bbMax);
	void SetBoundingBox(const DirectX::SimpleMath::Vector3 & bbMin, const DirectX::SimpleMath::Vector3 & bbMax);

	/** Removes all vegetation in range of the given position */
	void RemoveVegetationAt(const DirectX::SimpleMath::Vector3 & position, float range);

	/** Refits the bounding-box around the grass-meshes. If there are none, the box will be set to 0. */
	void RefitBoundingBox();

	/** Re-sets the grass with the given density */
	void ResetVegetationWithDensity(float density);

	/** Applys a uniform scaling to all vegetations */
	void ApplyUniformScaling(float scale);

	/** Returns true if this is empty */
	bool IsEmpty();

	/** Saves this box to the given FILE* */
	void SaveToFILE(FILE* f, int version);

	/** Loads this box from the given FILE* */
	void LoadFromFILE(FILE* f, int version);

	/** Returns whether this has been modified or not */
	bool HasBeenModified();

	/** Returns the current density of this volume */
	float GetDensity();
private:
	/** Puts trasformation for the given spots */
	void InitSpotsRandom(const std::vector<DirectX::SimpleMath::Vector3> & trisInside, EShape shape = S_None, float density = 1.0f);

	std::vector<DirectX::SimpleMath::Vector3> TrisInside;
	std::vector<DirectX::SimpleMath::Matrix> VegetationSpots;
	GMeshSimple* VegetationMesh;
	zCTexture * MeshTexture;
	MeshInfo * MeshPart;
	EShape Shape;
	float Density;

	DirectX::SimpleMath::Vector3 BoxMin;
	DirectX::SimpleMath::Vector3 BoxMax;
	D3D11Texture * VegetationTexture;
	D3D11VertexBuffer* InstancingBuffer;
	D3D11ConstantBuffer* GrassCB;
	bool DrawBoundingBox;
	bool Modified;
};

