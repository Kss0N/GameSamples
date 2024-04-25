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

static float2 c_QuadUV[] =
{
    float2(1, 0),
    float2(0, 0),
    float2(1, 1),
    
    float2(1, 1),
    float2(0, 0),
    float2(0, 1)
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

float3x2 snake_to_viewport_matrix(float rows, float cols, float snakeX, float snakeY)
{
    return float3x2(
        float2(1 / rows, 0),
        float2(0, 1 / cols),
        float2((2 / rows) * snakeX, (2 / cols) * snakeY) - float2(1 - 1 / rows, 1 - 1 / cols)
    );
}

static float3x2 quad_to_uv_matrix = transpose(float2x3(
    float3(0.5, 0, .5),
    float3(0, -.5, .5)
));