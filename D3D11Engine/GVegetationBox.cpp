#include <map>
#include "pch.h"
#include "GVegetationBox.h"
#include "GMeshSimple.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "zCBspTree.h"
#include "BaseGraphicsEngine.h"
#include "BaseLineRenderer.h"
#include "D3D11Texture.h"
#include "D3D11GraphicsEngine.h"
#include "zCMaterial.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

GVegetationBox::GVegetationBox()
{
	VegetationMesh = nullptr;
	VegetationTexture = nullptr;
	InstancingBuffer = nullptr;
	GrassCB = nullptr;
	MeshTexture = nullptr;
	MeshPart = nullptr;
	DrawBoundingBox = false;
	Modified = false;
	Density = 1.0f;
}


GVegetationBox::~GVegetationBox()
{
	delete VegetationMesh;
	delete InstancingBuffer;
	delete VegetationTexture;
	delete GrassCB;
}

/** Returns true if the given position is inside the box */
bool GVegetationBox::PositionInsideBox(const DirectX::SimpleMath::Vector3 & p)
{
	if (p.x > BoxMin.x &&
		p.y > BoxMin.y &&
		p.z > BoxMin.z &&
		p.x < BoxMax.x &&
		p.y < BoxMax.y &&
		p.z < BoxMax.z)
		return true;

	return false;
}

XRESULT GVegetationBox::InitVegetationBox( MeshInfo * mesh, 
								const std::string & vegetationMesh,
								float density, 
								float maxSize,
								zCTexture * meshTexture)
{
	if (VegetationMesh)
	{
		LogWarn() << "Tried to init GVegetationBox twice!";
		return XR_FAILED;
	}

	// Load vegetationmesh
	VegetationMesh = new GMeshSimple;
	if (XR_SUCCESS != VegetationMesh->LoadMesh("system\\GD3D11\\Meshes\\grass02.3ds"))
	{
		delete VegetationMesh;
		VegetationMesh = nullptr;
		return XR_FAILED;
	}

	Engine::GraphicsEngine->CreateTexture(&VegetationTexture);
	VegetationTexture->Init("system\\GD3D11\\Meshes\\grass02.png");

	MeshPart = mesh;
	MeshTexture = meshTexture;

	// Compute boundingbox and polys
	BoxMax = DirectX::SimpleMath::Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	BoxMin = DirectX::SimpleMath::Vector3(FLT_MAX, FLT_MAX, FLT_MAX);

	for(unsigned int i=0;i<mesh->Vertices.size();i++)
	{
		BoxMin.x = BoxMin.x > mesh->Vertices[i].Position.x ? mesh->Vertices[i].Position.x : BoxMin.x;
		BoxMin.y = BoxMin.y > mesh->Vertices[i].Position.y ? mesh->Vertices[i].Position.y : BoxMin.y;
		BoxMin.z = BoxMin.z > mesh->Vertices[i].Position.z ? mesh->Vertices[i].Position.z : BoxMin.z;

		BoxMax.x = BoxMax.x < mesh->Vertices[i].Position.x ? mesh->Vertices[i].Position.x : BoxMax.x;
		BoxMax.y = BoxMax.y < mesh->Vertices[i].Position.y ? mesh->Vertices[i].Position.y : BoxMax.y;
		BoxMax.z = BoxMax.z < mesh->Vertices[i].Position.z ? mesh->Vertices[i].Position.z : BoxMax.z;
	}

	std::vector<DirectX::SimpleMath::Vector3> trisInside;
	for(unsigned int i=0;i<mesh->Indices.size();i+=3)
	{
		DirectX::SimpleMath::Vector3 tri[3];

		tri[0] = *mesh->Vertices[mesh->Indices[i]].Position.toVector3();
		tri[1] = *mesh->Vertices[mesh->Indices[i+1]].Position.toVector3();
		tri[2] = *mesh->Vertices[mesh->Indices[i+2]].Position.toVector3();

		trisInside.push_back(tri[0]);
		trisInside.push_back(tri[1]);
		trisInside.push_back(tri[2]);
	}

	InitSpotsRandom(trisInside);
	TrisInside = trisInside;

	return XR_SUCCESS;
}

