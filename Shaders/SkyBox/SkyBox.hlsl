#include "../Common/BindlessRS.hlsli"
#include "../Common/ConstantBuffers.hlsli"

struct VSOutput
{
    float4 position : SV_Position;
    float4 modelSpacePosition : POSITION;
};

ConstantBuffer<SkyBoxRenderResources> renderResource : register(b0);

[RootSignature(BindlessRootSignature)]
VSOutput VsMain(uint vertexID : SV_VertexID)
{
    StructuredBuffer<float3> positionBuffer = ResourceDescriptorHeap[renderResource.positionBufferIndex];

    ConstantBuffer<SceneBuffer> sceneBuffer = ResourceDescriptorHeap[renderResource.sceneBufferIndex];

    VSOutput output;
    output.position = mul(float4(positionBuffer[vertexID], 0.0f), sceneBuffer.viewProjectionMatrix);
    output.modelSpacePosition = float4(positionBuffer[vertexID].xyz, 0.0f);
    output.position = output.position.xyww;

    return output;
}

float4 PsMain(VSOutput input) : SV_Target
{
    TextureCube environmentTexture = ResourceDescriptorHeap[renderResource.textureIndex];

    float3 samplingVector = normalize(input.modelSpacePosition.xyz);

    return environmentTexture.Sample(linearWrapSampler, samplingVector);
}