#include "Snake.h"
#include "ConstBuffer.h"

#include <d3dx12.h>
#include <d3dcompiler.h>
#include <WICTextureLoader.h>
#include <ResourceUploadBatch.h>
#include <DirectXHelpers.h>

using namespace DirectX;



Sample02::Sample02(UINT rows, UINT cols, UINT snakeMaxLength) : m_checkboardRows(rows), m_checkboardCols(cols), m_snakeMaxLength(snakeMaxLength){}

void Sample02::recordCmdList(ID3D12GraphicsCommandList* list, UINT ixFrame)
{
	m_cmdList->SetGraphicsRootSignature(m_rootSig.Get());

	ID3D12DescriptorHeap* descriptor_heaps[] = { m_cbvSrvUavHeap.Get() };
	m_cmdList->SetDescriptorHeaps(_countof(descriptor_heaps), descriptor_heaps);	

	auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), ixFrame, m_rtvHeapIncrementSize);
	auto heapHandle = m_cbvSrvUavHeap->GetGPUDescriptorHandleForHeapStart();

	auto viewport = CD3DX12_VIEWPORT((float)(m_rect.right - c_BoardSideLength)/2, (float)(m_rect.bottom - c_BoardSideLength)/2, 
		(float) c_BoardSideLength, 
		(float) c_BoardSideLength
	);

	m_cmdList->SetGraphicsRootDescriptorTable(RootSig::SceneConstBuffer, CD3DX12_GPU_DESCRIPTOR_HANDLE(heapHandle, 
		CbvSrvUAvHeap::SceneConstBuffer, m_cbvSrvUavHeapIncrementSize));
	m_cmdList->SetGraphicsRootDescriptorTable(RootSig::SnakeSheet, CD3DX12_GPU_DESCRIPTOR_HANDLE(heapHandle,
		CbvSrvUAvHeap::SnakeSheet, m_cbvSrvUavHeapIncrementSize));

	m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_cmdList->RSSetScissorRects(1, &m_rect);
	m_cmdList->RSSetViewports(1, &viewport); 
	m_cmdList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	auto pre_transition = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[ixFrame].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_cmdList->ResourceBarrier(1, &pre_transition);

	m_cmdList->ClearRenderTargetView(rtvHandle, c_ClearColor, 0, nullptr);
	m_cmdList->DrawInstanced(6, m_checkboardCols * m_checkboardRows, 0, 0);

	// Draw Snake and Fruit

	m_cmdList->SetPipelineState(m_snakePipelineState.Get());

	D3D12_VERTEX_BUFFER_VIEW buffer_views[] = {
		{m_snakeBuffer->GetGPUVirtualAddress(), m_snakeLength+1 * sizeof SnakePart, sizeof SnakePart},
	};
	m_cmdList->IASetVertexBuffers(0, _countof(buffer_views), buffer_views);
	m_cmdList->DrawInstanced(6, 1, 0, 0);
	
	auto post_transition = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[ixFrame].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_cmdList->ResourceBarrier(1, &post_transition);
}

std::filesystem::path Sample02::getBasePath()
{
	WCHAR name[MAX_PATH + 1];
	GetModuleFileName(NULL, name, MAX_PATH);
	return std::filesystem::path(name).parent_path();
}

