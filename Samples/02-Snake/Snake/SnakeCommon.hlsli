#include "Common.hlsli"

struct SnakePSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

SceneConstBuffer g_buffer : register(b0);

Texture2D g_sprite_sheet : register(t0);

SamplerState g_sampler : register(s0);