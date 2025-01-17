#include "CommandQueue.hpp"

namespace helios::gfx
{
	CommandQueue::CommandQueue(ID3D12Device5* const device, D3D12_COMMAND_LIST_TYPE commandListType, std::wstring_view commandQueueName) 
		: mCommandListType(commandListType)
	{
		mDevice = device;

		// Create the command queue based on list type.
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc
		{
			.Type = commandListType,
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
			.NodeMask = 0
		};

		ThrowIfFailed(mDevice->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&mCommandQueue)));
		mCommandQueue->SetName(commandQueueName.data());

		// Create command queue sync objects.
		ThrowIfFailed(mDevice->CreateFence(mFenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
		std::wstring fenceName = commandQueueName.data() + std::wstring(L" Fence");
		mFence->SetName(fenceName.data());

		mFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		
		if (!mFenceEvent)
		{
			ErrorMessage(L"Failed to create fence event");
		}
	}
	
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList1> CommandQueue::GetCommandList(const gfx::PipelineState* pipelineState)
	{
		// Check if there is command allocator (that allocates the list) is not in flight -> if so it can be reused.
		// Other wise a new command allocator has to be created.

		wrl::ComPtr<ID3D12GraphicsCommandList1> commandList;
		wrl::ComPtr<ID3D12CommandAllocator> commandAllocator;

		if (!mCommandAllocatorQueue.empty() && IsFenceComplete(mCommandAllocatorQueue.front().fenceValue))
		{
			commandAllocator = mCommandAllocatorQueue.front().commandAllocator;
			mCommandAllocatorQueue.pop();

			ThrowIfFailed(commandAllocator->Reset());
		}
		else
		{
			commandAllocator = CreateCommandAllocator();
		}

		// Check if there are any command list in the queue, hence create a new one.
		if (!mCommandListQueue.empty())
		{
			commandList = mCommandListQueue.front();
			mCommandListQueue.pop();

			if (pipelineState)
			{
				ThrowIfFailed(commandList->Reset(commandAllocator.Get(), pipelineState->pipelineStateObject.Get()));
			}
			else
			{
				ThrowIfFailed(commandList->Reset(commandAllocator.Get(), nullptr));
			}
		}
		else
		{
			commandList = CreateCommandList(commandAllocator.Get(), pipelineState);
		}

		// Create association between the command list and allocator so we can find out which command allocater the list was
		// allocated from.

		ThrowIfFailed(commandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get()));

		return commandList;
	}

	uint64_t CommandQueue::ExecuteCommandLists(std::span<ID3D12GraphicsCommandList1*> commandList)
	{
		for (auto& list : commandList)
		{
			list->Close();
		}

		std::vector<ID3D12CommandList*> commandListVec{};
		for (auto& list : commandList)
		{
			commandListVec.push_back(list);
		}

		mCommandQueue->ExecuteCommandLists(static_cast<UINT>(commandListVec.size()), commandListVec.data());
		uint64_t fenceValue = Signal();

		ID3D12CommandAllocator* commandAllocator{ nullptr };
		UINT dataSize = sizeof(ID3D12CommandAllocator);

		for (auto& list : commandList)
		{
			ThrowIfFailed(list->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator));


			mCommandAllocatorQueue.emplace(CommandAllocator{ .fenceValue = fenceValue, .commandAllocator = commandAllocator });
			mCommandListQueue.emplace(list);

			commandAllocator->Release();
		}

		return fenceValue;
	}

	void CommandQueue::ExecuteAndFlush(ID3D12GraphicsCommandList1* commandList)
	{
		std::array<ID3D12GraphicsCommandList1*, 1> commandLists{ std::move(commandList) };
		uint64_t signalValue = ExecuteCommandLists(commandLists);
		FlushQueue();
	}

	// Return fence value to for signal.
	uint64_t CommandQueue::Signal()
	{
		uint64_t fenceValueForSignal = ++mFenceValue;
		ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), fenceValueForSignal));

		return fenceValueForSignal;
	}

	bool CommandQueue::IsFenceComplete(uint64_t fenceValue)
	{
		return mFence->GetCompletedValue() >= fenceValue;
	}

	void CommandQueue::WaitForFenceValue(uint64_t fenceValue)
	{
		if (!IsFenceComplete(fenceValue))
		{
			ThrowIfFailed(mFence->SetEventOnCompletion(fenceValue, mFenceEvent));
			::WaitForSingleObject(mFenceEvent, static_cast<DWORD>(std::chrono::milliseconds::max().count()));
		}
	}

	void CommandQueue::FlushQueue()
	{
		uint64_t fenceValue = Signal();
		WaitForFenceValue(fenceValue);
	}

	wrl::ComPtr<ID3D12CommandAllocator> CommandQueue::CreateCommandAllocator()
	{
		wrl::ComPtr<ID3D12CommandAllocator> commandAllocator;
		ThrowIfFailed(mDevice->CreateCommandAllocator(mCommandListType, IID_PPV_ARGS(&commandAllocator)));

		return commandAllocator;
	}

	wrl::ComPtr<ID3D12GraphicsCommandList1> CommandQueue::CreateCommandList(ID3D12CommandAllocator* commandAllocator, const gfx::PipelineState* pipelineState)
	{
		wrl::ComPtr<ID3D12GraphicsCommandList1> commandList;
		ThrowIfFailed(mDevice->CreateCommandList(0u, mCommandListType, commandAllocator, pipelineState ? pipelineState->pipelineStateObject.Get() : nullptr, IID_PPV_ARGS(&commandList)));

		return commandList;
	}
}