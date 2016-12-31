#include "Renderer.h"
#include "NativeWindow.h"

#include <fstream>
#include <d3d11_1.h>
#include <DirectXMath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace
{
	constexpr size_t initial_vertex_buffer_size = sizeof(float) * 6 * 8 * 512;
	constexpr size_t initial_index_buffer_size = sizeof(Renderer::index_t) * 2 * 12 * 512;

	struct Buffer
	{
		size_t		size;
		char*		data;

		Buffer() : size(0), data(nullptr) {}
		Buffer(size_t size) : size(size), data(nullptr) { if (size > 0) data = new char[size]; }

		Buffer(const Buffer&) = delete;
		Buffer(Buffer&& other) : size(other.size), data(other.data) { other.size = 0; other.data = nullptr; }

		~Buffer() { if (nullptr != data) delete[] data; }

		operator void* () const { return data; }
		operator bool() const { return nullptr != data; }
	};

	Buffer LoadShaderFromFile(const char* filename)
	{
		std::ifstream fin(filename, std::ios::in | std::ios::binary | std::ios::ate);

		if (!fin.is_open())
			return Buffer();

		size_t length = fin.tellg();

		Buffer buffer(length);

		fin.seekg(0, std::ios::beg);
		fin.read(buffer.data, length);
		fin.close();

		return buffer;
	}
}

