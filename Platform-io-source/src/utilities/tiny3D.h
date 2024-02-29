#pragma once

#include "Arduino.h"
#include <stdint.h>


class FixedSinCos
{
	public:

	static const int32_t PRECISION_BITS = 16; // Adjust for desired precision

	static int32_t fixed_cos(int32_t angle_shifted)
	{
		int32_t x = 1 << (PRECISION_BITS - 1); // Initial approximation
		int32_t y = 0;
		const int32_t cordic_gain = 0xC90FDAA; // Pre-calculated constant

		// Limit angle to first quadrant (0 to 90 degrees)
		if (angle_shifted > (1 << (PRECISION_BITS - 2))) {
			angle_shifted -= (1 << (PRECISION_BITS - 2));
			// Swap x and y for symmetry
			int32_t temp = x;
			x = -y;
			y = temp;
		}

		for (int i = 0; i < PRECISION_BITS; i++) {
			if (y < 0) {
				int32_t temp = x;
				x += (y >> i) * cordic_gain; // Rotate vector
				y -= (temp >> i) * cordic_gain;
			} else {
				int32_t temp = x;
				x -= (y >> i) * cordic_gain; 
				y += (temp >> i) * cordic_gain;
			}
		}
		return x; 
	}

	static int32_t fixed_sin(int32_t angle_shifted)
	{
		// Convert angle to cosine using identity sin(x + 90) = cos(x)
		angle_shifted += (1 << (PRECISION_BITS - 2));
		return fixed_cos(angle_shifted);
	}
};

class Polygon
{
	public:
		uint16_t index[3];
		uint8_t color;

		Polygon()
		{
		}

		Polygon(uint16_t index_0, uint16_t index_1, uint16_t index_2)
		{
			index[0] = index_0;
			index[1] = index_1;
			index[2] = index_2;
		}

		Polygon(uint16_t index_0, uint16_t index_1, uint16_t index_2, uint8_t index_colour)
		{
			index[0] = index_0;
			index[1] = index_1;
			index[2] = index_2;
			color = index_colour;
		}
};

class Vector3D
{
	public:
		float x;
		float y;
		float z;

		Vector3D()
		{
		}

		Vector3D(float ax, float ay, float az)
		{
			x = ax;
			y = ay;
			z = az;
		}
		
		void Normalize()
		{
			float scale = sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z));
			this->x /= scale;
			this->y /= scale;
			this->z /= scale;
		}

		void CrossProduct(Vector3D* v)
		{
			float newX = (this->y * v->z) - (this->z * v->y);
			float newY = (this->z * v->x) - (this->x * v->z);
			float newZ = (this->x * v->y) - (this->y * v->x);
			this->x = newX;
			this->y = newY;
			this->z = newZ;
		}
		
		void operator*=(Vector3D *v)
		{
			this->x *= v->x;
			this->y *= v->y;
			this->z *= v->z;
		}
		void operator*=(float scale)
		{
			this->x *= scale;
			this->y *= scale;
			this->z *= scale;
		}
		void operator/=(float scale)
		{
			this->x /= scale;
			this->y /= scale;
			this->z /= scale;
		}
		void operator+=(Vector3D v)
		{
			this->x += v.x;
			this->y += v.y;
			this->z += v.z;
		}
		void operator-=(Vector3D v)
		{
			this->x -= v.x;
			this->y -= v.y;
			this->z -= v.z;
		}
		void operator=(Vector3D v)
		{
			this->x = v.x;
			this->y = v.y;
			this->z = v.z;
		}
		
		Vector3D operator*(float scale) const
		{
			Vector3D newVector;

			newVector.x = x * scale;
			newVector.y = y * scale;
			newVector.z = z * scale;

			return newVector;
		}
		Vector3D operator*(Vector3D v) const
		{
			Vector3D newVector;

			newVector.x = x * v.x;
			newVector.y = y * v.y;
			newVector.z = z * v.z;

			return newVector;
		}
		Vector3D operator+(Vector3D v) const
		{
			Vector3D newVector;

			newVector.x = x + v.x;
			newVector.y = y + v.y;
			newVector.z = z + v.z;

			return newVector;
		}
		Vector3D operator-(Vector3D v) const
		{
			Vector3D newVector;

			newVector.x = x - v.x;
			newVector.y = y - v.y;
			newVector.z = z - v.z;

			return newVector;
		}
};

class RotationMatrix
{
	public:
		Vector3D row1;
		Vector3D row2;
		Vector3D row3;

		RotationMatrix()
		{
		}

		void updateRotationMatrix(float angle_x, float angle_y, float angle_z)
		{
			float rSin[3] = { sin( angle_x * (float)DEG_TO_RAD ), sin( angle_y * (float)DEG_TO_RAD ), sin( angle_z * (float)DEG_TO_RAD ) };
			float rCos[3] = { cos( angle_x * (float)DEG_TO_RAD ), cos( angle_y * (float)DEG_TO_RAD ), cos( angle_z * (float)DEG_TO_RAD ) };
		
			row1.x = rCos[1] *  rCos[2];
			row1.y = rCos[1] * -rSin[2];
			row1.z = rSin[1];

			row2.x = -rSin[0] * -rSin[1] *  rCos[2] + rCos[0] * rSin[2];
			row2.y = -rSin[0] * -rSin[1] * -rSin[2] + rCos[0] * rCos[2];
			row2.z = -rSin[0] *  rCos[1];

			row3.x = rCos[0] * -rSin[1] *  rCos[2] + rSin[0] * rSin[2];
			row3.y = rCos[0] * -rSin[1] * -rSin[2] + rSin[0] * rCos[2];
			row3.z = rCos[0] *  rCos[1];	
		}

		Vector3D operator*( Vector3D v ) const
		{
			return Vector3D(				
				this->row1.x * v.x + this->row1.y * v.y + this->row1.z * v.z,
				this->row2.x * v.x + this->row2.y * v.y + this->row2.z * v.z,
				this->row3.x * v.x + this->row3.y * v.y + this->row3.z * v.z
			);
		}
};