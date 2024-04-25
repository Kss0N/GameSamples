#include "SnakeCommon.hlsli"

float4 main(SnakePSInput input) : SV_TARGET
{
    return g_sprite_sheet.Sample(g_sampler, input.uv);
}