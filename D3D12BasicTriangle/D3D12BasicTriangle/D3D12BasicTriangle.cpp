#include "D3D12BasicTriangle.h"

#include <d3d12.h>
#include <wrl/client.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <string>

CD3D12BasicTriangle::CD3D12BasicTriangle(HWND hWnd, UINT width, UINT height)
	: m_hWnd(hWnd)
	, m_width(width)
	, m_height(height)
{
}

void CD3D12BasicTriangle::Initialize()
{
	InitializePipeline();
	InitializeAssets();
}

void CD3D12BasicTriangle::OnRender()
{

}

//---------------------------------------------------------------------
// 1. Create the device
// 2. Create the command queue
// 3. Create the swap chain
// 4. Crate a render target view descriptor heap
// 5. Create frame resources (a render target view for each frame)
// 6. Create a command allocator
//---------------------------------------------------------------------
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
	m_renderTargetViewHeapDescSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// 5. Create frame resource (a render target view for each frame)
	D3D12_CPU_DESCRIPTOR_HANDLE renderTargetViewHandle = m_renderTargetViewHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT n = 0; n < m_frameCount; ++n)
	{
		if (FAILED(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n]))))
		{
			throw "Get frame resource failed.";
		}
		m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, renderTargetViewHandle);
		renderTargetViewHandle.ptr = SIZE_T(INT64(renderTargetViewHandle.ptr) + INT64(1) * INT64(m_renderTargetViewHeapDescSize));
	}

	// 6. Create a command allocator.
	if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator))))
	{
		throw "Create a command allocator failed.";
	}
}