/** Initializes the vegetationbox */
XRESULT GVegetationBox::InitVegetationBox(const DirectX::SimpleMath::Vector3 & min, 
										  const DirectX::SimpleMath::Vector3 & max, 
										  const std::string & vegetationMesh, 
										  float density, 
										  float maxSize,
										  const std::string & restrictByTexture,
										  EShape shape)
{
	if (VegetationMesh)
	{
		LogWarn() << "Tried to init GVegetationBox twice!";
		return XR_FAILED;
	}

	// Load vegetationmesh
	VegetationMesh = new GMeshSimple;
	if (XR_SUCCESS != VegetationMesh->LoadMesh("system\\GD3D11\\Meshes\\grass02.3ds"))
	{
		delete VegetationMesh;
		return XR_FAILED;
	}

	Engine::GraphicsEngine->CreateTexture(&VegetationTexture);
	VegetationTexture->Init("system\\GD3D11\\Meshes\\grass02.png");

	if (restrictByTexture != "")
	{
		zCMaterial* m = Engine::GAPI->GetMaterialByTextureName(restrictByTexture);
		MeshTexture = m ? m->GetTexture() : nullptr;
	}

	BoxMax = max;
	BoxMin = min;
	Shape = shape;

	// Get polygons laying in this box
	zCPolygon** p = Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetPolygons();
	std::vector<DirectX::SimpleMath::Vector3> polysInside;

	// Get polys inside the box //TODO: Get crossing polys too!
	for(int i=0;i<Engine::GAPI->GetLoadedWorldInfo()->BspTree->GetNumPolys();i++)
	{
		for(int v=0;v<4;v++)
		{
			if (v == 4)
			{
				// Check center too
				DirectX::SimpleMath::Vector3 tri[] = {   *p[i]->getVertices()[0]->Position.toVector3(), 
										*p[i]->getVertices()[1]->Position.toVector3(), 
										*p[i]->getVertices()[2]->Position.toVector3()};

				// Get the center
				DirectX::SimpleMath::Vector3 center = (tri[0] + tri[1] + tri[2]) / 3.0f;

				if (PositionInsideBox(*p[i]->getVertices()[v]->Position.toVector3()))
				{
					// Restrict by texture
					if (restrictByTexture.length() && 
						p[i]->GetMaterial() &&
						p[i]->GetMaterial()->GetTexture() &&
						p[i]->GetMaterial()->GetTexture()->GetNameWithoutExt() == restrictByTexture)
					{
						polysInside.push_back(tri[0]);
						polysInside.push_back(tri[1]);
						polysInside.push_back(tri[2]);
					} else if (!restrictByTexture.length())
					{
						polysInside.push_back(tri[0]);
						polysInside.push_back(tri[1]);
						polysInside.push_back(tri[2]);
					}

					// Use the texture of the first poly we find
					if (!MeshTexture)
					{
						MeshTexture = p[i]->GetMaterial() ? p[i]->GetMaterial()->GetTexture() : nullptr;
					}
				}
				break;
			}

			if (PositionInsideBox(*p[i]->getVertices()[v]->Position.toVector3()))
			{
				DirectX::SimpleMath::Vector3 tri[] = {   *p[i]->getVertices()[0]->Position.toVector3(), 
										*p[i]->getVertices()[1]->Position.toVector3(), 
										*p[i]->getVertices()[2]->Position.toVector3()};

				// Restrict by texture
				if (restrictByTexture.length() && 
					p[i]->GetMaterial() &&
					p[i]->GetMaterial()->GetTexture() &&
					p[i]->GetMaterial()->GetTexture()->GetNameWithoutExt() == restrictByTexture)
				{
					polysInside.push_back(tri[0]);
					polysInside.push_back(tri[1]);
					polysInside.push_back(tri[2]);
				} else if (!restrictByTexture.length())
				{
					polysInside.push_back(tri[0]);
					polysInside.push_back(tri[1]);
					polysInside.push_back(tri[2]);
				}

				// Use the texture of the first poly we find
				if (!MeshTexture)
				{
					MeshTexture = p[i]->GetMaterial() ? p[i]->GetMaterial()->GetTexture() : nullptr;
				}
				break;
			}
		}
	}


	InitSpotsRandom(polysInside, shape);
	TrisInside = polysInside;

	return XR_SUCCESS;
}