void Sample02::loadAssets()
{
	HRESULT hr;

	CD3DX12_DESCRIPTOR_RANGE cbv_descriptor;
	cbv_descriptor.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1u, 0u);

	CD3DX12_DESCRIPTOR_RANGE sheet_range;
	sheet_range.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER parameters[RootSig::Count] = {};
	parameters[RootSig::SceneConstBuffer].InitAsDescriptorTable(1, &cbv_descriptor);
	parameters[RootSig::SnakeSheet].InitAsDescriptorTable(1, &sheet_range, D3D12_SHADER_VISIBILITY_PIXEL);

	auto sampler_desc = CD3DX12_STATIC_SAMPLER_DESC(0, D3D12_FILTER_MIN_MAG_MIP_POINT);

	auto root_sig_desc = CD3DX12_ROOT_SIGNATURE_DESC(D3D12_DEFAULT);
	root_sig_desc.Init(_countof(parameters), parameters, 1, &sampler_desc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature, error;
	D3D12SerializeRootSignature(&root_sig_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSig));

	ComPtr<ID3DBlob> vertex_shader, pixel_shader;
	if (FAILED(hr = D3DReadFileToBlob((getBasePath() / L"CheckboardVS.cso").c_str(), &vertex_shader)))
		__debugbreak();
	if (FAILED(hr = D3DReadFileToBlob((getBasePath() / L"CheckboardPS.cso").c_str(), &pixel_shader)))
		__debugbreak();

	const D3D12_GRAPHICS_PIPELINE_STATE_DESC checkboard_pipeline_desc = {
		.pRootSignature = m_rootSig.Get(),
		.VS = CD3DX12_SHADER_BYTECODE(vertex_shader.Get()),
		.PS = CD3DX12_SHADER_BYTECODE(pixel_shader.Get()),
		.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		.SampleMask = UINT_MAX,
		.RasterizerState = D3D12_RASTERIZER_DESC{.FillMode = D3D12_FILL_MODE_SOLID, .CullMode = D3D12_CULL_MODE_NONE},
		.DepthStencilState = D3D12_DEPTH_STENCIL_DESC{.DepthEnable = false, .StencilEnable = false},
		.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		.NumRenderTargets = 1,
		.RTVFormats = {c_BackBufferFormat},
		.SampleDesc = {.Count = 1},
	};
	m_device->CreateGraphicsPipelineState(&checkboard_pipeline_desc, IID_PPV_ARGS(&m_checkboardPipelineState));

	if (FAILED(hr = D3DReadFileToBlob((getBasePath() / L"SnakeVS.cso").c_str(), &vertex_shader)))
		__debugbreak();
	if (FAILED(hr = D3DReadFileToBlob((getBasePath() / L"SnakePS.cso").c_str(), &pixel_shader)))
		__debugbreak();

	// See SnakeVS
	const D3D12_INPUT_ELEMENT_DESC input_element_descs[] = {
		{"SNAKE_POS",	0, DXGI_FORMAT_R32G32_UINT,	 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
		{"SHEET_OFFSET",0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},

	};
	const D3D12_BLEND_DESC blend_desc = {
		.RenderTarget = {
			{
				.BlendEnable = true,
				.SrcBlend = D3D12_BLEND_SRC_ALPHA,
				.DestBlend = D3D12_BLEND_INV_SRC_ALPHA,
				.BlendOp = D3D12_BLEND_OP_ADD,

				.SrcBlendAlpha = D3D12_BLEND_ONE,
				.DestBlendAlpha = D3D12_BLEND_ZERO,
				.BlendOpAlpha = D3D12_BLEND_OP_ADD,

				
				.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL,
			}
		},
	};
	const D3D12_GRAPHICS_PIPELINE_STATE_DESC snake_pipeline_desc = {
		.pRootSignature = m_rootSig.Get(),
		.VS = CD3DX12_SHADER_BYTECODE(vertex_shader.Get()),
		.PS = CD3DX12_SHADER_BYTECODE(pixel_shader.Get()),
		.BlendState = blend_desc,
		//.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		.SampleMask = UINT_MAX,
		.RasterizerState = D3D12_RASTERIZER_DESC{.FillMode = D3D12_FILL_MODE_SOLID, .CullMode = D3D12_CULL_MODE_BACK, .FrontCounterClockwise = true},
		.DepthStencilState = D3D12_DEPTH_STENCIL_DESC{.DepthEnable = false, .StencilEnable = false},
		.InputLayout = D3D12_INPUT_LAYOUT_DESC{input_element_descs, _countof(input_element_descs)},
		.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		.NumRenderTargets = 1,
		.RTVFormats = {c_BackBufferFormat},
		.SampleDesc = {.Count = 1},
	};
	m_device->CreateGraphicsPipelineState(&snake_pipeline_desc, IID_PPV_ARGS(&m_snakePipelineState));

	m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, 
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof hlsl::SceneConstBuffer),
		D3D12_RESOURCE_STATE_COMMON,
		NULL, IID_PPV_ARGS(&m_sceneConstBuffer)	
	);
	m_sceneConstBuffer->SetName(L"SceneConstBuffer");

	m_device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(m_snakeMaxLength + 1), D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_snakeBuffer)
	);

	hlsl::SceneConstBuffer* pSceneData;
	m_sceneConstBuffer->Map(0, &CD3DX12_RANGE(), reinterpret_cast<void**>(&pSceneData));
	{
		pSceneData->rows_cols = XMUINT2{ m_checkboardRows, m_checkboardCols };
	}
	m_sceneConstBuffer->Unmap(0, &CD3DX12_RANGE());

	auto batch = DirectX::ResourceUploadBatch(m_device.Get());
	
	batch.Begin();

	if (FAILED(hr = CreateWICTextureFromFile(m_device.Get(), batch, 
		(getBasePath() / L"Assets" / L"SnakeSheet.png").c_str(), 
		&m_snakeSheet)))
		__debugbreak();
	m_snakeSheet->SetName(L"SnakeSheet");

	std::vector<SnakePart> snake_parts = {
		// apple in bottom left
		SnakePart{.snake_coords = {0,0}, .sheet_offset = {0, 3 / 4.f} },
	};
	void* pData;
	m_snakeBuffer->Map(0, &CD3DX12_RANGE(), &pData);
	{
		memcpy(pData, snake_parts.data(), snake_parts.size() * sizeof snake_parts[0]);
	}
	m_snakeBuffer->Unmap(0, &CD3DX12_RANGE());	
	m_snakeLength = 0;

	batch.End(m_queue.Get());

	const D3D12_CONSTANT_BUFFER_VIEW_DESC scene_const_buf_desc = {
		.BufferLocation = m_sceneConstBuffer->GetGPUVirtualAddress(),
		.SizeInBytes = sizeof hlsl::SceneConstBuffer,
	};
	m_device->CreateConstantBufferView(&scene_const_buf_desc,
		CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(), CbvSrvUAvHeap::SceneConstBuffer, m_cbvSrvUavHeapIncrementSize)
	);

	DirectX::CreateShaderResourceView(m_device.Get(), m_snakeSheet.Get(), 
		CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvSrvUavHeap->GetCPUDescriptorHandleForHeapStart(), CbvSrvUAvHeap::SnakeSheet, m_cbvSrvUavHeapIncrementSize));
}

