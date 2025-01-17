#pragma once

#include "Descriptor.hpp"
#include "CommandQueue.hpp"
#include "GraphicsContext.hpp"
#include "MemoryAllocator.hpp"
#include "PipelineState.hpp"
#include "ComputeContext.hpp"
#include "MipMapGenerator.hpp"

namespace helios::gfx
{
	// Abstraction for creating / destroying various graphics resources.
	// Encapsulates most renderer resources / objects in use : the swap chain, descriptor heaps, command queue's, etc.
	// Inspired from : https://alextardif.com/DX12Tutorial.html.
	class Device
	{
	public:
		Device();
		~Device();

		Device(const Device& other) = delete;
		Device& operator=(const Device& other) = delete;

		Device(Device&& other) = delete;
		Device& operator=(Device&& other) = delete;

		// Getters
		ID3D12Device5* GetDevice() const { return mDevice.Get(); }
		IDXGIFactory6* GetFactory() const { return mFactory.Get(); }
		IDXGIAdapter4* GetAdapter() const { return mAdapter.Get(); }
		IDXGISwapChain4* GetSwapChain() const { return mSwapChain.Get(); }

		Descriptor* GetRtvDescriptor() const { return mRtvDescriptor.get(); }
		Descriptor* GetSrvCbvUavDescriptor() const { return mSrvCbvUavDescriptor.get(); }
		Descriptor* GetDsvDescriptor() const { return mDsvDescriptor.get(); }
		Descriptor* GetSamplerDescriptor() const { return mSamplerDescriptor.get(); }

		CommandQueue* GetGraphicsCommandQueue() const { return mGraphicsCommandQueue.get(); }
		CommandQueue* GetComputeCommandQueue() const { return mComputeCommandQueue.get(); }

		MemoryAllocator* GetMemoryAllocator() const { return mMemoryAllocator.get(); }
		BackBuffer* GetCurrentBackBuffer() { return &mBackBuffers[mCurrentBackBufferIndex]; }
		
		std::unique_ptr<GraphicsContext> const  GetGraphicsContext(const gfx::PipelineState* pipelineState = nullptr) { return std::move(std::make_unique<GraphicsContext>(this, pipelineState)); }
		std::unique_ptr<ComputeContext> const GetComputeContext(const gfx::PipelineState* pipelineState = nullptr) { return std::move(std::make_unique<ComputeContext>(this, pipelineState)); }
		
		MipMapGenerator* GetMipMapGenerator()  { return mMipMapGenerator.get(); }
		
		// Misc getters for resources and their contents.
		DescriptorHandle const GetTextureSrvDescriptorHandle(const Texture* texture) { return mSrvCbvUavDescriptor->GetDescriptorHandleFromIndex(texture->srvIndex); }

		// Device resources are the device, adapters, queues, descriptor heaps, etc.
		// Swap chain resources are kept separate for resources with depended upon the window.
		void InitDeviceResources();
		void InitSwapChainResources();

		void CreateBackBufferRTVs();
		void ResizeBuffers();

		// Rendering related functions.
		void BeginFrame();
		void EndFrame();

		void ExecuteContext(std::span<std::unique_ptr<GraphicsContext>> graphicsContext);
		void ExecuteContext(std::span<std::unique_ptr<ComputeContext>> computeContext);

		void ExecuteContext(std::unique_ptr<GraphicsContext> graphicsContext);
		void ExecuteContext(std::unique_ptr<ComputeContext> computeContext);
	
		void Present();

		// Helper creation functions.
		uint32_t CreateSrv(const SrvCreationDesc& srvCreationDesc, ID3D12Resource* resource) const;
		uint32_t CreateRtv(const RtvCreationDesc& rtvCreationDesc, ID3D12Resource* resource) const;
		uint32_t CreateDsv(const DsvCreationDesc& dsvCreationDesc, ID3D12Resource* resource) const;
		uint32_t CreateUav(const UavCreationDesc& uavCreationDesc, ID3D12Resource* resource) const;
		uint32_t CreateCbv(const CbvCreationDesc& cbvCreationDesc) const;
		uint32_t CreateSampler(const SamplerCreationDesc& samplerCreationDesc) const;

		template <typename T>
		Buffer CreateBuffer(const BufferCreationDesc& bufferCreationDesc, std::span<const T> data = {}) const;

		
		// note(rtarun9) : The creation desc are not passed as const T&, as the contents (the dimensions) are not set by user if the texture is being loaded from file.
		// Because of this, its passed as reference and not const reference.
		Texture CreateTexture(TextureCreationDesc& textureCreationDesc, const unsigned char *data = nullptr) const;
		RenderTarget CreateRenderTarget(TextureCreationDesc& textureCreationDesc) const;

		PipelineState CreatePipelineState(const GraphicsPipelineStateCreationDesc& graphicsPipelineStateCreationDesc) const;
		PipelineState CreatePipelineState(const ComputePipelineStateCreationDesc& computePipelineStateCreationDesc) const;

		// Resource resizing operations.
		void ResizeRenderTarget(RenderTarget* renderTarget);

