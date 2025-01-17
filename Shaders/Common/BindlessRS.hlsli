#ifndef __BINDLESS_RS_HLSLI__
#define __BINDLESS_RS_HLSLI__

#ifndef __cplusplus

// Set the matrix packing to row major by default. Prevents needing to transpose matrice's on the C++ side.
#pragma pack_matrix(row_major)

#define BindlessRootSignature           \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED | SAMPLER_HEAP_DIRECTLY_INDEXED)," \
    "RootConstants(b0, num32BitConstants=64, visibility = SHADER_VISIBILITY_ALL)," \
    "StaticSampler(s0, filter = FILTER_MIN_MAG_MIP_POINT, addressU = TEXTURE_ADDRESS_CLAMP, addressV = TEXTURE_ADDRESS_CLAMP, addressW = TEXTURE_ADDRESS_CLAMP, mipLODBias = 0.0f, minLOD = 0.0f, maxLOD = 100.0f)," \
    "StaticSampler(s1, filter = FILTER_MIN_MAG_MIP_POINT, addressU = TEXTURE_ADDRESS_WRAP, addressV = TEXTURE_ADDRESS_WRAP, addressW = TEXTURE_ADDRESS_WRAP,  mipLODBias = 0.0f, minLOD = 0.0f, maxLOD = 100.0f)," \
    "StaticSampler(s2, filter = FILTER_MIN_MAG_MIP_LINEAR, addressU = TEXTURE_ADDRESS_WRAP, addressV = TEXTURE_ADDRESS_WRAP, addressW = TEXTURE_ADDRESS_WRAP,  mipLODBias = 0.0f, minLOD = 0.0f, maxLOD = 100.0f), " \
    "StaticSampler(s3, filter = FILTER_MIN_MAG_LINEAR_MIP_POINT, addressU = TEXTURE_ADDRESS_CLAMP, addressV = TEXTURE_ADDRESS_CLAMP, addressW = TEXTURE_ADDRESS_WRAP,  mipLODBias = 0.0f, minLOD = 0.0f, maxLOD = 100.0f), " \
    "StaticSampler(s4, filter = FILTER_MIN_MAG_LINEAR_MIP_POINT, addressU = TEXTURE_ADDRESS_CLAMP, addressV = TEXTURE_ADDRESS_CLAMP, addressW = TEXTURE_ADDRESS_CLAMP)," \
    "StaticSampler(s5, filter = FILTER_MIN_MAG_LINEAR_MIP_POINT, addressU = TEXTURE_ADDRESS_WRAP, addressV = TEXTURE_ADDRESS_WRAP, addressW = TEXTURE_ADDRESS_WRAP)," \
    "StaticSampler(s6, filter = FILTER_MIN_MAG_POINT_MIP_LINEAR, addressU = TEXTURE_ADDRESS_WRAP, addressV = TEXTURE_ADDRESS_WRAP, addressW = TEXTURE_ADDRESS_WRAP), " \
    "StaticSampler(s7, filter = FILTER_MIN_MAG_POINT_MIP_LINEAR, addressU = TEXTURE_ADDRESS_CLAMP, addressV = TEXTURE_ADDRESS_CLAMP, addressW = TEXTURE_ADDRESS_CLAMP), " \
    "StaticSampler(s8, filter = FILTER_ANISOTROPIC, maxAnisotropy = 16)"

// Samplers
SamplerState pointClampSampler : register(s0);
SamplerState pointWrapSampler : register(s1);
SamplerState linearWrapSampler : register(s2);
SamplerState linearClampSampler: register(s3);
SamplerState minMapLinearMipPointClampSampler : register(s4);
SamplerState minMapLinearMipPointWrapSampler : register(s5);
SamplerState minMapPointMipLinearClampSampler : register(s6);
SamplerState minMapPointMipLinearWrapSampler : register(s7);
SamplerState anisotropicSampler : register(s8);

#else // ifdef __cplusplus
#define uint uint32_t
#endif