void Sample02::waitForGPU()
{
	// Schedule a Signal command in the queue.
	m_queue->Signal(m_fence.Get(), m_fenceValue);

	// Wait until the fence has been processed.
	m_fence->SetEventOnCompletion(m_fenceValue, m_hFenceEvent);
	WaitForSingleObjectEx(m_hFenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_fenceValue++;
}

void Sample02::OnInit()
{
	HRESULT hr;

	// Enabling Debug Layer allows us to see errors as they appear
#ifdef _DEBUG
	ComPtr<ID3D12Debug6> p_debug;
	if (SUCCEEDED(hr = D3D12GetDebugInterface(IID_PPV_ARGS(p_debug.GetAddressOf()))))
		p_debug->EnableDebugLayer();
#endif // _DEBUG

	if (FAILED(hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(m_device.ReleaseAndGetAddressOf()))))
	{
		__debugbreak();
		return;
	}

	// Cmd Queue
	const D3D12_COMMAND_QUEUE_DESC queue_desc = {
		.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
		.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
	};
	m_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(m_queue.ReleaseAndGetAddressOf()));

	// Descriptor Heaps
	const D3D12_DESCRIPTOR_HEAP_DESC rtv_desc = {
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = c_FrameCount,
	};
	m_device->CreateDescriptorHeap(&rtv_desc, IID_PPV_ARGS(m_rtvHeap.ReleaseAndGetAddressOf()));
	m_rtvHeapIncrementSize = m_device->GetDescriptorHandleIncrementSize(rtv_desc.Type);
	
	const D3D12_DESCRIPTOR_HEAP_DESC cbv_srv_uav_desc = {
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		.NumDescriptors = CbvSrvUAvHeap::Count,
		.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
	};
	m_device->CreateDescriptorHeap(&cbv_srv_uav_desc, IID_PPV_ARGS(m_cbvSrvUavHeap.ReleaseAndGetAddressOf()));
	m_cbvSrvUavHeapIncrementSize = m_device->GetDescriptorHandleIncrementSize(cbv_srv_uav_desc.Type);

	// Cmd Allocator
	m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_allocator.ReleaseAndGetAddressOf()));

	// Gfx Cmd List
	m_device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(m_cmdList.ReleaseAndGetAddressOf()));

	m_fenceValue = 0;
	m_hFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf()));

	loadAssets();

	waitForGPU();
}

