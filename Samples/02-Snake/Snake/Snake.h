#pragma once
#include "resource.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <filesystem>

#include <span>

struct snake_vector
{
	INT x, y;
	bool operator==(snake_vector& that)
	{
		return this->x == that.x && this->y == that.y;
	}
};

class Sample02 
{
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	static constexpr UINT c_FrameCount = 2u;
	static constexpr DXGI_FORMAT c_BackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	static inline const float c_ClearColor[4] = { .2f, 0.2f, .8f, 1.f };

	static constexpr UINT c_BoardSideLength = 400; //Px


	static constexpr DirectX::XMFLOAT2 
		c_AppleOffset = {0, 3/4.f},		// Apple
		c_SnakeBend1 = {0,0},			// Snake Bend left to down (alt. up to right)
		c_SnakeBend2 = {0, 1/4.f},		// Snake bend down to right (alt right left to up)
		c_SnakeHorizontal = {1/5.f, 0}, // Horizontal Snake (left to right vice versa)
		c_SnakeBend3 = {2/5.f, 0},		// Snake Bend right to down (alt. up to left)
		c_SnakeVertical = {2/5.f, 1/4.f},// Vertical Snake (up to down vice versa)
		c_SnakeBend4 = {2.f/5, 2.f/4},	// Snake bend right to up (alt. down to left)
		c_HeadUp   = {3/5.f, 0},		
		c_HeadRight= {4/5.f, 0},	
		c_HeadLeft = {3/5.f, 1/4.f},
		c_HeadDown = {4/5.f, 1/4.f},
		c_TailDown = {3/5.f, 2/4.f},
		c_TailLeft = {4/5.f, 2/4.f},
		c_TailRight= {3/5.f, 3/4.f},
		c_TailUp   = {4/5.f, 3/4.f};

	struct SnakePart {
		DirectX::XMUINT2 snake_coords;
		DirectX::XMFLOAT2 sheet_offset;
	};

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
	ComPtr<ID3D12PipelineState> m_snakePipelineState;

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
	UINT m_snakeLength;
	ComPtr<ID3D12Resource> m_sceneConstBuffer;
	ComPtr<ID3D12Resource> m_snakeSheet;
	ComPtr<ID3D12Resource> m_snakeBuffer;

	std::vector<SnakePart> m_entities;
	

	void recordCmdList(ID3D12GraphicsCommandList* list, UINT ixFrame);

	std::filesystem::path getBasePath();

	void loadAssets();

	void waitForGPU();

public:

	Sample02(UINT rows, UINT cols, UINT snakeMaxLength);

	void UpdateEntityPositions(std::span<snake_vector> snake, snake_vector apple);

	void OnInit();

	void OnDestroy();

	void OnSurfaceLoaded(HWND, UINT width, UINT height);

	void OnRender();
};