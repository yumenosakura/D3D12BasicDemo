#pragma once

#ifndef __D3D12_BASIC_TRIANGLE_H__
#define __D3D12_BASIC_TRIANGLE_H__

#include <wrl/client.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>

class CD3D12BasicTriangle
{
public:
	CD3D12BasicTriangle(HWND hWnd, UINT width, UINT height);

	void Initialize();
	void OnRender();

private:
	HWND m_hWnd;
	UINT m_width;
	UINT m_height;
	UINT m_frameIndex;
	static const UINT m_frameCount = 2;
	UINT m_renderTargetViewHeapDescSize;
	UINT64 m_fenceValue;

	Microsoft::WRL::ComPtr<ID3D12Device> m_device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_renderTargetViewHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[m_frameCount];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pipelineState;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_commandList;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	HANDLE m_fenceEvent;
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;

	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

	void InitializePipeline();
	void InitializeAssets();
	void WaitForPreviousFrame();
	void PopulateCommandList();
};

#endif // !__D3D12_BASIC_TRIANGLE_H__


