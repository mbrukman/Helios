#pragma once

#include "Context.hpp"

namespace helios::gfx
{
	class Device;

	// Wrapper class for Graphics CommandList, which provides a set of easy and simple functions to record commands for execution by GPU.
	// The command queue will contain a queue of command list, which can be passed into the GraphicsContext's constructor to create a GraphicsContext object.
	// Note : Can be used for some compute stuff as well for convenience, though you should probably switch to ComputeContext for using the Compute pipeline.
	// note (rtarun9) : This design is subject to change.
	class GraphicsContext : public Context
	{
	public:
		GraphicsContext(Device* device, const gfx::PipelineState* pipelineState = nullptr);

		// Core functionalities.

		void ClearRenderTargetView(BackBuffer* const backBuffer, std::span<const float, 4> color);
		void ClearRenderTargetView(std::span<const RenderTarget*> renderTargets, std::span<const float, 4> color);
		void ClearRenderTargetView(RenderTarget* renderTargets, std::span<const float, 4> color);

		void ClearDepthStencilView(Texture* const depthStencilTexture, float depth = 1.0f);

		void SetDescriptorHeaps(std::array<gfx::Descriptor*, 2> descriptors) const;

		// COnfigure pipeline / root signature related functions.
		void SetGraphicsPipelineState(PipelineState* const pipelineState) const;
		void SetComputePipelineState(PipelineState* const pipelineState) const;
		void SetGraphicsRootSignature(PipelineState* const pipelineState) const;
		void SetComputeRootSignature(PipelineState* const pipelineState) const;
		void SetPipelineStateObject(PipelineState* constpipelineState) const;

		void SetIndexBuffer(Buffer* const buffer) const;
		void Set32BitGraphicsConstants(const void* renderResources) const;

		void SetDefaultViewportAndScissor() const;
		void SetViewportAndScissor(const D3D12_VIEWPORT& viewport) const;
		
		void SetPrimitiveTopologyLayout(D3D_PRIMITIVE_TOPOLOGY primitiveTopology) const;

		void SetRenderTarget(BackBuffer* const renderTarget, const Texture* depthStencilTexture) const;
		void SetRenderTarget(std::span<const RenderTarget*> renderTargets, const Texture* depthStencilTexture) const;
		void SetRenderTarget(RenderTarget* renderTarget, const Texture* depthStencilTexture) const;
		void SetRenderTarget(const Texture* depthStencilTexture) const;

		// Draw functions.
		void DrawInstanceIndexed(uint32_t indicesCount, uint32_t instanceCount = 1u) const;
		void DrawIndexed(uint32_t indicesCount, uint32_t instanceCount = 1u) const;

		// Compute functions (as graphics context can be used for compute as well).
		void Dispatch(uint32_t threadGroupX, uint32_t threadGroupY, uint32_t threadGroupZ) const;
		void Set32BitComputeConstants(const void* renderResources) const;
		
		// Copy related calls.
		void CopyResource(ID3D12Resource* source, ID3D12Resource* destination);

	private:
		static constexpr uint32_t NUMBER_32_BIT_CONSTANTS = 64;

		// Functions such as SetRenderTargets() need to get the descriptor handle (present in the Device class) from the texture index.
		// For similar reasons, the GraphicsContext has a reference to the device.
		Device& mDevice;
	};
}