//----------------------------------------------------------------------------
// Flow for intialize assets.
// 1. Create an empty root signature.
// 2. Complie the shaders.
// 3. Create the vertex input layout.
// 4. Create a pipeline state object descripton, then create the object.
// 5. Create the command list.
// 6. Create and load the vertex buffers.
// 7. Create the vertex buffer views.
// 8. Create a fence.
// 9. Create an event handle.
// 10. Wait for the GPU to finish.
//----------------------------------------------------------------------------
void CD3D12BasicTriangle::InitializeAssets()
{
	// 1. Create an empty root signature.
	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.NumParameters = 0;
	rootSignatureDesc.pParameters = nullptr;
	rootSignatureDesc.NumStaticSamplers = 0;
	rootSignatureDesc.pStaticSamplers = nullptr;
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;

	if (FAILED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error)))
	{
		throw "Serialize root signature failed.";
	}
	else if (FAILED(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature))))
	{
		throw "Create root signature failed.";
	}

	// 2. Complie the shaders.
	Microsoft::WRL::ComPtr<ID3DBlob> pVertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pPixelShader;

	std::wstring shaderFilePath = L"./shaders.hlsl";
	if (FAILED(D3DCompileFromFile(shaderFilePath.c_str(), nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &pVertexShader, nullptr)))
	{
		throw "Complie the vertex shader failed.";
	}
	else if (FAILED(D3DCompileFromFile(shaderFilePath.c_str(), nullptr, nullptr, "PSMain", "vs_5_0", 0, 0, &pPixelShader, nullptr)))
	{
		throw "Complie the pixel shader failed.";
	}

	// 3. Create the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// 4. Create a pipeline state object descripton, then create the object.
	D3D12_SHADER_BYTECODE vertexShader;
	vertexShader.pShaderBytecode = pVertexShader.Get()->GetBufferPointer();
	vertexShader.BytecodeLength = pVertexShader.Get()->GetBufferSize();

	D3D12_SHADER_BYTECODE pixelShader;
	pixelShader.pShaderBytecode = pPixelShader.Get()->GetBufferPointer();
	pixelShader.BytecodeLength = pPixelShader.Get()->GetBufferSize();

	D3D12_RASTERIZER_DESC rasterizerDesc;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc;
	renderTargetBlendDesc.BlendEnable = FALSE;
	renderTargetBlendDesc.LogicOpEnable = FALSE;
	renderTargetBlendDesc.SrcBlend = D3D12_BLEND_ONE;
	renderTargetBlendDesc.DestBlend = D3D12_BLEND_ZERO;
	renderTargetBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
	renderTargetBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	renderTargetBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
	renderTargetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_BLEND_DESC blendDesc;
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	for (UINT n = 0; n < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++n)
	{
		blendDesc.RenderTarget[n] = renderTargetBlendDesc;
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc;
	graphicsPipelineStateDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	graphicsPipelineStateDesc.pRootSignature = m_rootSignature.Get();
	graphicsPipelineStateDesc.VS = vertexShader;
	graphicsPipelineStateDesc.PS = pixelShader;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
	graphicsPipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
	graphicsPipelineStateDesc.SampleMask = UINT_MAX;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;

	if (FAILED(m_device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&m_pipelineState))))
	{
		throw "Create graphics pipeline state failed.";
	}

	// 5. Create the command list.
	if (FAILED(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList))))
	{
		throw "Create command line failed.";
	}

	// 6. Create and load the vertex buffers.
	Vertex triangleVertices[3];
	triangleVertices[0].position = DirectX::XMFLOAT3(0.0f, 0.25f, 0.0f);
	triangleVertices[0].color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	triangleVertices[1].position = DirectX::XMFLOAT3(0.25f, -0.25f, 0.0f);
	triangleVertices[1].color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	triangleVertices[2].position = DirectX::XMFLOAT3(-0.25f, -0.25f, 0.0f);
	triangleVertices[2].color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	const UINT vertexBufferSize = sizeof(triangleVertices);

	D3D12_HEAP_PROPERTIES heapProperties;
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 1;
	heapProperties.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC vertexBufferDesc;
	vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexBufferDesc.Alignment = 0;
	vertexBufferDesc.Width = vertexBufferSize;
	vertexBufferDesc.Height = 1;
	vertexBufferDesc.DepthOrArraySize = 1;
	vertexBufferDesc.MipLevels = 1;
	vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	vertexBufferDesc.SampleDesc.Count = 1;
	vertexBufferDesc.SampleDesc.Quality = 0;
	vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	vertexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	if (FAILED(m_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &vertexBufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer))))
	{
		throw "Create committed resource falied.";
	}

	// 7. Create the vertex buffer views.
	UINT8* pVertexDataBegin;
	D3D12_RANGE readRange;
	readRange.Begin = 0;
	readRange.End = 0;

	if (FAILED(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin))))
	{
		throw "Map vertex buffer failed.";
	}
	memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
	m_vertexBuffer->Unmap(0, nullptr);

	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(Vertex);
	m_vertexBufferView.SizeInBytes = vertexBufferSize;

	// 8. Create a fence.
	if (FAILED(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))))
	{
		throw "Create fence failed";
	}

	// 9. Create an event handle.
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (nullptr == m_fenceEvent)
	{
		throw "Create fence event failed.";
	}

	// 10. Wait for the GPU to finish.
	WaitForPreviousFrame();
}

void CD3D12BasicTriangle::WaitForPreviousFrame()
{
}

//---------------------------------------------------------
// Flow for populate the command list.
// 1. Reset the command list allocator.
// 2. Reset the command list.
// 3. Set the graphics root signature.
// 4. Set the viewport and scissor rectangles.
// 5. Set a resource barrier, indicating the back buffer is to be used as a render target.
// 6. Record commands into the command list.
// 7. Indicate the back buffer will be used to present after the command list executed.
// 8. Close the command list to further recording.
//---------------------------------------------------------
void CD3D12BasicTriangle::PopulateCommandList()
{
	// 1. Reset the command list allocator.
	if (FAILED(m_commandAllocator->Reset()))
	{
		throw "Reset the command list allocator failed.";
	}
	// 2. Reset the command list.
	else if (FAILED(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get())))
	{
		throw "Reset the command list failed.";
	}

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);
}
