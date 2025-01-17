#pragma once

#include "PipelineState.hpp"

namespace helios::gfx
{
	// note(rtarun9) : since command allocators cannot be reset until they are no longer 'in flight', we store a fence value along with the command allocator,
	// to keep track if the commands have been executed by a command queue or not.
	struct CommandAllocator
	{
		uint64_t fenceValue{};
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	};

	// Command Queue abstraction : has a  circular queue for command list and allocators.
	// Has interfaces related to synchronization.
	class CommandQueue
	{
	public:
		CommandQueue(ID3D12Device5* const device, D3D12_COMMAND_LIST_TYPE commandListType = D3D12_COMMAND_LIST_TYPE_DIRECT, std::wstring_view commandQueueName = L"Main Command Queue");

		// GetCommandList and GetCommandQueue returns ComPtr as the pointer is also passed to the contexts.
		[[nodiscard]]
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList1> GetCommandList(const gfx::PipelineState* pipelineState = nullptr);

		[[nodiscard]]
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue() const { return mCommandQueue; }

		// Returns the fence value to wait for to notify when command list has finished execution.
		[[nodiscard]]
		uint64_t ExecuteCommandLists(std::span<ID3D12GraphicsCommandList1*> commandList);

		void ExecuteAndFlush(ID3D12GraphicsCommandList1* commandList);

		// Return value that the fence will be signalled with when GPU commands finish executing.
		[[nodiscard]]
		uint64_t Signal();

		bool IsFenceComplete(uint64_t fenceValue);
		void WaitForFenceValue(uint64_t fenceValue);
		void FlushQueue();

	private:
		// Helper functions to create command list / command allocator if none are available in the queue.
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator();
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList1> CreateCommandList(ID3D12CommandAllocator* commandAllocator, const gfx::PipelineState* pipelineState = nullptr);

	private:
		D3D12_COMMAND_LIST_TYPE mCommandListType{D3D12_COMMAND_LIST_TYPE_DIRECT};

		// Since the command queue will interact with the ID3D12Device interface, a reference to it is stored here.
		// Because of ComPtr's ref counting mechanism, this should not cause any problems.
		Microsoft::WRL::ComPtr<ID3D12Device5> mDevice{};

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> mCommandQueue{};
		
		// Synchronization objects.
		Microsoft::WRL::ComPtr<ID3D12Fence> mFence{};
		HANDLE mFenceEvent{};
		uint64_t mFenceValue{};

		std::queue<CommandAllocator> mCommandAllocatorQueue{};
		std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList1>> mCommandListQueue{};
	};
}