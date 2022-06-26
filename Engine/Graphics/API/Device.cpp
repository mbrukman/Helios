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

	Device::~Device()
	{
		mGraphicsCommandQueue->FlushQueue();
		mComputeCommandQueue->FlushQueue();
	}

	void Device::InitDeviceResources()
	{
		// Enable the debug layer.
#ifdef _DEBUG
		ThrowIfFailed(::D3D12GetDebugInterface(IID_PPV_ARGS(&mDebugInterface)));
		mDebugInterface->EnableDebugLayer();
		mDebugInterface->SetEnableGPUBasedValidation(TRUE);
		mDebugInterface->SetEnableSynchronizedCommandQueueValidation(TRUE);
#endif

		// Create DXGI Factory.
		UINT dxgiFactoryFlags{ 0 };
#ifdef _DEBUG
		dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

		ThrowIfFailed(::CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&mFactory)));

		// Select the adapter (in this case GPU with best performance).
		// Index 0 will be the GPU with highest preference.
		ThrowIfFailed(mFactory->EnumAdapterByGpuPreference(0u, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&mAdapter)));

		if (!mAdapter)
		{
			ErrorMessage(L"Could not get a D3D12 compatible adapter.");
		}

#ifdef _DEBUG
		// The adapter descriptor will be displayed as message (check infoQueue below).
		DXGI_ADAPTER_DESC adapterDesc{};
		ThrowIfFailed(mAdapter->GetDesc(&adapterDesc));
		std::string adapterInfo{ "\nAdapter Description : " + WstringToString(adapterDesc.Description) + ".\n" };
#endif

		// Create D3D12 device.
		ThrowIfFailed(::D3D12CreateDevice(mAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&mDevice)));
		mDevice->SetName(L"D3D12 Device");

		// Set break points on certain severity levels in debug mode.
#ifdef _DEBUG
		wrl::ComPtr<ID3D12InfoQueue> infoQueue;
		ThrowIfFailed(mDevice.As(&infoQueue));

		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

		infoQueue->AddApplicationMessage(D3D12_MESSAGE_SEVERITY_MESSAGE, adapterInfo.c_str());

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

		// Get the debug device.
		ThrowIfFailed(mDevice->QueryInterface(IID_PPV_ARGS(&mDebugDevice)));
