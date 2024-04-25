#ifndef CONST_BUFFER_H
#define CONST_BUFFER_H

#ifdef __cplusplus
#include "DirectXMath.h"
namespace hlsl 
{
	using float2 = DirectX::XMFLOAT2;
	using float3 = DirectX::XMFLOAT3;
	using float4 = DirectX::XMVECTOR;

	using float2x2 = DirectX::XMVECTOR;// FLOAT2X2 does to not exist

	using uint = UINT;
	using uint2 = DirectX::XMUINT2;
#define CONST_BUFFER_ALIGNED alignas(256)
#else 
#define CONST_BUFFER_ALIGNED
#endif // __cplusplus

struct CONST_BUFFER_ALIGNED SceneConstBuffer
{
	uint2 rows_cols;
	float2 sheet_scales;

};

#ifdef __cplusplus
}
#endif // __cplusplus



#endif // !CONST_BUFFER_H