/** Puts trasformation for the given spots */
void GVegetationBox::InitSpotsRandom(const std::vector<DirectX::SimpleMath::Vector3> & trisInside, EShape shape, float density)
{
	DirectX::SimpleMath::Vector3 mid = BoxMin * 0.5f + BoxMax * 0.5f;
	DirectX::SimpleMath::Vector3 bs = (BoxMax - BoxMin);
	float rad = std::min(bs.x, bs.z) / 2.0f;

	delete InstancingBuffer; InstancingBuffer = nullptr;
	delete GrassCB; GrassCB = nullptr;
	VegetationSpots.clear();

	// Find random spots on the polygons (TODO: This is still based off the size of the polygons!)
	std::vector<DirectX::SimpleMath::Vector3> spots;
	for(unsigned int i=0;i<trisInside.size();i+=3)
	{
		for(unsigned int d=0;d<std::max(1.0f, 30 * density);d++)
		{
			DirectX::SimpleMath::Vector3 tri[] = {   trisInside[i], 
									trisInside[i+1], 
									trisInside[i+2]};


			float b0 = Toolbox::frand();
			float b1 = (1.0f - b0) * Toolbox::frand();
			float b2 = 1 - b0 - b1;

			DirectX::SimpleMath::Vector3 rnd = tri[0] * b0
					 + tri[1] * b1
					 + tri[2] * b2;

			// Get 2 random points on the edges
			/*DirectX::SimpleMath::Vector3 rp[3];
			D3DXVec3Lerp(&rp[0], &tri[0], &tri[1], Toolbox::frand());
			D3DXVec3Lerp(&rp[1], &tri[0], &tri[2], Toolbox::frand());

			// Get the last point on that random made edge
			D3DXVec3Lerp(&rp[2], &rp[0], &rp[1], Toolbox::frand());*/

			if (PositionInsideBox(rnd))
			{
				if (shape == S_Circle) // Restrict to smalles circle inside our AABB
				{
					DirectX::SimpleMath::Vector2 m2 = DirectX::SimpleMath::Vector2(mid.x, mid.z);
					float dist = (DirectX::SimpleMath::Vector2(rnd.x, rnd.z) - m2).Length();

					if (dist >= rad)
						continue;
				}

				spots.push_back(rnd);
			}
		}
	}


	// Create the transformation matrices for every spot
	for(unsigned int i=0;i<spots.size();i++)
	{
		DirectX::XMMATRIX w;
		DirectX::SimpleMath::Matrix s;
		DirectX::SimpleMath::Matrix r;
		w = DirectX::XMMatrixTranslation(spots[i].x, spots[i].y, spots[i].z);
		float scale = Toolbox::lerp(20, 80, Toolbox::frand());
		s = DirectX::XMMatrixScaling(scale,scale,scale);
		r = DirectX::XMMatrixRotationY(Toolbox::frand() * (float)XM_PI * 2.0f);

		w = r * s * w;

		w = DirectX::XMMatrixTranspose(w);

		VegetationSpots.push_back(w);
	}

	if (VegetationSpots.empty())
	{
		return;
	}

	// Create instancing buffer for this box
	Engine::GraphicsEngine->CreateVertexBuffer(&InstancingBuffer);
	InstancingBuffer->Init(&VegetationSpots[0], VegetationSpots.size() * sizeof(DirectX::SimpleMath::Matrix));

	// Create constant buffer
	Engine::GraphicsEngine->CreateConstantBuffer(&GrassCB, nullptr, sizeof(GrassConstantBuffer));

	RefitBoundingBox();

	Density = density;
	return;
}

