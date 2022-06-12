#include "Pch.hpp"

#include "Device.hpp"
#include "Core/Application.hpp"

namespace helios::gfx
{
	Device::Device()
	{
		InitDeviceResources();
		InitSwapChainResources();
	}

	void Device::InitDeviceResources()
	{
		// Create DXGI Factory.
		UINT dxgiFactoryFlags = 0;
#ifdef _DEBUG
		dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

		ThrowIfFailed(::CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mFactory)));

		// Enable the debug layer.
#ifdef _DEBUG
		ThrowIfFailed(::D3D12GetDebugInterface(IID_PPV_ARGS(&mDebugInterface)));
		mDebugInterface->EnableDebugLayer();
		mDebugInterface->SetEnableGPUBasedValidation(TRUE);
		mDebugInterface->SetEnableSynchronizedCommandQueueValidation(TRUE);

		// Currently set to default behaviour.
		mDebugInterface->SetGPUBasedValidationFlags(D3D12_GPU_BASED_VALIDATION_FLAGS_NONE);
#endif

		// Select the adapter (in this case GPU with best performance).
		// Index 0 will be the GPU with highest preference.
		ThrowIfFailed(mFactory->EnumAdapterByGpuPreference(0u, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&mAdapter)));

#ifdef _DEBUG
		DXGI_ADAPTER_DESC adapterDesc{};
		ThrowIfFailed(mAdapter->GetDesc(&adapterDesc));
		std::wstring adapterInfo = L"\nAdapter Description : " + std::wstring(adapterDesc.Description) + L".\n";
		OutputDebugString(adapterInfo.c_str());
#endif

		// Create D3D12 device.
		ThrowIfFailed(::D3D12CreateDevice(mAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&mDevice)));
		mDevice->SetName(L"D3D12 Device");

		// Set break points on certain severity levels in debug mode.
#ifdef _DEBUG
		wrl::ComPtr<ID3D12InfoQueue> infoQueue;
		ThrowIfFailed(mDevice.As(&infoQueue));

		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		// Configure queue filter to ignore info message severity.
		std::array<D3D12_MESSAGE_SEVERITY, 1> ignoreMessageSeverities
		{
			D3D12_MESSAGE_SEVERITY_INFO
		};

		// Configure queue filter to ignore individual messages using thier ID.
		std::array<D3D12_MESSAGE_ID, 1> ignoreMessageIDs
		{
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE
		};

		D3D12_INFO_QUEUE_FILTER infoQueueFilter
		{
			.DenyList
			{
				.NumSeverities = static_cast<UINT>(ignoreMessageSeverities.size()),
				.pSeverityList = ignoreMessageSeverities.data(),
				.NumIDs = static_cast<UINT>(ignoreMessageIDs.size()),
				.pIDList = ignoreMessageIDs.data()
			},
		};

		ThrowIfFailed(infoQueue->PushStorageFilter(&infoQueueFilter));

		ThrowIfFailed(mDevice->QueryInterface(IID_PPV_ARGS(&mDebugDevice)));
#endif
	}

	void Device::InitSwapChainResources()
	{
		// Check if tearing is supported (needed to know if tearing is supported).
		BOOL allowTearing = TRUE;
		if (FAILED(mFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
		{
			allowTearing = FALSE;
		}

		// Create the swapchain.
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc
		{
			.Width = Application::GetClientWidth(),
			.Height = Application::GetClientHeight(),
			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
			.Stereo = FALSE,
			.SampleDesc
			{
				.Count = 1,
				.Quality = 0,
			},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = NUMBER_OF_FRAMES,
			.Scaling = DXGI_SCALING_STRETCH,
			.SwapEffect = mVSync ? DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL : DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags = allowTearing && !mVSync ? DXGI_PRESENT_ALLOW_TEARING : 0u
		};

		wrl::ComPtr<IDXGISwapChain1> swapChain1;
		ThrowIfFailed(mFactory->CreateSwapChainForHwnd(m_CommandQueue.GetCommandQueue().Get(), Application::GetWindowHandle(), &swapChainDesc, nullptr, nullptr, &swapChain1));

		// Prevent DXGI from switching to full screen state automatically while using ALT + ENTER combination.
		ThrowIfFailed(m_Factory->MakeWindowAssociation(Application::GetWindowHandle(), DXGI_MWA_NO_ALT_ENTER));

		ThrowIfFailed(swapChain1.As(&m_SwapChain));

		m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
	}
}