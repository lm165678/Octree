#pragma once

#include "Vector3.h"
#include "AABB.h"

class NativeWindow;

struct ID3D11Device1;
struct ID3D11DeviceContext1;
struct IDXGISwapChain1;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11Buffer;
struct ID3D11Texture2D;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11InputLayout;

constexpr size_t buffer_count = 3;

class Renderer
{
public:
	typedef unsigned short index_t;

private:
	bool						mInited;
	ID3D11Device1*				mDevice;
	ID3D11DeviceContext1*		mContext;
	IDXGISwapChain1*			mSwapChain;

	ID3D11RenderTargetView*		mRTV;

	ID3D11Texture2D*			mDepthStencilTex;
	ID3D11DepthStencilView*		mDSV;

	ID3D11VertexShader*			mVertexShader;
	ID3D11PixelShader*			mPixelShader;

	ID3D11InputLayout*			mInputLayout;

	ID3D11Buffer*				mVertexBuffers[buffer_count];
	ID3D11Buffer*				mIndexBuffers[buffer_count];
	size_t						mCurrentBuffer;

	size_t						mVertexBufferSize;
	size_t						mIndexBufferSize;
	size_t						mNextVertexBufferSize;
	size_t						mNextIndexBufferSize;

	float*						mVertexBufferPointer;
	index_t*					mIndexBufferPointer;

	size_t						mVertexBufferUsed[buffer_count];
	size_t						mIndexBufferUsed[buffer_count];

	unsigned int				mStride;

	ID3D11Buffer*				mConstantBufferProj;
	ID3D11Buffer*				mConstantBufferView;
	ID3D11Buffer*				mConstantBufferModel;

	vec3						mEyePosition;
	vec3						mLookDirection;

public:

	Renderer(NativeWindow* win);
	~Renderer();

	void AddLine(const vec3& start, const vec3& end, const vec3& col);
	void AddBox(const AABB& box, const vec3& col);

	void Clear();

	void Render();

	void Present();

	void SetCameraLookTo(const vec3& eye, const vec3& dir);

public:
	inline operator bool() const { return mInited; }

private:
	void MapBuffers(size_t idx);
	void UnmapBuffers(size_t idx);
};


