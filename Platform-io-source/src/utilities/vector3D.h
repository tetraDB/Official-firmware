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
		Vector3D* newVector = new Vector3D;

		newVector->x = (this->y * v->z) - (this->z * v->y);
		newVector->y = (this->z * v->x) - (this->x * v->z);
		newVector->z = (this->x * v->y) - (this->y * v->x);

		this->x = newVector->x;
		this->y = newVector->y;
		this->z = newVector->z;

		delete newVector;
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

	Vector3D operator+(Vector3D v) const
	{
		Vector3D newVector;

		newVector.x = x + v.x;
		newVector.y = y + v.y;
		newVector.z = z + v.z;

		return newVector;
	}

	void operator+=(Vector3D v)
	{
		this->x += v.x;
		this->y += v.y;
		this->z += v.z;
	}

	Vector3D operator-(Vector3D v) const
	{
		Vector3D newVector;

		newVector.x = x - v.x;
		newVector.y = y - v.y;
		newVector.z = z - v.z;

		return newVector;
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

// private:

};