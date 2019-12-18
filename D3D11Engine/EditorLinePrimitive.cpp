#include "pch.h"
#include "EditorLinePrimitive.h"
#include "D3D11GraphicsEngineBase.h"
#include "Engine.h"
#include "D3D11PShader.h"
#include "BaseLineRenderer.h"
#include "GothicAPI.h"
#include "D3D11ShaderManager.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

EditorLinePrimitive::EditorLinePrimitive()
{
	Vertices = nullptr;
	PrimVB = nullptr;
	PrimShader = nullptr;
	Location = Vector3::Zero;
	Rotation = Vector3::Zero;
	Scale = Vector3::One;
	RecalcTransforms();
	bHidden=false;

	SolidPrimShader = nullptr;
	SolidVertices = nullptr;
	NumSolidVertices = 0;
	SolidPrimVB = nullptr;

	NumVertices = 0;

	RotationMatrixAngles=Vector3::Zero;
	RotationMatrix = Matrix::Identity;
	bLocalRotation=false;

	bJustUseRotationMatrix=false;

	SetSolidShader(((D3D11GraphicsEngineBase *)Engine::GraphicsEngine)->GetShaderManager()->GetPShader("PS_Lines"));
	SetShader(((D3D11GraphicsEngineBase *)Engine::GraphicsEngine)->GetShaderManager()->GetPShader("PS_Lines"));
}


EditorLinePrimitive::~EditorLinePrimitive()
{
	delete[] Vertices;
	delete[] SolidVertices;
	if (PrimVB)PrimVB->Release();
	if (SolidPrimVB)SolidPrimVB->Release();
}

/** Deletes all content */
void EditorLinePrimitive::DeleteContent()
{
	delete[] Vertices;
	delete[] SolidVertices;
	if (PrimVB)PrimVB->Release();
	if (SolidPrimVB)SolidPrimVB->Release();
}

/** Creates a grid of lines */
HRESULT EditorLinePrimitive::CreateLineGrid(int LinesX,int LinesY,Vector2 * Middle, Vector4 * Color)
{
	HRESULT hr;
	LineVertex * vx = new LineVertex[(LinesX+1)*(LinesY+1)*4];
	UINT CurVertex=0;

	//Fill X based lines
	float x;
	float SpacingX=(1.0f/LinesX);
	for(x= -0.5; x<=0.5f+SpacingX; x+=SpacingX)
	{

		vx[CurVertex].Position=Vector3(x, 0,-0.5)+ Vector3(Middle->x, 0,Middle->y);
		EncodeColor(&vx[CurVertex], Color);
		CurVertex++;
		vx[CurVertex].Position=Vector3(x, 0, 0.5)+ Vector3(Middle->x, 0,Middle->y);
		EncodeColor(&vx[CurVertex], Color);
		CurVertex++;
	}

	//Fill Z based lines
	float z;
	float SpacingY=(1.0f/LinesY);
	for(z= -0.5; z<=0.5f+SpacingY; z+=SpacingY)
	{
		vx[CurVertex].Position=Vector3(-0.5, 0,z)+ Vector3(Middle->x, 0,Middle->y);
		EncodeColor(&vx[CurVertex], Color);
		CurVertex++;
		vx[CurVertex].Position=Vector3(0.5, 0,z)+ Vector3(Middle->x, 0,Middle->y);
		EncodeColor(&vx[CurVertex], Color);
		CurVertex++;
	}


	LE(CreatePrimitive(vx, (LinesX+1)*(LinesY+1)*4));

	delete[] vx;
	return hr;
}

