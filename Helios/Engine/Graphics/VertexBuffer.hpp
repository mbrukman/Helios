#pragma once

#include "Pch.hpp"

#include "GFXUtils.hpp"

namespace helios::gfx
{
	class VertexBuffer
	{
	public:
		template <typename T>
		void Init(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, std::span<const T> data)
		{
			auto vertexBuffer = utils::CreateGPUBuffer<T>(device, commandList, data);
			m_Buffer = vertexBuffer.first;
			m_IntermediateBuffer = vertexBuffer.second;
		
			m_BufferView =
			{
				.BufferLocation = m_Buffer->GetGPUVirtualAddress(),
				.SizeInBytes = static_cast<UINT>(data.size_bytes()),
				.StrideInBytes = sizeof(T)
			};
		}

		D3D12_VERTEX_BUFFER_VIEW GetBufferView()
		{
			return m_BufferView;
		}

		// The intermediate buffer is needed to be in scope until the command list has finished execution.
		// Because of this, the intermediate buffer is a data member.
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_Buffer;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_IntermediateBuffer;

		D3D12_VERTEX_BUFFER_VIEW m_BufferView{};
	};
}

