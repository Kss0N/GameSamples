#include "Common.hlsli"


SceneConstBuffer g_buffer : register(b0);



CheckBoardPsInput main(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID)
{
    
    
    float rows = g_buffer.rows_cols.x;
    float cols = g_buffer.rows_cols.y;
    
    float2 offset = float2(floor(instanceId / rows), instanceId % rows);
    
    float3x2 mat = float3x2(
        float2(1/rows, 0),
        float2(0, 1/cols),
        float2((2 / rows) * offset.x, (2 / cols) * offset.y) - float2(1 - 1 / rows, 1 - 1 / cols)
    );
    float2 pos = mul(float3(c_QuadVertices[vertexId], 1), mat).xy;
    
    
    CheckBoardPsInput ret;
    ret.position = float4(pos, 0, 1.f);
    ret.color = float4(c_RgbColors[(instanceId + uint(floor(instanceId / cols))) % 2], 1);
    return ret;
}