/** Draws this vegetation box */
void GVegetationBox::RenderVegetation(const DirectX::SimpleMath::Vector3 & eye)
{
	float drawRadius = Engine::GAPI->GetRendererState()->RendererSettings.OutdoorSmallVobDrawRadius;

	float dist = Toolbox::ComputePointAABBDistance(eye, BoxMin, BoxMax);
	if (dist > drawRadius)
		return;

	if (VegetationSpots.empty())
	{
		return;
	}



	if (MeshTexture)
	{	
		if (MeshTexture->CacheIn(0.6f) == zRES_CACHED_IN)
			MeshTexture->Bind(0);
		else
			return;
	}

	VegetationTexture->BindToPixelShader(1);

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_NONE;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	// Enable alpha-to-coverage
	
	if (Engine::GAPI->GetRendererState()->RendererSettings.VegetationAlphaToCoverage)
	{
		Engine::GAPI->GetRendererState()->BlendState.SetDefault();
		Engine::GAPI->GetRendererState()->BlendState.BlendEnabled = false;
		Engine::GAPI->GetRendererState()->BlendState.AlphaToCoverage = Engine::GAPI->GetRendererState()->RendererSettings.VegetationAlphaToCoverage;
	}

	Engine::GAPI->GetRendererState()->BlendState.SetDirty();

	Engine::GraphicsEngine->SetActiveVertexShader("VS_GrassInstanced");
	Engine::GraphicsEngine->SetActivePixelShader("PS_Grass");

	((D3D11GraphicsEngine *)Engine::GraphicsEngine)->SetupVS_ExMeshDrawCall();
	((D3D11GraphicsEngine *)Engine::GraphicsEngine)->SetupVS_ExConstantBuffer();
	//((D3D11GraphicsEngine *)Engine::GraphicsEngine)->SetupVS_ExPerInstanceConstantBuffer();

	// Unseed randomizer to always have the same set of scales/rotations
	//srand(0);

	DirectX::SimpleMath::Matrix view;
	Engine::GAPI->GetViewMatrix(&view);
	view = DirectX::XMMatrixTranspose(view);

	GrassConstantBuffer gcb;
	gcb.G_NormalVS = DirectX::SimpleMath::Vector3::TransformNormal(DirectX::SimpleMath::Vector3(0, 1, 0), view);
	gcb.G_Time = Engine::GAPI->GetTimeSeconds();
	gcb.G_WindStrength = Engine::GAPI->GetRendererState()->RendererSettings.GlobalWindStrength;
	GrassCB->UpdateBuffer(&gcb);
	GrassCB->BindToVertexShader(1);

	// Draw the batch
	VegetationMesh->DrawBatch(InstancingBuffer, VegetationSpots.size(), sizeof(DirectX::SimpleMath::Matrix));
	
	/*for(int i=0;i<VegetationSpots.size();i++)
	{
		//float sizeMod = 1 - powf((dist / drawRadius), 2.0f);

		//Engine::GraphicsEngine->GetLineRenderer()->AddPointLocator(VegetationSpots[i], 5.0f);
	}*/

	// Seed randomizer again
	//srand(timeGetTime());

	if (DrawBoundingBox)
		Engine::GraphicsEngine->GetLineRenderer()->AddAABBMinMax(BoxMin, BoxMax);

	Engine::GAPI->GetRendererState()->RasterizerState.CullMode = GothicRasterizerStateInfo::CM_CULL_FRONT;
	Engine::GAPI->GetRendererState()->RasterizerState.SetDirty();

	Engine::GAPI->GetRendererState()->BlendState.AlphaToCoverage = false;
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();
}

/** Sets bounding box rendering */
void GVegetationBox::SetRenderBoundingBox(bool value)
{
	DrawBoundingBox = value;
}

