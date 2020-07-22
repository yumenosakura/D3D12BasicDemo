#include "D3D12BasicTriangle.h"

#include <d3d12.h>
#include <wrl/client.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>

CD3D12BasicTriangle::CD3D12BasicTriangle(HWND hWnd, UINT width, UINT height)
	: m_hWnd(hWnd)
	, m_width(width)
	, m_height(height)
{
}

/*
* create d3d12 device
* create the command queue
* create the swap chain
* crate a render target view descriptor heap
* create frame resources (a render target view for each frame)
* create a command allocator
*/
void CD3D12BasicTriangle::InitializePipeline()
{
	// 1. Create the device.
	Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
	Microsoft::WRL::ComPtr<IDXGIFactory4> factory4;
	Microsoft::WRL::ComPtr<IDXGIFactory6> factory6;

	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
	{
		throw "Create factory1 failed.";
	}
	else if (FAILED(factory4->QueryInterface(IID_PPV_ARGS(&factory6))))
	{
		throw "Query Interface failed.";
	}
	else if (DXGI_ERROR_NOT_FOUND == factory6->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(&hardwareAdapter)))
	{
		throw "Not found hardware adapter.";
	}
	else if (FAILED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), &m_device)))
	{
		throw "Create device failed.";
	}

	// 2. Create the command queue.
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	if (FAILED(m_device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&m_commandQueue))))
	{
		throw "Create command queue failed.";
	}

	// 3. Create swap chain.
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferCount = m_frameCount;
	swapChainDesc.BufferDesc.Width = m_width;
	swapChainDesc.BufferDesc.Height = m_height;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = m_hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = TRUE;

	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	if (FAILED(factory4->CreateSwapChain(m_commandQueue.Get(), &swapChainDesc, &swapChain)))
	{
		throw "Create swap chain failed.";
	}
	else if (FAILED((swapChain.As(&m_swapChain))))
	{
		throw "Swap chain as 3 is failed.";
	}
	else if (FAILED(factory4->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER)))
	{
		throw "Make window association failed.";
	}
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// 4. Create a render target view (RTV) descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC renderTargetViewHeapDesc;
	renderTargetViewHeapDesc.NumDescriptors = m_frameCount;
	renderTargetViewHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	renderTargetViewHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	if (FAILED(m_device->CreateDescriptorHeap(&renderTargetViewHeapDesc, IID_PPV_ARGS(&m_renderTargetViewHeap))))
	{
		throw "Create a render target view descriptor failed.";
	}

	// 5. Create frame resource (a render target view for each frame)
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT n = 0; n < m_frameCount; ++n)
	{
		if (FAILED(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]))))
		{
			throw "Get frame resource failed.";
		}
		m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, renderTargetViewHandle);
		//renderTargetViewHandle.
	}

	// 6. Create a command allocator.
	if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator))))
	{
		throw "Create a command allocator failed.";
	}
}

void CD3D12BasicTriangle::InitializeAssets()
{

}
