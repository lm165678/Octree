#pragma once

template<typename T>
struct Vector3
{
	T x, y, z;

	template<typename N>
	inline operator Vector3<N>() const { return Vector3<N>{static_cast<N>(x), static_cast<N>(y), static_cast<N>(z)}; }
};

template<typename T>
inline Vector3<T> operator + (const Vector3<T>& a, const Vector3<T>& b)
{
	return Vector3<T>{ a.x + b.x, a.y + b.y, a.z + b.z };
}

template<typename T>
inline Vector3<T> operator - (const Vector3<T>& a, const Vector3<T>& b)
{
	return Vector3<T>{ a.x - b.x, a.y - b.y, a.z - b.z };
}

template<typename T>
inline Vector3<T> operator * (const Vector3<T>& a, const Vector3<T>& b)
{
	return Vector3<T>{ a.x * b.x, a.y * b.y, a.z * b.z };
}

template<typename T>
inline Vector3<T> operator * (const Vector3<T>& a, T b)
{
	return Vector3<T>{ a.x * b, a.y * b, a.z * b };
}

template<typename T>
inline Vector3<T> operator * (T a, const Vector3<T>& b)
{
	return Vector3<T>{ a * b.x, a * b.y, a * b.z };
}

template<typename T>
inline Vector3<T> operator / (const Vector3<T>& a, T b)
{
	return Vector3<T>{ a.x / b, a.y / b, a.z / b };
}

template<typename T>
inline T dot(const Vector3<T>& a, const Vector3<T>& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

template<typename T>
inline Vector3<T> cross(const Vector3<T>& a, const Vector3<T>& b)
{
	return Vector3<T>{ a.y * b.z - a.z *b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

typedef Vector3<float>	vec3;
typedef Vector3<int>	ivec3;
