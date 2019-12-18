#include "pch.h"
#include "GSpriteCloud.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

GSpriteCloud::GSpriteCloud()
{
}


GSpriteCloud::~GSpriteCloud()
{
}

struct CloudBB
{
	void MakeRandom(const Vector3 & center, const Vector3 & minSize, const Vector3 & maxSize)
	{
		Vector3 d = maxSize - minSize;
		
		// Random Box size
		Vector3 sr = Vector3(minSize.x + Toolbox::frand() * d.x,
			minSize.y + Toolbox::frand() * d.y,
			minSize.z + Toolbox::frand() * d.z);
		sr /= 2.0f;
		Size = sr;

		Center = center;
	}

	Vector3 GetRandomPointInBox()
	{
		Vector3 r = Vector3((Toolbox::frand() * Size.x * 2) - Size.x,
			(Toolbox::frand() * Size.y * 2) - Size.y,
			(Toolbox::frand() * Size.z * 2) - Size.z);

		return r;
	}

	Vector3 Center;
	Vector3 Size;
};

/** Initializes this cloud */
void GSpriteCloud::CreateCloud(const Vector3 & size, int numSprites)
{
	CloudBB c;
	c.MakeRandom(Vector3::Zero, size / 2.0f, size);

	// Fill the bb with sprites
	for(int i=0;i<numSprites;i++)
	{
		Vector3 rnd = c.GetRandomPointInBox();
		Sprites.push_back(rnd);

		DirectX::SimpleMath::Matrix m = XMMatrixTranslation(rnd.x, rnd.y, rnd.z);

		SpriteWorldMatrices.push_back(m);
	}
}