/** Visualizes the grass-meshes */
void GVegetationBox::VisualizeGrass(const DirectX::SimpleMath::Vector4 & color)
{
	// Draw bounding box
	Engine::GraphicsEngine->GetLineRenderer()->AddAABBMinMax(BoxMin, BoxMax, color);

	// Draw grass
	for(unsigned int i=0;i<VegetationSpots.size();i++)
	{
		if (i % 10 != 0)
			continue; // Only render every 10th grassmesh

		DirectX::SimpleMath::Vector3 spot = DirectX::SimpleMath::Vector3(VegetationSpots[i]._14, VegetationSpots[i]._24, VegetationSpots[i]._34); 
		DirectX::SimpleMath::Vector3 scale = DirectX::SimpleMath::Vector3::Zero;
		DirectX::SimpleMath::Vector4 * m = (DirectX::SimpleMath::Vector4 *)&VegetationSpots[i];

		// Compute scale
		//scale.x = D3DXVec3Length((DirectX::SimpleMath::Vector3 *)&m[0]);
		scale.y = ((DirectX::SimpleMath::Vector3 *)&m[1])->Length();
		//scale.z = D3DXVec3Length((DirectX::SimpleMath::Vector3 *)&m[2]);

		Engine::GraphicsEngine->GetLineRenderer()->AddLine(LineVertex(spot, color), LineVertex(spot + scale * 2.0f, color));
	}
}

/** Returns the boundingbox of this */
void GVegetationBox::GetBoundingBox(DirectX::SimpleMath::Vector3 * bbMin, DirectX::SimpleMath::Vector3 * bbMax)
{
	*bbMin = BoxMin;
	*bbMax = BoxMax;
}

void GVegetationBox::SetBoundingBox(const DirectX::SimpleMath::Vector3 & bbMin, const DirectX::SimpleMath::Vector3 & bbMax)
{
	BoxMin = bbMin;
	BoxMax = bbMax;
}

/** Removes all vegetation in range of the given position */
void GVegetationBox::RemoveVegetationAt(const DirectX::SimpleMath::Vector3 & position, float range) {
	// Make a list of the vector
	std::list<DirectX::SimpleMath::Matrix> s(VegetationSpots.begin(), VegetationSpots.end());

	// Remove everything in range
	for (std::list<DirectX::SimpleMath::Matrix>::iterator it = s.begin(); it != s.end();) {
		DirectX::SimpleMath::Vector3 spot = DirectX::SimpleMath::Vector3(it->_14, it->_24, it->_34); 

		
		const float d = (spot - position).Length();

		if (d < range) {
			it = s.erase(it);
		} else {
			++it;
		}
	}

	// Reassign
	VegetationSpots.clear();
	VegetationSpots.assign(s.begin(), s.end());

	// Recreate instancing buffer
	delete InstancingBuffer;
	InstancingBuffer = nullptr;

	if (!IsEmpty()) {
		Engine::GraphicsEngine->CreateVertexBuffer(&InstancingBuffer);
		InstancingBuffer->Init(&VegetationSpots[0], VegetationSpots.size() * sizeof(DirectX::SimpleMath::Matrix));
	}

	// Refit
	RefitBoundingBox();

	Modified = true;
}

/** Refits the bounding-box around the grass-meshes. If there are none, the box will be set to 0. */
void GVegetationBox::RefitBoundingBox()
{
	if (VegetationSpots.empty())
	{
		BoxMax = DirectX::SimpleMath::Vector3::Zero;
		BoxMin = DirectX::SimpleMath::Vector3::Zero;

		return;
	}

	// Compute boundingbox
	BoxMax = DirectX::SimpleMath::Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	BoxMin = DirectX::SimpleMath::Vector3(FLT_MAX, FLT_MAX, FLT_MAX);

	for(unsigned int i=0;i<VegetationSpots.size();i++)
	{
		DirectX::SimpleMath::Vector3 spot = DirectX::SimpleMath::Vector3(VegetationSpots[i]._14, VegetationSpots[i]._24, VegetationSpots[i]._34); 

		BoxMin.x = BoxMin.x > spot.x ? spot.x : BoxMin.x;
		BoxMin.y = BoxMin.y > spot.y ? spot.y : BoxMin.y;
		BoxMin.z = BoxMin.z > spot.z ? spot.z : BoxMin.z;

		BoxMax.x = BoxMax.x < spot.x ? spot.x : BoxMax.x;
		BoxMax.y = BoxMax.y < spot.y ? spot.y : BoxMax.y;
		BoxMax.z = BoxMax.z < spot.z ? spot.z : BoxMax.z;
	}
}