Renderer::Renderer(NativeWindow* win)
	:
	mInited(false),
	mDevice(nullptr),
	mContext(nullptr),
	mSwapChain(nullptr),
	mRTV(nullptr),
	mDepthStencilTex(nullptr),
	mDSV(nullptr),
	mVertexShader(nullptr),
	mPixelShader(nullptr),
	mInputLayout(nullptr),
	mCurrentBuffer(0),
	mVertexBufferSize(initial_vertex_buffer_size),
	mIndexBufferSize(initial_index_buffer_size),
	mNextVertexBufferSize(initial_vertex_buffer_size),
	mNextIndexBufferSize(initial_index_buffer_size),
	mStride(sizeof(float) * 6),
	mConstantBufferProj(nullptr),
	mConstantBufferView(nullptr),
	mConstantBufferModel(nullptr),
	mEyePosition(vec3{ 0.0f, 0.0f, -1.0f }),
	mLookDirection(vec3{ 0.0f, 0.0f, 1.0f })
{
	HRESULT hr = S_OK;

	{
		D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* context = nullptr;

		if (S_OK != D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevels, 2, D3D11_SDK_VERSION, &device, nullptr, &context))
		{
			return;
		}

		if (S_OK != device->QueryInterface(__uuidof(ID3D11Device1), (void**)&(this->mDevice)))
		{
			context->Release();
			device->Release();
			return;
		}

		context->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&(this->mContext));

		device->Release();
		context->Release();
	}

	IDXGIFactory2* factory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		if (S_OK == mDevice->QueryInterface<IDXGIDevice>(&dxgiDevice))
		{
			IDXGIAdapter* adapter = nullptr;
			if (S_OK == dxgiDevice->GetAdapter(&adapter))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory2), (void**)&factory);
				adapter->Release();
			}
			dxgiDevice->Release();
		}
		if (S_OK != hr)
		{
			mContext->Release();
			mDevice->Release();
			return;
		}
	}

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{ 0 };

	swapChainDesc.Width = win->width;
	swapChainDesc.Height = win->height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;//DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDesc.Flags = 0;// DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HWND hwnd = *reinterpret_cast<HWND*>(win->data);
	if (S_OK != (hr = factory->CreateSwapChainForHwnd(mDevice, hwnd, &swapChainDesc, nullptr, nullptr, &(mSwapChain))))
	{
		mContext->Release();
		mDevice->Release();
		factory->Release();
		return;
	}

	factory->Release();

	{
		ID3D11Texture2D* backbufferTex = nullptr;
		if (S_OK != (hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbufferTex)))
		{
			return;
		}
		if (S_OK != (hr = mDevice->CreateRenderTargetView(backbufferTex, nullptr, &mRTV)))
		{
			return;
		}

		D3D11_TEXTURE2D_DESC depthDesc{ 0 };
		depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthDesc.Width = win->width;
		depthDesc.Height = win->height;
		depthDesc.ArraySize = 1;
		depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthDesc.MipLevels = 1;
		depthDesc.SampleDesc.Count = 1;
		depthDesc.SampleDesc.Quality = 0;
		if (S_OK != (hr = mDevice->CreateTexture2D(&depthDesc, nullptr, &mDepthStencilTex)))
		{
			return;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = 0;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;

		if (S_OK != (hr = mDevice->CreateDepthStencilView(mDepthStencilTex, &dsvDesc, &mDSV)))
		{
			return;
		}

		mContext->OMSetRenderTargets(1, &mRTV, mDSV);

		backbufferTex->Release();
	}

	{
		D3D11_VIEWPORT viewport{ 0.0f, 0.0f, static_cast<float>(win->width), static_cast<float>(win->height), 0.0f, 1.0f };
		mContext->RSSetViewports(1, &viewport);
	}

	{
		D3D11_BUFFER_DESC vb_desc{ 0 };
		vb_desc.Usage = D3D11_USAGE_DYNAMIC;
		vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vb_desc.ByteWidth = static_cast<UINT>(mVertexBufferSize);
		vb_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_BUFFER_DESC ib_desc{ 0 };
		ib_desc.Usage = D3D11_USAGE_DYNAMIC;
		ib_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ib_desc.ByteWidth = static_cast<UINT>(mIndexBufferSize);
		ib_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		for (size_t i = 0; i < buffer_count; i++)
		{
			if (S_OK != (hr = mDevice->CreateBuffer(&vb_desc, nullptr, &mVertexBuffers[i])))
			{
				mContext->Release();
				mDevice->Release();
				factory->Release();
				return;
			}

			if (S_OK != (hr = mDevice->CreateBuffer(&ib_desc, nullptr, &mIndexBuffers[i])))
			{
				mContext->Release();
				mDevice->Release();
				factory->Release();
				return;
			}

			mVertexBufferUsed[i] = 0;
			mIndexBufferUsed[i] = 0;
		}
	}

	{
		Buffer vsBuffer = LoadShaderFromFile("VertexShader.cso");
		Buffer psBuffer = LoadShaderFromFile("PixelShader.cso");

		if (!vsBuffer || !psBuffer)
		{
			return;
		}

		if (S_OK != (hr = mDevice->CreateVertexShader(vsBuffer.data, vsBuffer.size, nullptr, &mVertexShader)))
		{
			return;
		}

		if (S_OK != (hr = mDevice->CreatePixelShader(psBuffer.data, psBuffer.size, nullptr, &mPixelShader)))
		{
			return;
		}

		mContext->VSSetShader(mVertexShader, nullptr, 0);
		mContext->PSSetShader(mPixelShader, nullptr, 0);


		D3D11_INPUT_ELEMENT_DESC input_layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		if (S_OK != (hr = mDevice->CreateInputLayout(input_layout, 2, vsBuffer.data, vsBuffer.size, &mInputLayout)))
		{
			return;
		}
		mContext->IASetInputLayout(mInputLayout);
	}

	{
		D3D11_BUFFER_DESC desc{ 0 };
		desc.ByteWidth = sizeof(float) * 16;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		if (S_OK != (hr = mDevice->CreateBuffer(&desc, nullptr, &mConstantBufferProj)))
		{
			return;
		}
		if (S_OK != (hr = mDevice->CreateBuffer(&desc, nullptr, &mConstantBufferView)))
		{
			return;
		}
		if (S_OK != (hr = mDevice->CreateBuffer(&desc, nullptr, &mConstantBufferModel)))
		{
			return;
		}

		auto matProj = DirectX::XMMatrixTranspose(
			DirectX::XMMatrixPerspectiveFovLH(3.1415926f * 0.5f, static_cast<float>(win->width) / static_cast<float>(win->height), 0.01f, 100.0f)
		);
		auto matView = DirectX::XMMatrixTranspose(
			DirectX::XMMatrixLookToLH(
				DirectX::XMVectorSet(mEyePosition.x, mEyePosition.y, mEyePosition.z, 0.0f),
				DirectX::XMVectorSet(mLookDirection.x, mLookDirection.y, mLookDirection.z, 0.0f),
				DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
			)
		);
		auto matModel = DirectX::XMMatrixIdentity();

		mContext->UpdateSubresource(mConstantBufferProj, 0, nullptr, &matProj, 0, 0);
		mContext->UpdateSubresource(mConstantBufferView, 0, nullptr, &matView, 0, 0);
		mContext->UpdateSubresource(mConstantBufferModel, 0, nullptr, &matModel, 0, 0);
	}

	MapBuffers(mCurrentBuffer);

	mInited = true;
}

