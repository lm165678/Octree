#pragma once

class NativeWindow
{
public:

	NativeWindow(const wchar_t* title, int w, int h);
	~NativeWindow();

	void Show();

	bool ProcessEvent();

	inline operator bool() const { return nullptr != data; }

private:
	friend class Renderer;
	struct Context;
	Context* data;

	int width, height;
};