/** Applys a uniform scaling to all vegetations */
void GVegetationBox::ApplyUniformScaling(float scale)
{
	DirectX::XMMATRIX s = DirectX::XMMatrixScaling(scale, scale, scale);

	for(unsigned int i=0;i<VegetationSpots.size();i++)
	{
		DirectX::XMMATRIX w = VegetationSpots[i];
		w = DirectX::XMMatrixTranspose(w);
		w = s * w;
		VegetationSpots[i] = DirectX::XMMatrixTranspose(w);
	}
	
	delete InstancingBuffer;
	Engine::GraphicsEngine->CreateVertexBuffer(&InstancingBuffer);
	InstancingBuffer->Init(&VegetationSpots[0], VegetationSpots.size() * sizeof(DirectX::SimpleMath::Matrix));
}

/** Returns true if this is empty */
bool GVegetationBox::IsEmpty()
{
	return VegetationSpots.empty();
}

/** Saves this box to the given FILE* */
void GVegetationBox::SaveToFILE(FILE* f, int version)
{
	// Save size of vegetation array
	int vsize = VegetationSpots.size();
	fwrite(&vsize, sizeof(vsize), 1, f);

	// Save vegetation array itself
	std::vector<DirectX::SimpleMath::Vector4> spots;
	for(unsigned int i=0;i<VegetationSpots.size();i++)
	{
		DirectX::SimpleMath::Vector4 * m = (DirectX::SimpleMath::Vector4 *)&VegetationSpots[i];
		DirectX::SimpleMath::Vector4 spot = DirectX::SimpleMath::Vector4(VegetationSpots[i]._14, VegetationSpots[i]._24, VegetationSpots[i]._34, ((DirectX::SimpleMath::Vector3 *)&m[1])->Length());

		spots.push_back(spot);
	}

	fwrite(&spots[0], sizeof(DirectX::SimpleMath::Vector4) * vsize, 1, f);

	// Save trisInside
	int tsize = TrisInside.size();
	fwrite(&tsize, sizeof(tsize), 1, f);
	fwrite(&TrisInside[0], sizeof(DirectX::SimpleMath::Vector3) * tsize, 1, f);

	// Save wether this was using a mesh info or not
	bool hasMeshInfo = MeshPart != nullptr;
	fwrite(&hasMeshInfo, sizeof(hasMeshInfo), 1, f);
}