void Sample02::OnDestroy()
{
	waitForGPU();
	CloseHandle(m_hFenceEvent);
}

void Sample02::OnSurfaceLoaded(HWND hWnd, UINT width, UINT height)
{
	if (m_device.Get() == NULL)
		return;

	ComPtr<IDXGIFactory2> factory;
#ifdef _DEBUG
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(factory.GetAddressOf()));
#else
	CreateDXGIFactory2(0, IID_PPV_ARGS(factory.GetAddressOf()));
#endif // _DEBUG

	//
	// Create Swapchain
	// NOTE: DXGI was introduced with D3D10; D3D12 requires the SwapChain3 and above, 
	// thus the newer interface has to be queried.
	//
	const DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {
		.Width = width,
		.Height = height,
		.Format = c_BackBufferFormat,
		.SampleDesc = {.Count = 1},
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		.BufferCount = c_FrameCount,
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
	};
	ComPtr<IDXGISwapChain1> dummy_swapchain;
	factory->CreateSwapChainForHwnd(m_queue.Get(), hWnd, &swap_chain_desc, NULL, NULL, dummy_swapchain.GetAddressOf());
	dummy_swapchain->QueryInterface(m_swapChain.ReleaseAndGetAddressOf());

	// Get the buffers and create RTVs for each of them.
	auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT ix = 0; ix < c_FrameCount; ix++)
	{
		m_swapChain->GetBuffer(ix, IID_PPV_ARGS(m_renderTargets[ix].ReleaseAndGetAddressOf()));
		m_device->CreateRenderTargetView(m_renderTargets[ix].Get(), NULL, rtvHandle);
		rtvHandle.Offset(1, m_rtvHeapIncrementSize);
	}
	m_ixCurrentFrame = m_swapChain->GetCurrentBackBufferIndex();
	m_rect = RECT{ 0, 0, (LONG)width, (LONG)height};
}

void Sample02::OnRender()
{
	if (m_swapChain.Get() == NULL)
		return;

	m_allocator->Reset();

	m_cmdList->Reset(m_allocator.Get(), m_checkboardPipelineState.Get());

	recordCmdList(m_cmdList.Get(), m_ixCurrentFrame);

	m_cmdList->Close();

	ID3D12CommandList* command_lists[] = { m_cmdList.Get() };
	m_queue->ExecuteCommandLists(_countof(command_lists), command_lists);

	waitForGPU();

	m_swapChain->Present(1, 0);
	m_ixCurrentFrame = m_swapChain->GetCurrentBackBufferIndex();
}