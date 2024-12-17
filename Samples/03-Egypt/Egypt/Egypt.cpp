#include "Egypt.h"
#include <d3dcompiler.h>

using std::filesystem::path;



std::filesystem::path Egypt::getBasePath()
{
	TCHAR name[MAX_PATH];
	GetModuleFileName(NULL, name, _countof(name));
	return path(name).parent_path();
}

void Egypt::recordCommandList(ID3D12GraphicsCommandList* cmdlist)
{
	cmdlist->SetGraphicsRootSignature(m_rootSig.Get());

	auto viewport = CD3DX12_VIEWPORT(0.f, 0.f, (FLOAT)m_rect.right, (FLOAT)m_rect.bottom);
	auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_ixCurrentFrame, m_rtvHeapIncrementSize);

	cmdlist->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	cmdlist->RSSetScissorRects(1, &m_rect);
	cmdlist->RSSetViewports(1, &viewport);
	cmdlist->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	// Back Buffer must be in Render Target resource State in order to be drawn onto.
	auto pre_transition = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_ixCurrentFrame].Get(),
		D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	cmdlist->ResourceBarrier(1, &pre_transition);

	cmdlist->ClearRenderTargetView(rtvHandle, c_ClearColor, 0, nullptr);
	cmdlist->DrawInstanced(3, 1, 0, 0);

	// Back Buffer must be in Present Resource state in order to be presented.
	auto post_transition = CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_ixCurrentFrame].Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
	cmdlist->ResourceBarrier(1, &post_transition);
}

void Egypt::waitForGPU()
{
	// Schedule a Signal command in the queue.
	m_queue->Signal(m_fence.Get(), m_fenceValue);

	// Wait until the fence has been processed.
	m_fence->SetEventOnCompletion(m_fenceValue, m_hFenceEvent);
	WaitForSingleObjectEx(m_hFenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_fenceValue++;
}

void Egypt::OnInit()
{
	HRESULT hr;

	// Enabling Debug Layer allows us to see errors as they appear

#ifdef _DEBUG
	ComPtr<ID3D12Debug6> p_debug;
	if (SUCCEEDED(hr = D3D12GetDebugInterface(IID_PPV_ARGS(p_debug.GetAddressOf()))))
		p_debug->EnableDebugLayer();
#endif // _DEBUG

	// TODO: find adapter. This is not needed for simpler games.
	if (FAILED(hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(m_device.ReleaseAndGetAddressOf()))))
	{
		__debugbreak();
		return;
	}

	const D3D12_COMMAND_QUEUE_DESC queue_desc = {
		.Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
		.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
	};
	m_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(m_queue.ReleaseAndGetAddressOf()));

	const D3D12_DESCRIPTOR_HEAP_DESC rtv_desc = {
		.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		.NumDescriptors = c_FrameCount,
	};
	m_device->CreateDescriptorHeap(&rtv_desc, IID_PPV_ARGS(m_rtvHeap.ReleaseAndGetAddressOf()));
	m_rtvHeapIncrementSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_allocator.ReleaseAndGetAddressOf()));

	m_device->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(m_cmdList.ReleaseAndGetAddressOf()));

	// Create Empty Root Signature
	CD3DX12_ROOT_SIGNATURE_DESC signature_desc(D3D12_DEFAULT);
	ComPtr<ID3DBlob> signature, error;

	D3D12SerializeRootSignature(&signature_desc, D3D_ROOT_SIGNATURE_VERSION_1_0, signature.GetAddressOf(), error.GetAddressOf());
	m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(m_rootSig.ReleaseAndGetAddressOf()));

	// Create Pipeline
	ComPtr<ID3DBlob> vertex_shader, pixel_shader;
	if (FAILED(hr = D3DReadFileToBlob((getBasePath() / "VertexShader.cso").c_str(), vertex_shader.GetAddressOf())))
		__debugbreak();
	if (FAILED(hr = D3DReadFileToBlob((getBasePath() / "PixelShader.cso").c_str(), pixel_shader.GetAddressOf())))
		__debugbreak();
	const D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeline_desc = {
		.pRootSignature = m_rootSig.Get(),
		.VS = CD3DX12_SHADER_BYTECODE(vertex_shader.Get()),
		.PS = CD3DX12_SHADER_BYTECODE(pixel_shader.Get()),
		.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT),
		.SampleMask = UINT_MAX,
		.RasterizerState = D3D12_RASTERIZER_DESC{.FillMode = D3D12_FILL_MODE_SOLID,.CullMode = D3D12_CULL_MODE_NONE, },
		.DepthStencilState = D3D12_DEPTH_STENCIL_DESC{.DepthEnable = false, .StencilEnable = false},
		.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
		.NumRenderTargets = 1,
		.RTVFormats = {c_BackBufferFormat},
		.SampleDesc = {.Count = 1},
	};
	m_device->CreateGraphicsPipelineState(&pipeline_desc, IID_PPV_ARGS(m_pipeline.ReleaseAndGetAddressOf()));

	m_fenceValue = 0;
	m_hFenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf()));

	waitForGPU();
}

void Egypt::OnDestroy()
{
	waitForGPU();

	CloseHandle(m_hFenceEvent);
}

void Egypt::OnSurfaceLoaded(HWND hWnd, UINT width, UINT height)
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
}

void Egypt::OnResize(UINT width, UINT height)
{
	for (auto rt : m_renderTargets)
		rt.Reset();

	// Get the buffers and create RTVs for each of them.
	auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT ix = 0; ix < c_FrameCount; ix++)
	{
		m_swapChain->GetBuffer(ix, IID_PPV_ARGS(m_renderTargets[ix].ReleaseAndGetAddressOf()));
		m_device->CreateRenderTargetView(m_renderTargets[ix].Get(), NULL, rtvHandle);
		rtvHandle.Offset(1, m_rtvHeapIncrementSize);
	}

	m_ixCurrentFrame = m_swapChain->GetCurrentBackBufferIndex();
	m_rect = RECT{ 0,0, (LONG)width, (LONG)height };

}

void Egypt::OnSuspend()
{
	OutputDebugStringA("Suspending!");
}

void Egypt::OnResume()
{
	OutputDebugStringA("Resuming!");
}

void Egypt::OnRender()
{
	// Reset Allocator
	m_allocator->Reset();

	// Reset Command List 
	m_cmdList->Reset(m_allocator.Get(), m_pipeline.Get());

	recordCommandList(m_cmdList.Get());

	m_cmdList->Close();

	ID3D12CommandList* command_lists[] = { m_cmdList.Get() };
	m_queue->ExecuteCommandLists(_countof(command_lists), command_lists);

	waitForGPU();

	m_swapChain->Present(1, 0);
	m_ixCurrentFrame = m_swapChain->GetCurrentBackBufferIndex();
}