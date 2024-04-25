#pragma once
#include "resource.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <filesystem>

class Sample02 
{
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	static constexpr UINT c_FrameCount = 2u;
	static constexpr DXGI_FORMAT c_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static inline const float c_ClearColor[4] = { .2f, 0.2f, .8f, 1.f };

	static constexpr UINT c_BoardSideLength = 400; //Px


	struct CbvSrvUAvHeap {
		enum {
			SceneConstBuffer,
			SnakeSheet,

			Count,
		};
	};
	struct RootSig {
		enum {
			SceneConstBuffer,
			SnakeSheet,

			Count,
		};
	};

	struct Check {
		DirectX::XMFLOAT2 position;
	};

	ComPtr<ID3D12Device4> m_device;
	ComPtr<ID3D12CommandQueue> m_queue;
	ComPtr<ID3D12CommandAllocator> m_allocator;
	ComPtr<ID3D12GraphicsCommandList> m_cmdList;

	ComPtr<ID3D12RootSignature> m_rootSig;
	ComPtr<ID3D12PipelineState> m_checkboardPipelineState;

	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_cbvSrvUavHeap;
	UINT m_rtvHeapIncrementSize;
	UINT m_cbvSrvUavHeapIncrementSize;

	ComPtr<ID3D12Resource> m_renderTargets[c_FrameCount];
	ComPtr<IDXGISwapChain4> m_swapChain;
	RECT m_rect;
	UINT m_ixCurrentFrame;

	HANDLE m_hFenceEvent;
	UINT64 m_fenceValue;
	ComPtr<ID3D12Fence> m_fence;

	UINT m_checkboardRows, m_checkboardCols, m_snakeMaxLength;
	ComPtr<ID3D12Resource> m_sceneConstBuffer;
	ComPtr<ID3D12Resource> m_snakeSheet;

	void recordCmdList(ID3D12GraphicsCommandList* list, UINT ixFrame);

	std::filesystem::path getBasePath();

	void loadAssets();

	void waitForGPU();

public:

	Sample02(UINT rows, UINT cols, UINT snakeMaxLength);

	void OnInit();

	void OnDestroy();

	void OnSurfaceLoaded(HWND, UINT width, UINT height);

	void OnRender();
};