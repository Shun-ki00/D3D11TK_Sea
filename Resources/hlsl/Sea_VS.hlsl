#include "Sea.hlsli"


VS_OUTPUT main(VS_INPUT input, uint instanceId : SV_InstanceID)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    // ’¸“_À•W‚ğİ’è
    float3 offset = InstanceBuffer[instanceId];
    output.position = input.position + float4(offset, 0.0f);
    
    // UV’l‚ğİ’è
    output.uv = input.uv;
    
    return output;
}