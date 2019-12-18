#include "pch.h"
#include "MeshModifier.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;


MeshModifier::MeshModifier()
{
}


MeshModifier::~MeshModifier()
{
}

/** Performs catmul-clark smoothing on the mesh */
void MeshModifier::DoCatmulClark(const std::vector<ExVertexStruct> & inVertices, const std::vector<unsigned short> & inIndices, std::vector<ExVertexStruct> & outVertices, std::vector<unsigned short> & outIndices, int iterations)
{

}

/** Detects borders on the mesh */
void MeshModifier::DetectBorders(const std::vector<ExVertexStruct> & inVertices, const std::vector<unsigned short> & inIndices, std::vector<ExVertexStruct> & outVertices, std::vector<unsigned short> & outIndices)
{

}

/** Drops texcoords on the given mesh, making it appear crackless */
void MeshModifier::DropTexcoords(const std::vector<ExVertexStruct> & inVertices, const std::vector<unsigned short> & inIndices, std::vector<ExVertexStruct> & outVertices, std::vector<VERTEX_INDEX> & outIndices)
{

}

/** Decimates the mesh, reducing its complexity */
void MeshModifier::Decimate(const std::vector<ExVertexStruct> & inVertices, const std::vector<unsigned short> & inIndices, std::vector<ExVertexStruct> & outVertices, std::vector<VERTEX_INDEX> & outIndices)
{
	
}

struct PNAENEdge
{
	// "An Edge should consist of the origin index, the destination index, the origin position and the destination position"
	unsigned int iO;
	unsigned int iD;

	float3 pO;
	float3 pD;

	// "Reverse simply flips the sense of the edge"
	void ReverseEdge()
	{
		std::swap(iO, iD);
		std::swap(pO, pD);
	}

	bool operator == (const PNAENEdge& o) const
	{
		if (iO == o.iO && iD == o.iD)
			return true;

		if (pO == o.pO && pD == o.pD)
			return true;
		
		return false;
	}

};

bool operator< (const PNAENEdge& lhs, const PNAENEdge& rhs)
{
	return (lhs.iO < rhs.iO) || (lhs.iO == rhs.iO && lhs.iD < rhs.iD);
}

struct PNAENKeyHasher
{
	static const size_t bucket_size = 10; // mean bucket size that the container should try not to exceed
	static const size_t min_buckets = (1 << 10); // minimum number of buckets, power of 2, >0

	static std::size_t hash_value(float value)
	{
		stdext::hash<float> hasher;
		return hasher(value);
	}

