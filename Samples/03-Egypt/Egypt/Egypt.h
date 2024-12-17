#pragma once
#include "resource.h"


#include "framework.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dx12.h>
#include <filesystem>

class Egypt
{
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;
	

	static constexpr UINT c_FrameCount = 2;
	static constexpr DXGI_FORMAT c_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	static const inline float c_ClearColor[4] = { .2f, .2f, 0.8f, 1.f };

	ComPtr<ID3D12Device4> m_device;
	ComPtr<ID3D12CommandQueue> m_queue;
	ComPtr<ID3D12CommandAllocator> m_allocator;
	ComPtr<ID3D12GraphicsCommandList> m_cmdList;

	ComPtr<ID3D12RootSignature> m_rootSig;
	ComPtr<ID3D12PipelineState> m_pipeline;

	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	UINT m_rtvHeapIncrementSize;

	ComPtr<ID3D12Resource> m_renderTargets[c_FrameCount];
	ComPtr<IDXGISwapChain4> m_swapChain;
	RECT m_rect;
	UINT m_ixCurrentFrame;


	HANDLE m_hFenceEvent;
	UINT64 m_fenceValue;
	ComPtr<ID3D12Fence> m_fence;


	static std::filesystem::path getBasePath();

	void recordCommandList(ID3D12GraphicsCommandList* cmdlist);

	void waitForGPU();

public:

	void OnInit();

	void OnDestroy();

	void OnSurfaceLoaded(HWND hWnd, UINT width, UINT height);

	void OnResize(UINT width, UINT height);

	void OnSuspend();

	void OnResume();

	void OnRender();

};

