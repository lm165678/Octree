#include <iostream>
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

}

void output_data(const OctreeData<Obj>* data, string indent)
{
	cout << indent << "[ ";

	while (nullptr != data)
	{
		cout << data->object->ID << ' ';

		data = data->next;
	}

	cout << ']' << endl;
}

void output_octree(const OctreeNode<Obj>* node, string indent)
{
	if (nullptr == node)
		return;

	if (node->IsLeaf())
	{
		output_data(node->objects, indent);
	}
	else
	{
		cout << indent << '{' << endl;

		output_data(node->objects, indent + "    ");

		for (size_t i = 0; i < 8; i++)
		{
			cout << indent << "    " << i << endl;
			output_octree(node->GetChild(i), indent + "    ");
		}

		cout << indent << '}' << endl;
	}
}

int main()
{

	Octree<Obj, 3> octree(vec3{ 0.0f, 0.0f, 0.0f }, 4.0f);

	Obj obj{ 'A', AABB(vec3{0.1f, 0.1f, 0.1f},vec3{0.9f, 0.9f, 0.9f}) };

	octree.Insert(&obj);

	auto r = octree.GetRoot();

	output_octree(r, "");

	system("Pause");

	return 0;
}
