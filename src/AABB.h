#pragma once

#include "Vector3.h"

struct AABB
{
	vec3	min;
	vec3	max;

	inline AABB() : min(vec3()), max(vec3()) {}
	inline AABB(const vec3& min, const vec3& max) : min(min), max(max) {}
	inline AABB(const vec3& center, float halfSize)
		:
		min(vec3{ center.x - halfSize, center.y - halfSize , center.z - halfSize }),
		max(vec3{ center.x + halfSize, center.y + halfSize , center.z + halfSize }) {}

	inline bool Contains(const vec3& point) const
	{
		return
			min.x <= point.x && point.x <= max.x &&
			min.y <= point.y && point.y <= max.y &&
			min.z <= point.z && point.z <= max.z;
	}

	inline bool Contains(const AABB& other) const
	{
		return
			min.x <= other.min.x && other.max.x <= max.x &&
			min.y <= other.min.y && other.max.y <= max.y &&
			min.x <= other.min.z && other.max.z <= max.z;
	}
};
