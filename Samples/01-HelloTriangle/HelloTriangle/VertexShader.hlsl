#include "Common.hlsli"

static float2 poss[] =
{
    float2(0, .5),
    float2(-.33, -.25),
    float2(0.33, -.25),
};
static float4 colors[] =
{
    float4(1, 0, 0, 1),
    float4(0, 1, 0, 1),
    float4(0, 0, 1, 1),
};


PSInput main(uint id : SV_VertexID ) 
{
    PSInput ret;
    ret.pos = float4(poss[id], 0, 1);    
    ret.color = colors[id];
    return ret;
}
