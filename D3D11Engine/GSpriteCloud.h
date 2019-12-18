#pragma once
#include "pch.h"

class GSpriteCloud
{
public:
	GSpriteCloud();
	~GSpriteCloud();

	/** Initializes this cloud */
	void CreateCloud(const DirectX::SimpleMath::Vector3 & size, int numSprites = 10);

	/** Returns the sprite world matrices */
	const std::vector<DirectX::SimpleMath::Matrix> & GetWorldMatrices();

protected:
	/** World matrices for the sprites */
	std::vector<DirectX::SimpleMath::Vector3> Sprites;

	/** Sprite positions as world matrices */
	std::vector<DirectX::SimpleMath::Matrix> SpriteWorldMatrices;
};

