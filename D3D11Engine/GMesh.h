#pragma once

#include <string>
#include <vector>
#include <d3d11.h>
#include <SimpleMath.h>

enum XRESULT;
struct MeshInfo;

class GMesh {
public:
	GMesh();
	virtual ~GMesh();

	enum ELoadType {
		LT_DEFAULT,
		LT_SIMPLEOBJ
	};

	/** Load a mesh from file  */
	XRESULT LoadMesh(const std::string & file, float scale = 1.0f);

	/** Fills this mesh with a grid */
	XRESULT CreateGrid(int tesselation, const DirectX::SimpleMath::Vector2 & min, const DirectX::SimpleMath::Vector2 & max, float height);

	/** Draws all buffers this holds */
	void DrawMesh();

	/** Returns the meshes */
	std::vector<MeshInfo *> & GetMeshes() { return Meshes; }
	std::vector<std::string> & GetTextures() { return Textures; }

private:
	/** Loads the cache-file-format */
	XRESULT LoadCached(const std::string & file);

	std::vector<MeshInfo *> Meshes;
	std::vector<std::string> Textures;
};
