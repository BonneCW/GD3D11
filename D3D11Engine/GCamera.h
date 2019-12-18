#pragma once
#include "pch.h"

class GCamera
{
public:
	GCamera();
	~GCamera();



	/** Recreates the view matrix. Only call after you changed a value! */
	void RecreateViewMatrix(){};

	/** Returns the View-Matrix*/
	DirectX::SimpleMath::Matrix& GetView(){return View;}

	/** Returns the WorldPosition */
	DirectX::SimpleMath::Vector3 & GetWorldPosition(){return WorldPosition;}

	/** Returns the WorldPosition */
	DirectX::SimpleMath::Vector3 & GetLookAt(){return LookAt;}

protected:
	/** Cameras view matrix */
	DirectX::SimpleMath::Matrix View;

	/** Cameras position */
	DirectX::SimpleMath::Vector3 WorldPosition;

	/** Cameras Look-At position */
	DirectX::SimpleMath::Vector3 LookAt;
};

