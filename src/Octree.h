#pragma once

#include "Vector3.h"
#include "AABB.h"

template<T>
struct OctreeData
{
	OctreeData*		next;
	const T*		object;

	//
	inline const AABB& AABB() const { return object->GetAABB(); }
};

template<T>
struct OctreeNode
{
	OctreeNode**	children;
	OctreeNode*		parent;
	Vector3			center;
	float			halfSize;
	OctreeData<T>*	objects;
};

template<T>
class Octree
{
public:

	Octree(Vector3 center, float halfSize)
		: root(nullptr)
	{

	}

	void Insert(const T* object)
	{
		const AABB& aabb = object->GetAABB();

	}

private:
	OctreeNode<T>*		root;
};