/** Creates a box of lines */
HRESULT EditorLinePrimitive::CreateLineBoxPrimitive(Vector4 * Color)
{
	LineVertex vx[24];

	// Bottom
	vx[0].Position = Vector3(-1,-1,-1);
	EncodeColor(&vx[0], Color);
	vx[1].Position = Vector3(1,-1,-1);
	EncodeColor(&vx[1], Color);

	vx[2].Position = Vector3(1,-1,-1);
	EncodeColor(&vx[2], Color);
	vx[3].Position = Vector3(1,-1, 1);
	EncodeColor(&vx[3], Color);

	vx[4].Position = Vector3(1,-1, 1);
	EncodeColor(&vx[4], Color);
	vx[5].Position = Vector3(-1,-1, 1);
	EncodeColor(&vx[5], Color);

	vx[6].Position = Vector3(-1,-1, 1);
	EncodeColor(&vx[6], Color);
	vx[7].Position = Vector3(-1,-1,-1);
	EncodeColor(&vx[7], Color);

	// Sides | | | |

	vx[8].Position = Vector3(-1,-1,-1);
	EncodeColor(&vx[8], Color);
	vx[9].Position = Vector3(-1, 1,-1);
	EncodeColor(&vx[9], Color);

	vx[10].Position = Vector3(1,-1,-1);
	EncodeColor(&vx[10], Color);
	vx[11].Position = Vector3(1, 1,-1);
	EncodeColor(&vx[11], Color);

	vx[12].Position = Vector3(1,-1, 1);
	EncodeColor(&vx[12], Color);
	vx[13].Position = Vector3(1, 1, 1);
	EncodeColor(&vx[13], Color);

	vx[14].Position = Vector3(-1,-1, 1);
	EncodeColor(&vx[14], Color);
	vx[15].Position = Vector3(-1, 1, 1);
	EncodeColor(&vx[15], Color);

	// Top
	vx[16].Position = Vector3(-1, 1,-1);
	EncodeColor(&vx[16], Color);
	vx[17].Position = Vector3(1, 1,-1);
	EncodeColor(&vx[17], Color);

	vx[18].Position = Vector3(1, 1,-1);
	EncodeColor(&vx[18], Color);
	vx[19].Position = Vector3(1, 1, 1);
	EncodeColor(&vx[19], Color);

	vx[20].Position = Vector3(1, 1, 1);
	EncodeColor(&vx[20], Color);
	vx[21].Position = Vector3(-1, 1, 1);
	EncodeColor(&vx[21], Color);

	vx[22].Position = Vector3(-1, 1, 1);
	EncodeColor(&vx[22], Color);
	vx[23].Position = Vector3(-1, 1,-1);
	EncodeColor(&vx[23], Color);

	HRESULT hr;
	LE(CreatePrimitive(vx, 24));

	return hr;
}

