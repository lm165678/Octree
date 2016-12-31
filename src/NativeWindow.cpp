#include "NativeWindow.h"

#include <Windows.h>

namespace
{
	wchar_t g_WindowClassName[] = L"NativeWindow";

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}

		return 0;
	}
}

struct NativeWindow::Context
{
	HWND hWnd;
	HINSTANCE hInstance;
};

NativeWindow::NativeWindow(const wchar_t* title, int w, int h)
	:
	data(new Context{ 0 }),
	width(w), height(h)
{
	HINSTANCE hInstance = GetModuleHandle(nullptr);

	WNDCLASSEX cls{ 0 };
	cls.cbSize = sizeof(WNDCLASSEX);
	cls.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
	cls.hInstance = hInstance;
	cls.lpfnWndProc = WndProc;
	cls.lpszClassName = g_WindowClassName;
	cls.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&cls))
	{
		delete data;
		data = nullptr;
		return;
	}

	RECT rc = { 0, 0, w, h };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hWnd = CreateWindow(g_WindowClassName, title, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr/*reinterpret_cast<void*>(this)*/);

	if (!hWnd)
	{
		UnregisterClass(g_WindowClassName, hInstance);
		delete data;
		data = nullptr;
		return;
	}

	data->hInstance = hInstance;
	data->hWnd = hWnd;
}

NativeWindow::~NativeWindow()
{
	if (nullptr != data)
	{
		UnregisterClass(g_WindowClassName, data->hInstance);
		delete data;
		data = nullptr;
	}
}

void NativeWindow::Show()
{
	if (nullptr != data)
		ShowWindow(data->hWnd, SW_SHOW);
}

bool NativeWindow::ProcessEvent()
{
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return false;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return true;
}

