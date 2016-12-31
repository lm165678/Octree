#include <Windows.h>

#include "NativeWindow.h"
#include "Renderer.h"
#include "Octree.h"

namespace
{
	float vertices[] = {
		0.0f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
	};

	struct Obj
	{
		char	ID;
		AABB	aabb;
		const AABB& GetAABB() const { return aabb; }
	};

	void render_objects(Renderer& renderer, const OctreeData<Obj>* data)
	{
		while (nullptr != data)
		{
			//cout << data->object->ID << ' ';
			renderer.AddBox(data->object->GetAABB(), vec3{ 1.0f, 0.0f, 0.0f });

			data = data->next;
		}
	}

	void render_octree(Renderer& renderer, const OctreeNode<Obj>* node)
	{
		if (nullptr == node)
			return;

		if (node->IsLeaf())
		{
			render_objects(renderer, node->objects);
		}
		else
		{
			if (nullptr != node->objects)
			{
				render_objects(renderer, node->objects);
			}

			for (size_t i = 0; i < 8; i++)
			{
				renderer.AddBox(node->GetChildBound(i), vec3{ 0.0f, 1.0f, 0.0f });
				render_octree(renderer, node->GetChild(i));
			}

		}
	}
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	NativeWindow win(L"Octree", 800, 600);
	Renderer renderer(&win);

	if (!win || !renderer)
		return -1;

	win.Show();

	Octree<Obj, 3> octree(vec3{ 0.0f, 0.0f, 0.0f }, 4.0f);

	Obj obj{ 'A', AABB(vec3{ 0.1f, 0.1f, 0.1f },vec3{ 0.9f, 0.9f, 0.9f }) };

	Obj obj2{ 'B', AABB(vec3{ -3.0f, -3.0f, -3.0f },vec3{ -0.1f, -0.1f, -0.1f }) };

	octree.Insert(&obj);
	octree.Insert(&obj2);

	renderer.SetCameraLookTo(vec3{ 5.0f, 5.0f, -5.0f }, vec3{ -1.0f, -1.0f, 1.0f });

	while (win.ProcessEvent())
	{
		renderer.Clear();

		render_octree(renderer, octree.GetRoot());

		renderer.Render();

		renderer.Present();
	}

	return 0;
}

