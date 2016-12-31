#include <iostream>
#include <iomanip>
#include <string>
#include "Octree.h"

using namespace std;

struct Obj
{
	char	ID;
	AABB	aabb;
	const AABB& GetAABB() const { return aabb; }
};

void output_aabb(const AABB& aabb)
{
	cout << "( " << setw(6) << aabb.min.x << ',' << setw(6) << aabb.min.y << ',' << setw(6) << aabb.max.x << ',' << setw(6) << aabb.max.y << " )";
}

void output_data(const OctreeData<Obj>* data)
{
	cout << "[ ";

	while (nullptr != data)
	{
		cout << data->object->ID << ' ';

		data = data->next;
	}

	cout << ']';
}

void output_octree(const OctreeNode<Obj>* node, string indent)
{
	if (nullptr == node)
		return;

	if (node->IsLeaf())
	{
		output_data(node->objects);
	}
	else
	{
		cout << endl << indent << '{' << endl;

		if (nullptr != node->objects)
		{
			cout << indent << "    "; output_data(node->objects);  cout << endl;
		}

		for (size_t i = 0; i < 8; i++)
		{
			cout << indent << "    " << i << ' ';
			output_aabb(node->GetChildBound(i));
			output_octree(node->GetChild(i), indent + "    ");
			cout << endl;
		}

		cout << indent << '}';
	}
}

int main()
{

	Octree<Obj, 3> octree(vec3{ 0.0f, 0.0f, 0.0f }, 4.0f);

	Obj obj{ 'A', AABB(vec3{0.1f, 0.1f, 0.1f},vec3{0.9f, 0.9f, 0.9f}) };

	Obj obj2{ 'B', AABB(vec3{ -3.0f, -3.0f, -3.0f },vec3{ -0.1f, -0.1f, -0.1f }) };

	octree.Insert(&obj);
	octree.Insert(&obj2);

	auto r = octree.GetRoot();

	output_octree(r, "");

	system("Pause");

	return 0;
}
