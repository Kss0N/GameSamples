#include "Common.hlsli"


SceneConstBuffer g_buffer : register(b0);



CheckBoardPsInput main(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID)
{
    float 
        rows = g_buffer.rows_cols.x,
        cols = g_buffer.rows_cols.y;
    
    float2 offset = float2(floor(instanceId / rows), instanceId % rows);
    
    float2 pos = mul(float3(c_QuadVertices[vertexId], 1), snake_to_viewport_matrix(rows, cols, offset.x, offset.y)).xy;
    
    CheckBoardPsInput ret;
    ret.position = float4(pos, 0, 1.f);
    ret.color = float4(c_RgbColors[(instanceId + uint(floor(instanceId / cols))) % 2], 1);
    return ret;
}