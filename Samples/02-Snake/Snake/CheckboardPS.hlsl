#include "Common.hlsli"


float4 main(CheckBoardPsInput input) : SV_TARGET
{
    return input.color;
}