/** Loads this box from the given FILE* */
void GVegetationBox::LoadFromFILE(FILE* f, int version)
{
	// Save size of vegetation array
	int vsize;
	fread(&vsize, sizeof(vsize), 1, f);

	std::vector<DirectX::SimpleMath::Vector4> spots;
	spots.resize(vsize);
	fread(&spots[0], sizeof(DirectX::SimpleMath::Vector4) * vsize, 1, f);

	// Reconstruct spots
	for(unsigned int i=0;i<spots.size();i++)
	{
		DirectX::XMMATRIX w;
		DirectX::SimpleMath::Matrix s;
		DirectX::SimpleMath::Matrix r;
		w = DirectX::XMMatrixTranslation(spots[i].x, spots[i].y, spots[i].z);
		float scale = spots[i].w;
		s = DirectX::XMMatrixScaling(scale,scale,scale);
		r = DirectX::XMMatrixRotationY(Toolbox::frand() * (float)XM_PI * 2.0f);

		w = r * s * w;

		w = DirectX::XMMatrixTranspose(w);

		VegetationSpots.push_back(w);
	}

	// Load tris inside
	int tsize;
	fread(&tsize, sizeof(tsize), 1, f);
	TrisInside.resize(tsize);
	fread(&TrisInside[0], sizeof(Vector3) * tsize, 1, f);

	// Save wether this was using a mesh info or not
	bool hasMeshInfo = MeshPart != nullptr;
	fread(&hasMeshInfo, sizeof(hasMeshInfo), 1, f);

	MeshInfo * hitMesh = nullptr;
	zCMaterial* hitMaterial = nullptr;

	std::unordered_map<MeshInfo *, int> hitMeshMap;
	std::unordered_map<zCMaterial*, int> hitMaterialMap;

	// 90% confidence level, 10% error margin, assuming sample distribution is very close to population distribution
	// n without population size (see law of large numbers):
	// float n = pow(1.6448f, 2) * 95 * (100 - 95) / pow(10, 2);
	float n = 12.85f;
	int j = floor(spots.size() / n);

	for (unsigned int i = 0; i < spots.size(); i += j)
	{
		// Use grass-piece and trace straight down
		Vector3 spot = Vector3(spots[i].x, spots[i].y, spots[i].z);

		// Little offset
		spot.y += 1.0f;

		// Try to find meshpart and texture
		Vector3 hit;
		MeshInfo * hitMeshTrace = nullptr;
		zCMaterial* hitMaterialTrace = nullptr;
		Engine::GAPI->TraceWorldMesh(spot, Vector3(0, -1, 0), hit, nullptr, nullptr, &hitMeshTrace, &hitMaterialTrace);

		// Save results
		if (hitMeshTrace != nullptr)
			hitMeshMap[hitMeshTrace]++;
		if (hitMaterialTrace != nullptr)
			hitMaterialMap[hitMaterialTrace]++;
	}

	// Set mesh and texture
	if (hasMeshInfo)
	{
		for (auto it = hitMeshMap.begin(); it != hitMeshMap.end(); ++it)
		{
			if ((hitMesh == nullptr) || (hitMeshMap[hitMesh] < it->second))
				hitMesh = it->first;
		}

		MeshPart = hitMesh;
	}

	for (auto it = hitMaterialMap.begin(); it != hitMaterialMap.end(); ++it)
	{
		if ((hitMaterial == nullptr) || (hitMaterialMap[hitMaterial] < it->second))
			hitMaterial = it->first;
	}

	MeshTexture = hitMaterial != nullptr ? hitMaterial->GetTexture() : nullptr;

	RefitBoundingBox();

	// Create instancing buffer for this box
	Engine::GraphicsEngine->CreateVertexBuffer(&InstancingBuffer);
	InstancingBuffer->Init(&VegetationSpots[0], VegetationSpots.size() * sizeof(DirectX::SimpleMath::Matrix));

	// Create constant buffer
	Engine::GraphicsEngine->CreateConstantBuffer(&GrassCB, nullptr, sizeof(GrassConstantBuffer));

	// TODO: Make resource-load method!
	if (VegetationMesh)
	{
		LogWarn() << "Tried to init GVegetationBox twice!";
	}

	// Load vegetationmesh
	VegetationMesh = new GMeshSimple;
	if (XR_SUCCESS != VegetationMesh->LoadMesh("system\\GD3D11\\Meshes\\grass02.3ds"))
	{
		delete VegetationMesh;
	}

	Engine::GraphicsEngine->CreateTexture(&VegetationTexture);
	VegetationTexture->Init("system\\GD3D11\\Meshes\\grass02.png");

	Modified = true;
}

/** Re-sets the grass with the given density */
void GVegetationBox::ResetVegetationWithDensity(float density)
{
	srand(0);

	InitSpotsRandom(TrisInside, Shape, density);
	Modified = false;

	srand(timeGetTime());
}

/** Returns whether this has been modified or not */
bool GVegetationBox::HasBeenModified()
{
	return Modified;
}

/** Returns the current density of this volume */
float GVegetationBox::GetDensity()
{
	return Density;
}