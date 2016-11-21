#pragma once

#include "Vector3.h"
#include "AABB.h"

#pragma pack(push, 4)
struct NodeBoundingBox
{
	vec3	center;
	float	halfSize;

	inline operator AABB() const { return AABB(center, halfSize); }
};
#pragma pack(pop)

template<typename T>
struct OctreeData
{
	OctreeData*		next;
	T*				object;

	//
	inline const AABB& AABB() const { return object->GetAABB(); }
};

template<typename T>
struct OctreeNode
{
	OctreeNode**	children;
	OctreeNode*		parent;
	NodeBoundingBox	bound;
	OctreeData<T>*	objects;

	OctreeNode(const vec3& center, float halfSize)
		:
		children(nullptr),
		parent(nullptr),
		center(center),
		halfSize(halfSize),
		objects(nullptr)
	{

	}

	~OctreeNode()
	{
		if (nullptr != children)
		{
			// TODO clear children
			children = nullptr;
		}

		if (nullptr != objects)
		{
			// TODO clear object nodes
			objects = nullptr;
		}
	}

	inline const NodeBoundingBox& GetBound() const { return bound; }

	inline bool IsLeaf() const { return nullptr == children; }

	inline NodeBoundingBox GetChildBound(size_t idx) const
	{
		auto child = GetChild(idx);
		if (nullptr != child)
			return child->GetBound();

		vec3 d{ (idx & 1) - 0.5f, ((idx >> 1) & 1) - 0.5f, ((idx >> 2 & 1)) - 0.5f };
		vec3 c = center + d * halfSize;
		return NodeBoundingBox{ c, halfSize * 0.5f };
	}

	inline OctreeNode<T>* GetChild(size_t idx) const
	{
		if (IsLeaf())
			return nullptr;
		else
			return children[idx];
	}

	inline void SetChild(size_t idx, OctreeNode<T>* node)
	{
		if (nullptr == children)
			children = new (OctreeNode*)[8]{ nullptr };

		children[idx] = node;
	}

	inline void Insert(T* object)
	{

	}
};

template<typename T, int MAX_DEPTH>
class Octree
{
public:

	typedef OctreeNode<T> Node;

	inline Octree(vec3 center, float halfSize)
		: root(new Node(center, halfSize)) { }

	inline void Insert(T* object) { Insert(root, object, MAX_DEPTH); }

private:

	inline void Insert(Node* node, T* object, int depth)
	{
		const NodeBoundingBox& nbox = node->GetBound();
		const AABB& obox = object->GetAABB();

		if (nbox.Contains(obox))
		{
			if (depth > 0)
			{
				float halfSize = node->halfSize * 0.5f;
				for (size_t i = 0; i < 8; i++)
				{
					NodeBoundingBox childBound = node->GetChildBound(i);
					if (AABB(childBound).Contains(obox))
					{
						Node* child = node->GetChild(i);
						if (nullptr == child)
						{
							child = new Node(childBound.center, childBound.halfSize);
							child->parent = node;
							node->SetChild(i, child);
						}
						Insert(child, object, depth - 1);
						return;
					}
				}
			}

			node->Insert(object);
		}
		else
		{
			// TODO expand tree
		}

	}

private:
	Node*		root;
};