// All *RenderResources structs are placed here to prevent having them in multiple places.
struct MeshViewerRenderResources
{
    uint positionBufferIndex;
    uint textureBufferIndex;
    uint normalBufferIndex;
    uint tangetBufferIndex;
    
    uint transformBufferIndex;
    
    uint sceneBufferIndex;

    uint albedoTextureIndex;
    uint albedoTextureSamplerIndex;

    uint metalRoughnessTextureIndex;
    uint metalRoughnessTextureSamplerIndex;

    uint normalTextureIndex;
    uint normalTextureSamplerIndex;

    uint aoTextureIndex;
    uint aoTextureSamplerIndex;

    uint emissiveTextureIndex;
    uint emissiveTextureSamplerIndex;
};

struct LightRenderResources
{
    uint positionBufferIndex;
    uint lightBufferIndex;

    uint transformBufferIndex;
    uint sceneBufferIndex;
};

// Its technically the Deferred GPass render resources, but as it contains all details for PBR stuff its named as such.
struct PBRRenderResources
{
    uint positionBufferIndex;
    uint textureBufferIndex;
    uint normalBufferIndex;
    uint tangentBufferIndex;
    uint biTangentBufferIndex;

    uint transformBufferIndex;

    uint sceneBufferIndex;
    uint lightBufferIndex;

    uint albedoTextureIndex;
    uint albedoTextureSamplerIndex;

    uint metalRoughnessTextureIndex;
    uint metalRoughnessTextureSamplerIndex;

    uint normalTextureIndex;
    uint normalTextureSamplerIndex;

    uint aoTextureIndex;
    uint aoTextureSamplerIndex;

    uint emissiveTextureIndex;
    uint emissiveTextureSamplerIndex;
};

struct SceneRenderResources
{
    uint sceneBufferIndex;
    uint lightBufferIndex;
};

struct RenderTargetRenderResources
{   
    uint positionBufferIndex;
    uint textureBufferIndex;
    uint textureIndex;
    uint postProcessBufferIndex;
};

struct PostProcessRenderResources
{
    uint positionBufferIndex;
    uint textureBufferIndex;
    uint finalRenderTextureIndex;
    uint bloomTextureIndex;
    uint postProcessBufferIndex;
};

struct DeferredLightingPassRenderResources
{
    uint positionBufferIndex;
    uint textureBufferIndex;

    uint lightBufferIndex;
    uint sceneBufferIndex;
    
    uint albedoGBufferIndex;
    uint positionEmissiveGBufferIndex;
    uint normalEmissiveGBufferIndex;
    uint aoMetalRoughnessEmissiveGBufferIndex;

    uint shadowMappingBufferIndex;
    uint shadowDepthTextureIndex;
    
    uint irradianceMapIndex;
    uint prefilterMapIndex;
    uint brdfLutIndex;
};

struct ShadowMappingRenderResources
{
    uint positionBufferIndex;
    uint transformBufferIndex;
    uint shadowMappingBufferIndex;
};

struct BloomPassRenderResources
{
    uint inputTextureIndex;
    uint outputTextureIndex;
    uint bloomBufferIndex;
};

struct MipMapGenerationRenderResources
{
    uint sourceMipIndex;

    uint outputMip1Index;
    uint outputMip2Index;
    uint outputMip3Index;
    uint outputMip4Index;

    uint mipMapGenerationBufferIndex;
};

struct CubeFromEquirectRenderResources
{
    uint textureIndex;
    uint outputTextureIndex;
};

struct SkyBoxRenderResources
{
    uint positionBufferIndex;
    uint sceneBufferIndex;
    uint textureIndex;
};

struct IrradianceRenderResources
{
    uint skyBoxTextureIndex;
    uint ouputIrradianceMapIndex;
};

struct PreFilterCubeMapRenderResources
{
    uint skyBoxTextureIndex;
    uint outputPreFilteredCubeMapIndex;
    uint mipLevel;
};

struct BRDFLutRenderResources
{
    uint lutTextureIndex;
};

#endif