	static void hash_combine(std::size_t& seed, float value)
	{	
		seed ^= hash_value(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	std::size_t operator()(const PNAENEdge& k) const
	{
		// Start with a hash value of 0    .
		std::size_t seed = 0;

		// Modify 'seed' by XORing and bit-shifting in
		// one member of 'Key' after the other:

		hash_combine(seed, k.pO.x);
		hash_combine(seed, k.pO.y);
		hash_combine(seed, k.pO.z);
		//hash_combine(seed, hash_value(k.iO));

		hash_combine(seed, k.pD.x);
		hash_combine(seed, k.pD.y);
		hash_combine(seed, k.pD.z);
		//hash_combine(seed, hash_value(k.iD));

		// Return the result.
		return seed;
	}

	/*bool operator()(const PNAENEdge &left, const PNAENEdge &right) 
	{
		if (left.iD < right.iD || (left.iD == right.iD && left.iO < right.iO))
			return true;

		return left.pD < right.pD || (left.pD == right.pD && left.pO < right.pO);
    }*/
};

void MeshModifier::ComputePNAENIndices(const std::vector<ExVertexStruct> & inVertices, const std::vector<unsigned short> & inIndices, std::vector<VERTEX_INDEX> & outIndices)
{
	std::vector<unsigned int> ix;
	std::vector<unsigned int> out;
	ix.assign(inIndices.begin(), inIndices.end());

	// Run the computation
	ComputePNAENIndices(inVertices, ix, out);

	outIndices.assign(out.begin(), out.end());
}

/** Computes PNAEN-Indices for the given mesh */
void MeshModifier::ComputePNAENIndices(const std::vector<ExVertexStruct> & inVertices, const std::vector<unsigned int> & inIndices, std::vector<unsigned int> & outIndices)
{
	// PNAEN algorithm from NVIDIA (http://developer.download.nvidia.com/whitepapers/2010/PN-AEN-Triangles-Whitepaper.pdf)
	
	// "For each edge, store the reverse of that edge in an easily searchable data structure for the next step. 
	//  The reference implementation uses an stdext::unordered_map<Edge,Edge> for this purpose"
	std::unordered_map<PNAENEdge,PNAENEdge,PNAENKeyHasher> EdgeReverseMap;

	// "Create an output IB that is 3 times the size of input IB"
	outIndices.resize(inIndices.size() * 3);

	// "For each input Triangle in IB, with indices i0, i1 and i2"
	for(unsigned int i=0;i<inIndices.size();i+=3)
	{
		unsigned int i0 = inIndices[i + 0];
		unsigned int i1 = inIndices[i + 1];
		unsigned int i2 = inIndices[i + 2];

		// "Write out an initial output entry of: i0, i1, i2, i0, i1, i1, i2, i2, i0"

		outIndices[i * 3 + 0] = i0;
		outIndices[i * 3 + 1] = i1;
		outIndices[i * 3 + 2] = i2;
		outIndices[i * 3 + 3] = i0;
		outIndices[i * 3 + 4] = i1;
		outIndices[i * 3 + 5] = i1;
		outIndices[i * 3 + 6] = i2;
		outIndices[i * 3 + 7] = i2;
		outIndices[i * 3 + 8] = i0;

		// "Lookup the positions p0, p1, and p2, using i0, i1 and i2"
		float3 p0 = inVertices[i0].Position;
		float3 p1 = inVertices[i1].Position;
		float3 p2 = inVertices[i2].Position;

		// "Define 3 Edges, which consist of the two indices and two positions that make up the corresponding Edge"
		PNAENEdge e0, r0;
		e0.iO = i0;
		e0.iD = i1;
		e0.pO = p0;
		e0.pD = p1;
		r0 = e0;
		r0.ReverseEdge();

		PNAENEdge e1, r1;
		e1.iO = i1;
		e1.iD = i2;
		e1.pO = p1;
		e1.pD = p2;
		r1 = e1;
		r1.ReverseEdge();

		PNAENEdge e2, r2;
		e2.iO = i2;
		e2.iD = i0;
		e2.pO = p2;
		e2.pD = p0;
		r2 = e2;
		r2.ReverseEdge();

		// "For each edge, store the reverse of that edge in an easily searchable data structure for the next step"
		EdgeReverseMap.emplace(e0, r0);
		EdgeReverseMap.emplace(e1, r1);
		EdgeReverseMap.emplace(e2, r2);
	}

	// "Walk the output index buffer (OB) constructed in step 2. "
	for(unsigned int i=3;i<outIndices.size();i+=9)
	{
		for (int k = 0; k < 6; k += 2)
		{
			int i0 = outIndices[i + k];
			int i1 = outIndices[i + k + 1];
			PNAENEdge temp;
			temp.iO = i1;
			temp.iD = i0;
			temp.pO = inVertices[i1].Position;
			temp.pD = inVertices[i0].Position;

			auto foundIt = EdgeReverseMap.find(temp);
			if (foundIt != EdgeReverseMap.end()) //look up in edge vector
			{
				const PNAENEdge& second = foundIt->second;
				outIndices[i + k] = second.iO;
				outIndices[i + k + 1] = second.iD;
				
			}
		}
	}
}

// Helper struct which defines == for ExVertexStruct
struct Vertex
{
	Vertex(ExVertexStruct* vx)
	{
		this->vx = vx;
	}

	ExVertexStruct* vx;

	bool operator == (const Vertex& o) const
	{
		if (vx->Position == o.vx->Position)
			return true;

		return false;
	}
};

struct VertexKeyHasher
{
	static const size_t bucket_size = 10; // mean bucket size that the container should try not to exceed
	static const size_t min_buckets = (1 << 10); // minimum number of buckets, power of 2, >0

	static std::size_t hash_value(float value)
	{
		stdext::hash<float> hasher;
		return hasher(value);
	}

	static void hash_combine(std::size_t& seed, float value)
	{	
		seed ^= hash_value(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	std::size_t operator()(const Vertex& k) const
	{
		// Start with a hash value of 0    .
		std::size_t seed = 0;

		// Modify 'seed' by XORing and bit-shifting in
		// one member of 'Key' after the other:
		hash_combine(seed, k.vx->Position.x);
		hash_combine(seed, k.vx->Position.y);
		hash_combine(seed, k.vx->Position.z);
							
		hash_combine(seed, k.vx->Position.x);
		hash_combine(seed, k.vx->Position.y);
		hash_combine(seed, k.vx->Position.z);

		// Return the result.
		return seed;
	}
};

struct Float3KeyHasher
{
	static const size_t bucket_size = 10; // mean bucket size that the container should try not to exceed
	static const size_t min_buckets = (1 << 10); // minimum number of buckets, power of 2, >0

	static std::size_t hash_value(float value)
	{
		stdext::hash<float> hasher;
		return hasher(value);
	}

	static void hash_combine(std::size_t& seed, float value)
	{	
		seed ^= hash_value(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	std::size_t operator()(const float3 & k) const
	{
		// Start with a hash value of 0    .
		std::size_t seed = 0;

		// Modify 'seed' by XORing and bit-shifting in
		// one member of 'Key' after the other:
		hash_combine(seed, k.x);
		hash_combine(seed, k.y);
		hash_combine(seed, k.z);
							
		hash_combine(seed, k.x);
		hash_combine(seed, k.y);
		hash_combine(seed, k.z);

		// Return the result.
		return seed;
	}
};

void MeshModifier::ComputePNAEN18Indices(std::vector<ExVertexStruct> & inVertices, const std::vector<unsigned short> & inIndices, std::vector<VERTEX_INDEX> & outIndices, bool detectBorders, bool softNormals)
{
	std::vector<unsigned int> ix;
	std::vector<unsigned int> out;
	ix.assign(inIndices.begin(), inIndices.end());

	// Run the computation
	ComputePNAEN18Indices(inVertices, ix, out, detectBorders, softNormals);

	outIndices.assign(out.begin(), out.end());
}

/** Computes PNAEN-Indices for the given mesh */
void MeshModifier::ComputePNAEN18Indices(std::vector<ExVertexStruct> & inVertices, const std::vector<unsigned int> & inIndices, std::vector<unsigned int> & outIndices, bool detectBorders, bool softNormals)
{
	// PNAEN algorithm from NVIDIA (http://developer.download.nvidia.com/whitepapers/2010/PN-AEN-Triangles-Whitepaper.pdf)
	
	// "For each edge, store the reverse of that edge in an easily searchable data structure for the next step. 
	//  The reference implementation uses an stdext::unordered_map<Edge,Edge> for this purpose"
	std::unordered_map<PNAENEdge,PNAENEdge,PNAENKeyHasher> EdgeReverseMap;

	// "Create an output IB that is 3 times the size of input IB"
	outIndices.resize(inIndices.size() * 3 * 2);

	// Map to store adj. vertices
	std::unordered_map<float3, std::pair<std::vector<unsigned int>, std::vector<ExVertexStruct*>>, Float3KeyHasher> VertexMap;

	for(unsigned int i = 0; i < inIndices.size(); i++)
	{
		// Put adj. vertices of this face together
		VertexMap[inVertices[inIndices[i]].Position].first.push_back(inIndices[i]);
		VertexMap[inVertices[inIndices[i]].Position].second.push_back(&inVertices[inIndices[i]]);
	}

	unsigned int num = 0;
	for(auto it = VertexMap.begin(); it != VertexMap.end(); it++)
	{
		std::vector<ExVertexStruct *> & vx = it->second.second;

		DirectX::SimpleMath::Vector3 nrm = DirectX::SimpleMath::Vector3::Zero;
		if (softNormals)
		{
			// Average normal of all adj. vertices			
			for(unsigned int i=0;i<vx.size();i++)
			{
				nrm += *vx[i]->Normal.toVector3();
			}
			nrm /= vx.size();
		}

		// Set it to all of them
		for(unsigned int i=0;i<vx.size();i++)
		{
			if (detectBorders)
			{
				vx[i]->TexCoord2.x = 0.0f;
				vx[i]->TexCoord2.y = num / 1000.0f;
				num++;
			}
			if (softNormals)
				vx[i]->Normal = nrm;
		}
	}

	// "For each input Triangle in IB, with indices i0, i1 and i2"
	for(unsigned int i=0;i<inIndices.size();i+=3)
	{
		unsigned int i0 = inIndices[i + 0];
		unsigned int i1 = inIndices[i + 1];
		unsigned int i2 = inIndices[i + 2];

		// "Write out an initial output entry of: i0, i1, i2, i0, i1, i1, i2, i2, i0"

		outIndices[i * 6 + 0] = i0;
		outIndices[i * 6 + 1] = i1;
		outIndices[i * 6 + 2] = i2;
					   
		// Adj.		   
		outIndices[i * 6 + 3] = i0;
		outIndices[i * 6 + 4] = i1;
		outIndices[i * 6 + 5] = i1;
		outIndices[i * 6 + 6] = i2;
		outIndices[i * 6 + 7] = i2;
		outIndices[i * 6 + 8] = i0;
					   
		// Dom.		   
		outIndices[i * 6 + 9] = i0;
		outIndices[i * 6 + 10] = i1;
		outIndices[i * 6 + 11] = i1;
		outIndices[i * 6 + 12] = i2;
		outIndices[i * 6 + 13] = i2;
		outIndices[i * 6 + 14] = i0;
					   
		// Dom. UV	   
		outIndices[i * 6 + 15] = i0;
		outIndices[i * 6 + 16] = i1;
		outIndices[i * 6 + 17] = i2;

		// "Lookup the positions p0, p1, and p2, using i0, i1 and i2"
		float3 p0 = inVertices[i0].Position;
		float3 p1 = inVertices[i1].Position;
		float3 p2 = inVertices[i2].Position;

		// "Define 3 Edges, which consist of the two indices and two positions that make up the corresponding Edge"
		PNAENEdge e0, r0;
		e0.iO = i0;
		e0.iD = i1;
		e0.pO = p0;
		e0.pD = p1;
		r0 = e0;
		r0.ReverseEdge();

		PNAENEdge e1, r1;
		e1.iO = i1;
		e1.iD = i2;
		e1.pO = p1;
		e1.pD = p2;
		r1 = e1;
		r1.ReverseEdge();

		PNAENEdge e2, r2;
		e2.iO = i2;
		e2.iD = i0;
		e2.pO = p2;
		e2.pD = p0;
		r2 = e2;
		r2.ReverseEdge();

		// "For each edge, store the reverse of that edge in an easily searchable data structure for the next step"
		EdgeReverseMap.emplace(e0, r0);
		EdgeReverseMap.emplace(e1, r1);
		EdgeReverseMap.emplace(e2, r2);
	}

	// "Walk the output index buffer (OB) constructed in step 2. "
	for(unsigned int i=3;i<outIndices.size();i+=18)
	{
		for (int k = 0; k < 6; k += 2)
		{
			int i0 = outIndices[i + k];
			int i1 = outIndices[i + k + 1];
			PNAENEdge temp;
			temp.iO = i1;
			temp.iD = i0;
			temp.pO = inVertices[i1].Position;
			temp.pD = inVertices[i0].Position;

			bool domReversed = false;
			PNAENEdge domEdge;
			domEdge.iO = 0;
			domEdge.iD = 0;

			auto foundIt = EdgeReverseMap.find(temp);
			if (foundIt != EdgeReverseMap.end()) //look up in edge vector
			{
				const PNAENEdge& second = foundIt->second;
				outIndices[i + k] = second.iO;
				outIndices[i + k + 1] = second.iD;

				domEdge = second;
			} else
			{
				temp.ReverseEdge();
				foundIt = EdgeReverseMap.find(temp);
				if (foundIt != EdgeReverseMap.end()) 
				{
					const PNAENEdge& second = foundIt->second;
					domEdge = second;
					domEdge.ReverseEdge();
				} else
				{
					
				}
				/*temp.ReverseEdge(); // Look again for reverse
				auto foundIt = EdgeReverseMap.find(temp);
				if (foundIt != EdgeReverseMap.end()) //look up in edge vector
				{
					domEdge = foundIt->second;
					domReversed = true;
					temp.ReverseEdge();
				}*/
			}

			if (!(temp < domEdge) && !(domEdge < temp))
			{
				inVertices[i0].TexCoord2.x = 1.0f;
				inVertices[i1].TexCoord2.x = 1.0f;
			}

			if (detectBorders)
			{
				//continue;
				outIndices[i + 6 + k] = i0;
				outIndices[i + 6 + k + 1] = i1;

				if (domEdge.iO != 0 && domEdge.iD != 0)
				{
					//temp.ReverseEdge();

					if (domEdge < temp) // Find edge with lower indices
					{
						outIndices[i + 6 + k] = domEdge.iO;
						outIndices[i + 6 + k + 1] = domEdge.iD;
					}/*else
					{
						//if (domReversed)
						temp.ReverseEdge();

						outIndices[i + 6 + k] = temp.iO;
						outIndices[i + 6 + k + 1] = temp.iD;
					}*/
				} else
				{
				

					//temp.ReverseEdge();
					/*outIndices[i + 6 + k] = temp.iO;
					outIndices[i + 6 + k + 1] = temp.iD;*/
				}
			}
		}

		if (detectBorders)
		{
			// Dom. UV-Coords
			for(int k=0;k<3;k++)
			{
				float3 v = inVertices[outIndices[(i - 3) + k]].Position;
				std::pair<std::vector<unsigned int>, std::vector<ExVertexStruct*>> & adj = VertexMap[v];


				float2 smallest = float2(FLT_MAX, FLT_MAX);
				int smallestIdx = outIndices[(i - 3) + k];
				bool isBorder = false;
				for(unsigned int j=0;j<adj.first.size();j++)
				{
					if (adj.second[j]->TexCoord < smallest)
					{
						smallest = adj.second[j]->TexCoord;
						smallestIdx = adj.first[j];
					}

					if (adj.second[j]->TexCoord2.x == 1.0f)
						isBorder = true;
				}

				// If one vertex is a border vertex, apply that to all of them
				if (isBorder)
				{
					for(unsigned int j=0;j<adj.first.size();j++)
					{
						adj.second[j]->TexCoord2.x = 1.0f;
					}
				}

				outIndices[(i - 3) + 15 + k] = smallestIdx;
			}
		}
	}
}


bool TexcoordSame(float2 a, float2 b)
{
	if ((abs(a.x - 		b.x) > 0.001f &&
		abs((a.x + 1) - b.x) > 0.001f &&
		abs((a.x - 1) - b.x) > 0.001f) ||
		(abs(a.y - 		b.y) > 0.001f &&
		abs((a.y + 1) - b.y) > 0.001f &&
		abs((a.y - 1) - b.y) > 0.001f))
		return false;
		
	return true;
};

/** Computes smooth normals for the given mesh */
void MeshModifier::ComputeSmoothNormals(std::vector<ExVertexStruct> & inVertices)
{


	// Map to store adj. vertices
	std::unordered_map<Vertex, std::vector<ExVertexStruct*>, VertexKeyHasher> VertexMap;

	for(unsigned int i = 0; i < inVertices.size(); i+=3)
	{
		for(int x=0;x<3;x++)
		{
			// Put adj. vertices of this face together
			VertexMap[Vertex(&inVertices[i+x])].push_back(&inVertices[i+x]);
		}
	}

	// Run through all the adj. vertices and average the normals between them
	for(auto it = VertexMap.begin(); it != VertexMap.end(); it++)
	{
		std::vector<ExVertexStruct *> & vx = it->second;
		// Average all face normals
		DirectX::SimpleMath::Vector3 avgNormal = DirectX::SimpleMath::Vector3::Zero;
		for(unsigned int i=0;i<vx.size();i++)
		{
			avgNormal += *vx[i]->Normal.toVector3();
		}
		avgNormal /= (float)vx.size();

		// Lerp between the average and the face normal for every vertex
		for(unsigned int i=0;i<vx.size();i++)
		{
			// Find out if we are a corner/border vertex
			vx[i]->TexCoord2.x = 1.0f;
			for(unsigned int n=0;n<vx.size();n++)
			{				
				if (!TexcoordSame(vx[i]->TexCoord, vx[n]->TexCoord))
				{
					vx[i]->TexCoord2.x = 0.0f;
					break;
				}
			}

			vx[i]->Normal = avgNormal;
			//D3DXVec3Lerp(vx[i]->Normal.toVector3(), &avgNormal, vx[i]->Normal.toVector3(), 0.7f);
		}
	}
}

/** Fills an index array for a non-indexed mesh */
void MeshModifier::FillIndexArrayFor(unsigned int numVertices, std::vector<unsigned int> & outIndices)
{
	for(unsigned int i=0;i<numVertices;i++)
	{
		outIndices.push_back(i);
	}
}


/** Fills an index array for a non-indexed mesh */
void MeshModifier::FillIndexArrayFor(unsigned int numVertices, std::vector<VERTEX_INDEX> & outIndices)
{
	for(unsigned int i=0;i<numVertices;i++)
	{
		outIndices.push_back(i);
	}
}