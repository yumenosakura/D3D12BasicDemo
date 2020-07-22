#pragma once

#ifndef __D3D12_BASIC_TRIANGLE_H__
#define __D3D12_BASIC_TRIANGLE_H__

#include <wrl/client.h>
#include <d3d12.h>
#include <dxgi1_4.h>

class CD3D12BasicTriangle
{
public:
	CD3D12BasicTriangle(HWND hWnd, UINT width, UINT height);

	void InitializePipeline();
	void InitializeAssets();

private:
	HWND m_hWnd;
	UINT m_width;
	UINT m_height;
	UINT m_frameIndex;
	static const UINT m_frameCount = 2;

private:
	Microsoft::WRL::ComPtr<ID3D12Device> m_device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_commandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_renderTargetViewHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_renderTargets[m_frameCount];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_commandAllocator;
};

#endif // !__D3D12_BASIC_TRIANGLE_H__