Renderer::~Renderer()
{
	if (mInited)
	{
		for (size_t i = 0; i < buffer_count; i++)
		{
			mVertexBuffers[i]->Release();
			mIndexBuffers[i]->Release();
		}

		mRTV->Release();
		mSwapChain->Release();
		mContext->Release();
		mDevice->Release();
	}
}

void Renderer::AddLine(const vec3& start, const vec3& end, const vec3& col)
{
	float* p = reinterpret_cast<float*>(reinterpret_cast<char*>(mVertexBufferPointer) + mVertexBufferUsed[mCurrentBuffer]);

	index_t idx = static_cast<index_t>(mVertexBufferUsed[mCurrentBuffer] / mStride);

	*(p++) = start.x;
	*(p++) = start.y;
	*(p++) = start.z;
	*(p++) = col.x;
	*(p++) = col.y;
	*(p++) = col.z;

	*(p++) = end.x;
	*(p++) = end.y;
	*(p++) = end.z;
	*(p++) = col.x;
	*(p++) = col.y;
	*(p++) = col.z;

	mVertexBufferUsed[mCurrentBuffer] += mStride * 2;

	index_t* q = reinterpret_cast<index_t*>(reinterpret_cast<char*>(mIndexBufferPointer) + mIndexBufferUsed[mCurrentBuffer]);

	*(q++) = idx;
	*(q++) = idx + 1;

	mIndexBufferUsed[mCurrentBuffer] += sizeof(index_t) * 2;
}

void Renderer::AddBox(const AABB& box, const vec3& col)
{
	float* p = reinterpret_cast<float*>(reinterpret_cast<char*>(mVertexBufferPointer) + mVertexBufferUsed[mCurrentBuffer]);

	index_t idx = static_cast<index_t>(mVertexBufferUsed[mCurrentBuffer] / mStride);

	*(p++) = box.min.x;
	*(p++) = box.min.y;
	*(p++) = box.min.z;
	*(p++) = col.x;
	*(p++) = col.y;
	*(p++) = col.z;

	*(p++) = box.max.x;
	*(p++) = box.min.y;
	*(p++) = box.min.z;
	*(p++) = col.x;
	*(p++) = col.y;
	*(p++) = col.z;

	*(p++) = box.min.x;
	*(p++) = box.min.y;
	*(p++) = box.max.z;
	*(p++) = col.x;
	*(p++) = col.y;
	*(p++) = col.z;

	*(p++) = box.max.x;
	*(p++) = box.min.y;
	*(p++) = box.max.z;
	*(p++) = col.x;
	*(p++) = col.y;
	*(p++) = col.z;

	*(p++) = box.min.x;
	*(p++) = box.max.y;
	*(p++) = box.min.z;
	*(p++) = col.x;
	*(p++) = col.y;
	*(p++) = col.z;

	*(p++) = box.max.x;
	*(p++) = box.max.y;
	*(p++) = box.min.z;
	*(p++) = col.x;
	*(p++) = col.y;
	*(p++) = col.z;

	*(p++) = box.min.x;
	*(p++) = box.max.y;
	*(p++) = box.max.z;
	*(p++) = col.x;
	*(p++) = col.y;
	*(p++) = col.z;

	*(p++) = box.max.x;
	*(p++) = box.max.y;
	*(p++) = box.max.z;
	*(p++) = col.x;
	*(p++) = col.y;
	*(p++) = col.z;

	mVertexBufferUsed[mCurrentBuffer] += mStride * 8;

	index_t* q = reinterpret_cast<index_t*>(reinterpret_cast<char*>(mIndexBufferPointer) + mIndexBufferUsed[mCurrentBuffer]);

	*(q++) = idx;
	*(q++) = idx + 1;

	*(q++) = idx;
	*(q++) = idx + 2;

	*(q++) = idx + 1;
	*(q++) = idx + 3;

	*(q++) = idx + 2;
	*(q++) = idx + 3;

	*(q++) = idx;
	*(q++) = idx + 4;

	*(q++) = idx + 1;
	*(q++) = idx + 5;

	*(q++) = idx + 2;
	*(q++) = idx + 6;

	*(q++) = idx + 3;
	*(q++) = idx + 7;

	*(q++) = idx + 4;
	*(q++) = idx + 5;

	*(q++) = idx + 4;
	*(q++) = idx + 6;

	*(q++) = idx + 5;
	*(q++) = idx + 7;

	*(q++) = idx + 6;
	*(q++) = idx + 7;

	mIndexBufferUsed[mCurrentBuffer] += sizeof(index_t) * 2 * 12;
}

