#pragma once

#include "Arduino.h"

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