#endif

		// Create memory allocator.
		mMemoryAllocator = std::make_unique<MemoryAllocator>(mDevice.Get(), mAdapter.Get());

		// Create the command queue's.
		mGraphicsCommandQueue = std::make_unique<CommandQueue>(mDevice.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT, L"Graphics Command Queue");
		mComputeCommandQueue = std::make_unique<CommandQueue>(mDevice.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE, L"Compute Command Queue");
		mCopyCommandQueue = std::make_unique<CommandQueue>(mDevice.Get(), D3D12_COMMAND_LIST_TYPE_COPY, L"Copy Command Queue");
		// Create the descriptor heaps.
		mRTVDescriptor = std::make_unique<Descriptor>(mDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 15u, L"RTV Descriptor");
		mDSVDescriptor = std::make_unique<Descriptor>(mDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_DSV, D3D12_DESCRIPTOR_HEAP_FLAG_NONE, 15u, L"DSV Descriptor");
		mSrvCbvUavDescriptor = std::make_unique<Descriptor>(mDevice.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE, 1020u, L"SRV_CBV_UAV Descriptor");

		// Create the depth stencil buffer
		mDepthStencilBuffer = std::make_unique<DepthStencilBuffer>(mDevice.Get(), mDSVDescriptor.get(), mSrvCbvUavDescriptor.get(), DXGI_FORMAT_R24G8_TYPELESS, Application::GetClientWidth(), Application::GetClientHeight(), L"Depth Stencil Buffer");
		
		// Create bindless root signature.
		PipelineState::CreateBindlessRootSignature(mDevice.Get(), L"Shaders/BindlessRS.cso");
	}

	void Device::InitSwapChainResources()
	{
		// Check if tearing is supported (needed to know if tearing should be done when vsync is off).
		BOOL allowTearing = FALSE;
		if (FAILED(mFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
		{
			allowTearing = FALSE;
		}

		mTearingSupported = allowTearing;

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
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags = mTearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u
		};

		wrl::ComPtr<IDXGISwapChain1> swapChain1;
		ThrowIfFailed(mFactory->CreateSwapChainForHwnd(mGraphicsCommandQueue->GetCommandQueue().Get(), Application::GetWindowHandle(), &swapChainDesc, nullptr, nullptr, &swapChain1));

		// Prevent DXGI from switching to full screen state automatically while using ALT + ENTER combination.
		ThrowIfFailed(mFactory->MakeWindowAssociation(Application::GetWindowHandle(), DXGI_MWA_NO_ALT_ENTER));

		ThrowIfFailed(swapChain1.As(&mSwapChain));

		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
		
		CreateBackBufferRTVs();
	}

	void Device::CreateBackBufferRTVs()
	{
		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
		
		// Create Backbuffer render target views.
		for (int i = 0; i < NUMBER_OF_FRAMES; ++i)
		{
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = mRTVDescriptor->GetCurrentDescriptorHandle().cpuDescriptorHandle;

			wrl::ComPtr<ID3D12Resource> backBuffer;
			ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));

			mDevice->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

			mBackBuffers[i].backBufferResource = backBuffer;
			mBackBuffers[i].backBufferResource->SetName(L"SwapChain BackBuffer");

			mBackBuffers[i].backBufferDescriptorHandle = mRTVDescriptor->GetCurrentDescriptorHandle();

			mRTVDescriptor->OffsetCurrentHandle();
		}
	}

	void Device::ResizeBuffers()
	{
		mGraphicsCommandQueue->FlushQueue();

		// Resize the swap chain's back buffer.
		for (int i = 0; i < NUMBER_OF_FRAMES; i++)
		{
			mBackBuffers[i].backBufferResource.Reset();
			mFrameFenceValues[i] = mFrameFenceValues[mCurrentBackBufferIndex];
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		ThrowIfFailed(mSwapChain->GetDesc(&swapChainDesc));
		ThrowIfFailed(mSwapChain->ResizeBuffers(NUMBER_OF_FRAMES, Application::GetClientWidth(), Application::GetClientHeight(), swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

		CreateBackBufferRTVs();

		// Resize the depth buffer.
		// note (rtarun9) : approach of recreating depth buffer while resizing maybe correct, but have to look into it a bit more.
		mDepthStencilBuffer = std::make_unique<DepthStencilBuffer>(mDevice.Get(), mDSVDescriptor.get(), mSrvCbvUavDescriptor.get(), DXGI_FORMAT_R24G8_TYPELESS, Application::GetClientWidth(), Application::GetClientHeight(), L"Depth Stencil Buffer");
	}

	void Device::BeginFrame()
	{
	}

	void Device::EndFrame()
	{
	}

	void Device::ExecuteContext(std::unique_ptr<GraphicsContext> graphicsContext)
	{
		// Execute commands recorded into the graphics context.
		mFrameFenceValues[mCurrentBackBufferIndex] = mGraphicsCommandQueue->ExecuteCommandList(graphicsContext->GetCommandList());
	}

	void Device::Present()
	{
		uint32_t syncInterval = mVSync ? 1u : 0u;
		uint32_t presentFlags = mTearingSupported && !mVSync ? DXGI_PRESENT_ALLOW_TEARING : 0u;

		ThrowIfFailed(mSwapChain->Present(syncInterval, presentFlags));

		mCurrentBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();

		mGraphicsCommandQueue->WaitForFenceValue(mFrameFenceValues[mCurrentBackBufferIndex]);
	}

	uint32_t Device::CreateSrv(const SrvCreationDesc& srvCreationDesc, ID3D12Resource* resource) const
	{
		uint32_t srvIndex = mSrvCbvUavDescriptor->GetCurrentDescriptorIndex();
		mDevice->CreateShaderResourceView(resource, &srvCreationDesc.srvDesc, mSrvCbvUavDescriptor->GetCurrentDescriptorHandle().cpuDescriptorHandle);
		
		mSrvCbvUavDescriptor->OffsetCurrentHandle();

		return srvIndex;
	}

	uint32_t Device::CreateRtv(const RtvCreationDesc& rtvCreationDesc, ID3D12Resource* resource) const
	{
		mDevice->CreateRenderTargetView(resource, nullptr,  mSrvCbvUavDescriptor->GetCurrentDescriptorHandle().cpuDescriptorHandle);
		mRTVDescriptor->OffsetCurrentHandle();

		return mRTVDescriptor->GetCurrentDescriptorIndex();
	}

	Buffer Device::CreateBuffer(const BufferCreationDesc& bufferCreationDesc, const void *data) const
	{
		Buffer buffer{};
		buffer.sizeInBytes = bufferCreationDesc.size * bufferCreationDesc.stride;

		ResourceCreationDesc resourceCreationDesc
		{
			.resourceDesc
			{
				.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
				.Width = buffer.sizeInBytes,
				.Height = 1u,
				.DepthOrArraySize = 1u,
				.MipLevels = 1u,
				.Format = DXGI_FORMAT_UNKNOWN,
				.SampleDesc
				{
					.Count = 1u,
					.Quality = 0u
				},
				.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
				.Flags = D3D12_RESOURCE_FLAG_NONE
			}
		};

		buffer.allocation = mMemoryAllocator->CreateResourceAllocation(bufferCreationDesc, resourceCreationDesc);

		// Currently, not using a backing storage for upload context's and such. Simply using D3D12MA to create a upload buffer, copy the data onto the upload buffer,
		// and then copy data from upload buffer -> GPU only buffer.
		if (data)
		{
			// Create upload buffer.
			BufferCreationDesc uploadBufferCreationDesc
			{
				.usage = BufferUsage::UploadBuffer,
				.size = bufferCreationDesc.size,
				.stride = bufferCreationDesc.stride,
				.name = L"Upload buffer - " + bufferCreationDesc.name,
			};

			std::unique_ptr<Allocation> uploadAllocation = mMemoryAllocator->CreateResourceAllocation(uploadBufferCreationDesc, resourceCreationDesc);

			uploadAllocation->Update(data, buffer.sizeInBytes);

			// Get a copy command and list and execute copy resource functions on the command queue.
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> copyCommandList = mCopyCommandQueue->GetCommandList();
			copyCommandList->CopyResource(buffer.allocation->resource.Get(), uploadAllocation->resource.Get());
			mCopyCommandQueue->ExecuteAndFlush(copyCommandList.Get());
		}

		if (bufferCreationDesc.usage == BufferUsage::StructuredBuffer || bufferCreationDesc.usage == BufferUsage::IndexBuffer || bufferCreationDesc.usage == BufferUsage::ConstantBuffer)
		{
			SrvCreationDesc srvCreationDesc
			{
				.srvDesc
				{
					.Format = DXGI_FORMAT_UNKNOWN,
					.ViewDimension = D3D12_SRV_DIMENSION_BUFFER,
					.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
					.Buffer
					{
						.FirstElement = 0u,
						.NumElements = bufferCreationDesc.size,
						.StructureByteStride = bufferCreationDesc.stride
					}
				}
			};

			buffer.srvUavCbvIndexInDescriptorHeap = CreateSrv(srvCreationDesc, buffer.allocation->resource.Get());
		}

		return buffer;
	}

	PipelineState Device::CreatePipelineState(const GraphicsPipelineStateCreationDesc& graphicsPipelineStateCreationDesc) const
	{
		PipelineState pipelineState(mDevice.Get(), graphicsPipelineStateCreationDesc);

		return pipelineState;
	}

}