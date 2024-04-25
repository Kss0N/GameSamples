#include "ConstBuffer.h"

// Quad is two triangles
static float2 c_QuadVertices[] =
{
    float2( 1,  1),
    float2(-1,  1),
    float2( 1, -1),
    
    float2( 1, -1),
    float2(-1,  1),
    float2(-1, -1),
};

static float3 c_RgbColors[] =
{
    float3(0, .4, 0),
    float3(0, .6, 0),
};


struct CheckBoardPsInput
{
    float4 position : SV_Position;
    float4 color : COLOR;
};