/** Creates a solid box */
HRESULT EditorLinePrimitive::CreateSolidBoxPrimitive(Vector4 * Color, float Extends)
{
	LineVertex vx[36];
	int i=0;

	vx[i].Position = Vector3(-1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1,-1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1,-1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1,-1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1, 1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1, 1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1, 1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1, 1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1, 1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1, 1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1,-1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1, 1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1, 1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1, 1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1, 1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1,-1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1, 1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1, 1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1, 1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1,-1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1, 1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1, 1,-1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1,-1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1,-1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1, 1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1,-1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(1, 1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	vx[i].Position = Vector3(-1, 1, 1);
	EditorLinePrimitive::EncodeColor(&vx[i++], Color);

	// Loop through all vertices and apply the extends
	for(i = 0; i < 36; i++)
	{
		*(Vector4 *)&vx[i].Position *= Extends;
	}

	HRESULT hr;
	LE(CreateSolidPrimitive(&vx[0], 36));

	return hr;
}

/** Creates the buffers and sets up the rest od the object */
HRESULT EditorLinePrimitive::CreateSolidPrimitive(LineVertex * PrimVerts, UINT NumVertices, D3D11_PRIMITIVE_TOPOLOGY Topology)
{
	HRESULT hr=S_OK;

	// Clean up previous data
	delete[] SolidVertices;
	if (SolidPrimVB)SolidPrimVB->Release();

	// Copy over the new data
	SolidVertices = new LineVertex[NumVertices];
	memcpy(SolidVertices, PrimVerts, sizeof(LineVertex)*NumVertices);
	this->NumSolidVertices = NumVertices;
	
	//Create the vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.ByteWidth = NumVertices * sizeof(LineVertex);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = &SolidVertices[0];
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase*)Engine::GraphicsEngine;

	LE(engine->GetDevice()->CreateBuffer(&bufferDesc,&InitData,&SolidPrimVB));

	SolidPrimitiveTopology = Topology;

	return hr;
}


/** Creates a plate, not of lines. Can't use intersection on this*/
HRESULT EditorLinePrimitive::CreateFilledCirclePrimitive(float Radius, UINT Detail, const Vector4 * Color, int Axis)
{
	UINT NumVerts = Detail*3;
	LineVertex * vx = new LineVertex[NumVerts];

	float Step = (XM_PI*2)/((float)(Detail)-1);
	float s = 0;

	size_t i=0;
	while(i < NumVerts)
	{
		switch(Axis)
		{
		case 0:
			// XZ-Axis
			vx[i].Position = Vector3(sinf(s), 0, cosf(s));
			break;
		
		case 1:
			// YZ-Axis
			vx[i].Position = Vector3(0, sinf(s), cosf(s));
			break;

		case 2:
			// ZY-Axis
			vx[i].Position = Vector3(sinf(s), cosf(s), 0);
			break;
		}
		EncodeColor(&vx[i], Color);
		s+=Step;
		i++;


		switch(Axis)
		{
		case 0:
			// XZ-Axis
			vx[i].Position = Vector3(sinf(s), 0, cosf(s));
			break;
		
		case 1:
			// YZ-Axis
			vx[i].Position = Vector3(0, sinf(s), cosf(s));
			break;

		case 2:
			// ZY-Axis
			vx[i].Position = Vector3(sinf(s), cosf(s), 0);
			break;
		}
		EncodeColor(&vx[i], Color);
		//s+=Step;
		i++;

		vx[i].Position = Vector3::Zero;
		EncodeColor(&vx[i], Color);
		i++;

	}

	HRESULT hr = CreatePrimitive(vx, NumVerts, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	delete[] vx;
	return hr;
}

/** Creates a circle of lines */
HRESULT EditorLinePrimitive::CreateCirclePrimitive(float Radius, UINT Detail, const Vector4 * Color, int Axis)
{
	LineVertex * vx = new LineVertex[Detail];

	float Step = (XM_PI*2)/((float)Detail-1);
	float s = 0;

	for(UINT i = 0; i < Detail; i++)
	{
		switch(Axis)
		{
		case 0:
			// XZ-Axis
			vx[i].Position = Vector3(sinf(s), 0, cosf(s));
			break;
		
		case 1:
			// YZ-Axis
			vx[i].Position = Vector3(0, sinf(s), cosf(s));
			break;

		case 2:
			// ZY-Axis
			vx[i].Position = Vector3(sinf(s), cosf(s), 0);
			break;
		}
		
		EncodeColor(&vx[i], Color);

		s+=Step;
	}

	HRESULT hr = CreatePrimitive(vx, Detail, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	delete[] vx;
	return hr;
}

/** Creates a ball of lines.*/
HRESULT EditorLinePrimitive::CreateLineBallPrimitive(UINT Detail, const Vector4 * Color)
{
	// Allocate enough memory for all 3 slices
	LineVertex * vx = new LineVertex[(Detail*3)];

	float Step = (XM_PI*2)/((float)Detail-1);
	float s = 0;

	// Create first
	for(UINT i = 0; i < Detail; i++)
	{
		vx[i].Position = Vector3(sinf(s), 0, cosf(s));
			
		EncodeColor(&vx[i], Color);

		s+=Step;
	}

	s=0;

	// Create second
	for(UINT i = Detail; i < Detail*2; i++)
	{
		vx[i].Position = Vector3(0, sinf(s), cosf(s));
			
		EncodeColor(&vx[i], Color);

		s+=Step;
	}

	s=0;

	// Create third
	for(UINT i = Detail*2; i < Detail*3; i++)
	{
		vx[i].Position = Vector3(sinf(s), cosf(s), 0);
			
		EncodeColor(&vx[i], Color);

		s+=Step;
	}

	// Fix the last position
	//vx[(Detail*3)].Position = Vector3(sinf(s), cosf(s), 0);

	HRESULT hr = CreatePrimitive(vx, (Detail*3), D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	delete[] vx;

	return hr;
}


/** Creates an arrow of lines */
HRESULT EditorLinePrimitive::CreateArrowPrimitive(const Vector4 * Color,float ArrowRadius, float ArrowOffset)
{
	LineVertex vx[10];

	

	// Middle streak
	vx[0].Position = Vector3::Zero;
	vx[1].Position = Vector3::Right;

	// Outer lines
	vx[2].Position = Vector3::Right;
	vx[3].Position = Vector3(ArrowOffset, ArrowRadius, ArrowRadius);

	vx[4].Position = Vector3::Right;
	vx[5].Position = Vector3(ArrowOffset, ArrowRadius, -ArrowRadius);

	vx[6].Position = Vector3::Right;
	vx[7].Position = Vector3(ArrowOffset, -ArrowRadius, ArrowRadius);

	vx[8].Position = Vector3::Right;
	vx[9].Position = Vector3(ArrowOffset, -ArrowRadius, -ArrowRadius);

	for(int i=0; i< 10; i++)
	{
		EncodeColor(&vx[i], Color);
	}
	HRESULT hr = CreatePrimitive(vx, 10);
	return hr;
}

/** Creates a cone of lines */
HRESULT EditorLinePrimitive::CreateSimpleConePrimitive(float Length, float Radius, UINT Detail, const Vector4 * Color)
{
	UINT NumVerts;
	NumVerts = Detail*2; // Two for each connection line
	NumVerts += Detail*2; // Two for each circle line

	LineVertex * vx = new LineVertex[NumVerts];

	float Step = (XM_PI*2)/((float)Detail-1);
	float s = 0;

	UINT i = 0;
	while(i < NumVerts)
	{
		// First vertex of the circle-line
		vx[i].Position = Vector3(Length, (sinf(s)*Radius), cosf(s)*Radius);	
		EncodeColor(&vx[i], Color);
		i++;

		s+=Step;

		// Second vertex of the circle-line
		vx[i].Position = Vector3(Length, (sinf(s)*Radius), cosf(s)*Radius);	
		EncodeColor(&vx[i], Color);

		i++;
		
		// Connector line #1
		vx[i].Position = vx[i-2].Position;
		EncodeColor(&vx[i], Color);
		i++;

		// Connector line #2
		vx[i].Position = Vector3::Zero;
		EncodeColor(&vx[i], Color);
		i++;

		

		
	}

	HRESULT hr = CreatePrimitive(vx, NumVerts);

	delete[] vx;

	return hr;
}

/** Puts the color into the Normal and TexCoord channels */
void EditorLinePrimitive::EncodeColor(LineVertex * vx,const DirectX::SimpleMath::Vector4 * Color)
{
	vx->Color = *Color;
}


/** Intersects the whole primitive. If hit, it returns a distance other than -1 */
float EditorLinePrimitive::IntersectPrimitive(Vector3 * RayOrigin, Vector3 * RayDirection, float Epsilon)
{
	UINT i;
	float Shortest = -1;

	// Bring the ray into object space
	DirectX::SimpleMath::Matrix invWorld = DirectX::XMMatrixInverse(nullptr, WorldMatrix);

	Vector3 Origin = XMVector3TransformCoord(*RayOrigin, invWorld);
	Vector3 Dir = XMVector3TransformNormal(*RayDirection, invWorld);

	// Go through all line segments and intersect them
	for(i = 0; i < NumVertices; i+=2)
	{	
		Vector3 vx[2];
		vx[0] = XMVector3TransformCoord(*Vertices[i].Position.toVector3(), WorldMatrix);
		vx[1] = XMVector3TransformCoord(*Vertices[i+1].Position.toVector3(), WorldMatrix);

		//float Dist = IntersectLineSegment(&Origin, &Dir, Vertices[i].Position.toVector3(), Vertices[i+1].Position.toVector3(), Epsilon);
		float Dist = IntersectLineSegment(RayOrigin, RayDirection, &vx[0], &vx[1], Epsilon);
		if (Dist < Shortest || Shortest == -1)
		{
			Shortest = Dist / Scale.x;
		}
	}

	if (NumSolidVertices>0)
	{
		FLOAT fBary1, fBary2;
		FLOAT fDist;
		int NumIntersections=0;
		Shortest = FLT_MAX;
		
		for(DWORD i = 0; i < NumSolidVertices; i+=3)
		{
			Vector3 v0 = *SolidVertices[i + 0].Position.toVector3();
			Vector3 v1 = *SolidVertices[i + 1].Position.toVector3();
			Vector3 v2 = *SolidVertices[i + 2].Position.toVector3();

			// Check if the pick ray passes through this point
			if (IntersectTriangle(&Origin, &Dir, v0, v1, v2,
				&fDist, &fBary1, &fBary2))
			{
				if (fDist < Shortest  || Shortest == -1)
				{
					NumIntersections++;
					Shortest=0;
								
					//if (NumIntersections == MAX_INTERSECTIONS)
					//	break;
				}
			}
		}
	}

	return Shortest;
}

bool EditorLinePrimitive::IntersectTriangle(const Vector3 * orig, const Vector3 * dir,
                        Vector3 & v0, Vector3 & v1, Vector3 & v2,
                        FLOAT* t, FLOAT* u, FLOAT* v)
{
    // Find vectors for two edges sharing vert0
    Vector3 edge1 = v1 - v0;
    Vector3 edge2 = v2 - v0;

    // Begin calculating determinant - also used to calculate U parameter
	Vector3 pvec = dir->Cross(edge2);

    // If determinant is near zero, ray lies in plane of triangle
    FLOAT det = edge1.Dot(pvec);

    Vector3 tvec;
    if (det > 0)
    {
        tvec = (*orig) - v0;
    }
    else
    {
        tvec = v0 - (*orig);
        det = -det;
    }

    if (det < 0.0001f)
        return FALSE;

    // Calculate U parameter and test bounds
    *u = tvec.Dot(pvec);
    if (*u < 0.0f || *u > det)
        return FALSE;

    // Prepare to test V parameter
    Vector3 qvec = tvec.Cross(edge1);

    // Calculate V parameter and test bounds
    *v = dir->Dot(qvec);
    if (*v < 0.0f || *u + *v > det)
        return FALSE;

    // Calculate t, scale parameters, ray intersects triangle
    *t = edge2.Dot(qvec);
    FLOAT fInvDet = 1.0f / det;
    *t *= fInvDet;
    *u *= fInvDet;
    *v *= fInvDet;

    return TRUE;
}


float EditorLinePrimitive::IntersectLineSegment(const Vector3 * rayOrigin,const Vector3 * rayVec,const Vector3 * lineStart,const Vector3 * lineEnd, float Epsilon)
{
	
	Vector3 u = *rayVec;
	Vector3 v = (*lineEnd) - (*lineStart);
	Vector3 w = (*rayOrigin) - (*lineStart);

	
	float a = u.Dot(u);	// always >= 0
	float b = u.Dot(v);
	float c = v.Dot(v);	// always >= 0
	float d = u.Dot(w);
	float e = v.Dot(w);
	float D = a*c - b*b;	// always >= 0
	float sc, sN, sD = D;	// sc = sN / sD, default sD = D >= 0
	float tc, tN, tD = D;	// tc = tN / tD, default tD = D >= 0


	// compute the line parameters of the two closest points
	if (D < Epsilon) {	// the lines are almost parallel
		sN = 0.0;			// force using point P0 on segment S1
		sD = 1.0;			// to prevent possible division by 0.0 later
		tN = e;
		tD = c;
	}
	else {				// get the closest points on the infinite lines
		sN = (b*e - c*d);
		tN = (a*e - b*d);
		if (sN < 0.0) {	// sc < 0 => the s=0 edge is visible
			sN = 0.0;
			tN = e;
			tD = c;
		}
	}

	if (tN < 0.0) {		// tc < 0 => the t=0 edge is visible
		tN = 0.0;
		// recompute sc for this edge
		if (-d < 0.0)
			sN = 0.0;
		else {
			sN = -d;
			sD = a;
		}
	}
	else if (tN > tD) {	  // tc > 1 => the t=1 edge is visible
		tN = tD;
		// recompute sc for this edge
		if ((-d + b) < 0.0)
			sN = 0;
		else {
			sN = (-d + b);
			sD = a;
		}
	}
	// finally do the division to get sc and tc
	sc = (abs(sN) < Epsilon ? 0.0f : sN / sD);
	tc = (abs(tN) < Epsilon ? 0.0f : tN / tD);

	// get the difference of the two closest points
	Vector3 dP = w + (sc * u) - (tc * v);	// = S1(sc) - S2(tc)
	//info.iFracRay = sc;
	//info.iFracLine = tc;
	return dP.Length();	// return the closest distance
}

void EditorLinePrimitive::SetWorldMatrix(const DirectX::SimpleMath::Matrix* World,const Vector3 * Loc,const Vector3 * Rot,const Vector3 * Scale)
{
	Location=*Loc;
	Rotation=*Rot;
	this->Scale=*Scale;
	WorldMatrix = *World;
}

void EditorLinePrimitive::SetLocation(const Vector3 & NewLoc)
{
	Location=NewLoc;
	RecalcTransforms();
}

void EditorLinePrimitive::SetRotation(const Vector3 & NewRotation)
{
	Rotation=NewRotation;
	RecalcTransforms();
}

void EditorLinePrimitive::SetScale(const Vector3 & NewScale)
{
	Scale=NewScale;
	RecalcTransforms();
}

void EditorLinePrimitive::RecalcTransforms()
{
	//Matrices we need
	DirectX::SimpleMath::Matrix matWorld,matScale,MatRot,MatTemp;

	//Temporary translation
	Vector3 Trans;


	//Copy from the original location, 
	//so we can modify it without hurting anything
	Trans=Location;

	//Devide Trans through Scale
	/*Trans.x/=Scale.x;
	Trans.y/=Scale.y;
	Trans.z/=Scale.z;*/


	//Apply translation to the WorldMatrix
	matWorld = XMMatrixTranslation(Trans.x,Trans.y,Trans.z);

	//Now scale another matrix
	matScale = XMMatrixScaling(Scale.x, Scale.y, Scale.z);

	//Apply rotation
	MatRot = Matrix::Identity;
	Vector3 DeltaRot = Rotation - RotationMatrixAngles;

	if (Rotation != Vector3::Zero)
	{
		// Calculate matrix with the new angles
		if (bLocalRotation)
		{
			Vector3 Up(0, 1, 0);
			Vector3 Front(1, 0, 0);
			Vector3 Right;
			
			Up = XMVector3TransformNormal(Up, RotationMatrix);
			Front = XMVector3TransformNormal(Front, RotationMatrix);
			Right = Up.Cross(Front);

			DirectX::SimpleMath::Matrix X = XMMatrixRotationAxis(Front, DeltaRot.x);
			DirectX::SimpleMath::Matrix Y = XMMatrixRotationAxis(Up, DeltaRot.y);
			DirectX::SimpleMath::Matrix Z = XMMatrixRotationAxis(Right, DeltaRot.z);

			RotationMatrix *= X * Y * Z;
		} else
		{
			MatRot = Matrix::Identity;

			MatTemp = XMMatrixRotationAxis(Vector3(1, 0, 0), Rotation.x);        // Pitch
			MatRot = XMMatrixMultiply(MatRot, MatTemp);
			MatTemp = XMMatrixRotationAxis(Vector3(0, 1, 0), Rotation.y);         // Yaw
			MatRot = XMMatrixMultiply(MatRot, MatTemp);
			MatTemp = XMMatrixRotationAxis(Vector3(0, 0, 1), Rotation.z);       // Roll
			RotationMatrix = XMMatrixMultiply(MatRot, MatTemp);

			//RotationMatrix = X * Y * Z;
		}

		RotationMatrixAngles = Rotation;
	} else if (!bJustUseRotationMatrix)
	{
		// Reset matrix to identity (Todo: ROTATION! Ò.ó Y U NO WORK!? (As I want))
		RotationMatrix = Matrix::Identity;
		RotationMatrixAngles = Vector3::Zero;
	}


	WorldMatrix = matScale * RotationMatrix * matWorld;
	
}

/** Creates the buffers and sets up the rest od the object */
HRESULT EditorLinePrimitive::CreatePrimitive(LineVertex * PrimVerts, UINT NumVertices, D3D11_PRIMITIVE_TOPOLOGY Topology)
{
	HRESULT hr=S_OK;

	// Clean up previous data
	DeleteContent();

	// Copy over the new data
	Vertices = new LineVertex[NumVertices];
	memcpy(Vertices, PrimVerts, sizeof(LineVertex)*NumVertices);
	this->NumVertices = NumVertices;
	
	//Create the vertex buffer
	D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.ByteWidth = NumVertices * sizeof(LineVertex);
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = &Vertices[0];
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase*)Engine::GraphicsEngine;

	LE(engine->GetDevice()->CreateBuffer(&bufferDesc,&InitData,&PrimVB));

	PrimitiveTopology = Topology;

	return hr;
}

/** Sets the shader to render with */
void EditorLinePrimitive::SetShader(D3D11PShader * Shader)
{
	PrimShader = Shader;
}

/** Sets the solid shader to render with */
void EditorLinePrimitive::SetSolidShader(D3D11PShader * SolidShader)
{
	SolidPrimShader = SolidShader;
}

/** Renders a vertexbuffer with the given shader */
void EditorLinePrimitive::RenderVertexBuffer(ID3D11Buffer* VB, UINT NumVertices, D3D11PShader * Shader, D3D11_PRIMITIVE_TOPOLOGY Topology, int Pass)
{
	D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase*)Engine::GraphicsEngine;

	DirectX::SimpleMath::Matrix tr = WorldMatrix.Transpose();
	Engine::GAPI->SetWorldTransform(tr);

	engine->SetActiveVertexShader("VS_Lines");
	engine->SetActivePixelShader("PS_Lines");
	
	engine->SetupVS_ExMeshDrawCall();
	engine->SetupVS_ExConstantBuffer();
	engine->SetupVS_ExPerInstanceConstantBuffer();

	engine->SetDefaultStates();
	Engine::GAPI->GetRendererState()->BlendState.SetAlphaBlending();
	Engine::GAPI->GetRendererState()->BlendState.SetDirty();
	engine->UpdateRenderStates();

	Shader->Apply();

	// Set vertex buffer
	UINT stride = sizeof(LineVertex);
	UINT offset = 0;
	engine->GetContext()->IASetVertexBuffers(0, 1, &VB, &stride, &offset);
	engine->GetContext()->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	
	engine->GetContext()->IASetPrimitiveTopology(Topology);

	engine->GetContext()->Draw(NumVertices, 0);
}

HRESULT EditorLinePrimitive::RenderPrimitive(int Pass)
{
	if (!PrimShader && !SolidPrimShader)
	{
		return E_FAIL;
	}

	if (bHidden)
	{
		return S_OK;
	}

	if (NumVertices > 0 && PrimShader)
	{
		RenderVertexBuffer(PrimVB, NumVertices, PrimShader, PrimitiveTopology, Pass);
	}

	if (NumSolidVertices > 0 && SolidPrimShader)
	{
		RenderVertexBuffer(SolidPrimVB, NumSolidVertices, SolidPrimShader, SolidPrimitiveTopology, Pass);
	}
	
	return S_OK;
}