		// VSync related functions.
		void EnableVSync() { mVSync = true; }
		void DisableVSync() { mVSync = false; }

	public:
		// Number of SwapChain back buffers.
		static constexpr uint8_t NUMBER_OF_FRAMES = 3u;
		static constexpr DXGI_FORMAT SWAPCHAIN_FORMAT = DXGI_FORMAT_R10G10B10A2_UNORM;
	private:
		Microsoft::WRL::ComPtr<ID3D12Device5> mDevice{};
		Microsoft::WRL::ComPtr<ID3D12DebugDevice2> mDebugDevice{};
		Microsoft::WRL::ComPtr<ID3D12Debug5> mDebugInterface{};

		Microsoft::WRL::ComPtr<ID3D12DeviceRemovedExtendedDataSettings> mDredSettings{};

		Microsoft::WRL::ComPtr<IDXGIFactory6> mFactory{};
		Microsoft::WRL::ComPtr<IDXGIAdapter4> mAdapter{};
		Microsoft::WRL::ComPtr<IDXGISwapChain4> mSwapChain{};
		
		bool mVSync{true};
		bool mTearingSupported{};

		bool mIsInitialized{ false };

		uint32_t mCurrentBackBufferIndex{};
		std::array<BackBuffer, NUMBER_OF_FRAMES> mBackBuffers{};

		std::array<uint64_t, NUMBER_OF_FRAMES> mFrameFenceValues{};

		std::unique_ptr<MemoryAllocator> mMemoryAllocator{};

		std::unique_ptr<Descriptor> mRtvDescriptor{};
		std::unique_ptr<Descriptor> mDsvDescriptor{};
		std::unique_ptr<Descriptor> mSrvCbvUavDescriptor{};
		std::unique_ptr<Descriptor> mSamplerDescriptor{};

		std::unique_ptr<CommandQueue> mGraphicsCommandQueue{};
		std::unique_ptr<CommandQueue> mComputeCommandQueue{};
		std::unique_ptr<CommandQueue> mCopyCommandQueue{};

		std::unique_ptr<MipMapGenerator> mMipMapGenerator{};

		// Mutex is used primarily to prevent race conditions where more than one resource has the same index.
		mutable std::recursive_mutex mResourceMutex{};
	};

	template <typename T>
	Buffer Device::CreateBuffer(const BufferCreationDesc& bufferCreationDesc, std::span<const T> data) const
	{
		Buffer buffer{};

		// If data.size() == 0, it means that the data to fill the buffer will be passed later on (via the Update functions).
		uint32_t numberComponents = data.size() == 0 ? 1 : static_cast<uint32_t>(data.size());

		buffer.sizeInBytes = numberComponents * sizeof(T);

		ResourceCreationDesc resourceCreationDesc = ResourceCreationDesc::CreateBufferResourceCreationDesc(buffer.sizeInBytes);


		buffer.allocation = mMemoryAllocator->CreateBufferResourceAllocation(bufferCreationDesc, resourceCreationDesc);

		std::lock_guard<std::recursive_mutex> resourceLockGuard(mResourceMutex);
		// Currently, not using a backing storage for upload context's and such. Simply using D3D12MA to create a upload buffer, copy the data onto the upload buffer,
		// and then copy data from upload buffer -> GPU only buffer.
		if (data.data())
		{
			// Create upload buffer.
			BufferCreationDesc uploadBufferCreationDesc
			{
				.usage = BufferUsage::UploadBuffer,
				.name = L"Upload buffer - " + bufferCreationDesc.name,
			};

			std::unique_ptr<Allocation> uploadAllocation = mMemoryAllocator->CreateBufferResourceAllocation(uploadBufferCreationDesc, resourceCreationDesc);

			uploadAllocation->Update(data.data(), buffer.sizeInBytes);

			// Get a copy command and list and execute copy resource functions on the command queue.
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList1> copyCommandList = mCopyCommandQueue->GetCommandList();
			copyCommandList->CopyResource(buffer.allocation->resource.Get(), uploadAllocation->resource.Get());
			mCopyCommandQueue->ExecuteAndFlush(copyCommandList.Get());

			uploadAllocation->Reset();
		}

		if (bufferCreationDesc.usage == BufferUsage::StructuredBuffer)
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
						.NumElements = static_cast<UINT>(data.size()),
						.StructureByteStride = static_cast<UINT>(sizeof(T))
					}
				}
			};

			buffer.srvIndex = CreateSrv(srvCreationDesc, buffer.allocation->resource.Get());
		}

		else if (bufferCreationDesc.usage == BufferUsage::ConstantBuffer)
		{
			CbvCreationDesc cbvCreationDesc
			{
				.cbvDesc
				{
					.BufferLocation = buffer.allocation->resource->GetGPUVirtualAddress(),
					.SizeInBytes = static_cast<UINT>(buffer.sizeInBytes)
				}
			};

			buffer.cbvIndex = CreateCbv(cbvCreationDesc);
		}

		buffer.bufferName = bufferCreationDesc.name;
		return buffer;
	}
}