#include "SnakeCommon.hlsli"


//
// 1. Translate SnakeCoords to Viewport Coords & transform the quads
//	- 
// 
// 2. Translate quads into texture Coords
//	- In Viewport Coords top-left is (-1, 1), Bottom Right is (1, -1)
//	- In Texture Coords top-left is (0,0), bottom right is (1, 1)
//  - Top-Left refers to the Top-Left corner of the Texture2D being sampled from
//  - Since a sprite sheet is being used, Texcoords need to be transformed to only cover a portion of the sprite sheet.
//	- That is done by down-scaling and translating.
//
//

SnakePSInput main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID,
    uint2 snake_coords : SNAKE_POS,
    float2 sheet_offset: SHEET_OFFSET)
{
    float board_rows = float(g_buffer.rows_cols.x),
        board_cols = float(g_buffer.rows_cols.y);   
    
    float2 pos = mul(
        float3(c_QuadVertices[vertexID], 1), 
        snake_to_viewport_matrix(board_rows, board_cols, float(snake_coords.x), float(snake_coords.y))
    );
    
    float2 quad_as_uv = mul(float3(c_QuadVertices[vertexID], 1), quad_to_uv_matrix);
    
    SnakePSInput ret;
    ret.pos = float4(c_QuadVertices[vertexID], 0, 1);
    //ret.uv = float2(1/5 * quad_as_uv.x + sheet_offset.x, 1/4 * quad_as_uv.y + sheet_offset.y);
    //ret.uv = quad_as_uv;
    ret.uv = c_QuadUV[vertexID];
    return ret;
}