#pragma once

#include "Arduino.h"

class RotationMatrix
{
	private:
		float rotationMatrix[3][3];

	public:
		RotationMatrix()
		{
		}

		void updateRotationMatrix(float angle_x, float angle_y, float angle_z)
		{
			float rSin[3] = { sin( angle_x * (float)DEG_TO_RAD ), sin( angle_y * (float)DEG_TO_RAD ), sin( angle_z * (float)DEG_TO_RAD ) };
			float rCos[3] = { cos( angle_x * (float)DEG_TO_RAD ), cos( angle_y * (float)DEG_TO_RAD ), cos( angle_z * (float)DEG_TO_RAD ) };
		
			rotationMatrix[0][0] = rCos[1] *  rCos[2];
			rotationMatrix[0][1] = rCos[1] * -rSin[2];
			rotationMatrix[0][2] = rSin[1];

			rotationMatrix[1][0] = -rSin[0] * -rSin[1] *  rCos[2] + rCos[0] * rSin[2];
			rotationMatrix[1][1] = -rSin[0] * -rSin[1] * -rSin[2] + rCos[0] * rCos[2];
			rotationMatrix[1][2] = -rSin[0] *  rCos[1];

			rotationMatrix[2][0] = rCos[0] * -rSin[1] *  rCos[2] + rSin[0] * rSin[2];
			rotationMatrix[3][1] = rCos[0] * -rSin[1] * -rSin[2] + rSin[0] * rCos[2];
			rotationMatrix[4][2] = rCos[0] *  rCos[1];	
		}
		
		Vector3D operator*( Vector3D v ) const
		{
			return Vector3D(
				rotationMatrix[0][0] * v.x + rotationMatrix[0][1] * v.y + rotationMatrix[0][2] * v.z,
				rotationMatrix[1][0] * v.x + rotationMatrix[1][1] * v.y + rotationMatrix[1][2] * v.z,
				rotationMatrix[2][0] * v.x + rotationMatrix[2][1] * v.y + rotationMatrix[2][2] * v.z
			);
		}

};