void Renderer::Clear()
{
	float clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	mContext->ClearRenderTargetView(mRTV, clear_color);
	mContext->ClearDepthStencilView(mDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderer::Render()
{
	UnmapBuffers(mCurrentBuffer);
	size_t drawIdx = (mCurrentBuffer == 0) ? (buffer_count - 1) : (mCurrentBuffer - 1);

	mCurrentBuffer = (mCurrentBuffer + 1) % buffer_count;
	MapBuffers(mCurrentBuffer);

	if (mIndexBufferUsed[drawIdx] > 0)
	{
		UINT offset = 0;

		auto matView = DirectX::XMMatrixTranspose(
			DirectX::XMMatrixLookToLH(
				DirectX::XMVectorSet(mEyePosition.x, mEyePosition.y, mEyePosition.z, 0.0f),
				DirectX::XMVector3Normalize(DirectX::XMVectorSet(mLookDirection.x, mLookDirection.y, mLookDirection.z, 0.0f)),
				DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
			)
		);

		mContext->UpdateSubresource(mConstantBufferView, 0, nullptr, &matView, 0, 0);

		mContext->VSSetConstantBuffers(0, 1, &mConstantBufferProj);
		mContext->VSSetConstantBuffers(1, 1, &mConstantBufferView);
		mContext->VSSetConstantBuffers(2, 1, &mConstantBufferModel);

		mContext->IASetVertexBuffers(0, 1, &mVertexBuffers[drawIdx], &mStride, &offset);
		mContext->IASetIndexBuffer(mIndexBuffers[drawIdx], DXGI_FORMAT_R16_UINT, 0);

		mContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		mContext->DrawIndexed(static_cast<UINT>(mIndexBufferUsed[drawIdx] / sizeof(index_t)), 0, 0);
	}

}

void Renderer::Present()
{
	mSwapChain->Present(1, 0);
}

void Renderer::SetCameraLookTo(const vec3& eye, const vec3& dir)
{
	mEyePosition = eye;
	mLookDirection = dir;
}

void Renderer::MapBuffers(size_t idx)
{
	D3D11_MAPPED_SUBRESOURCE mapped_resource;

	mContext->Map(mVertexBuffers[idx], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	mVertexBufferPointer = reinterpret_cast<float*>(mapped_resource.pData);

	mContext->Map(mIndexBuffers[idx], 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource);
	mIndexBufferPointer = reinterpret_cast<index_t*>(mapped_resource.pData);

	mVertexBufferUsed[idx] = 0;
	mIndexBufferUsed[idx] = 0;
}

void Renderer::UnmapBuffers(size_t idx)
{
	mContext->Unmap(mVertexBuffers[idx], 0);
	mContext->Unmap(mIndexBuffers[idx], 0);
}

