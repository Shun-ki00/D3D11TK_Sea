#include "Sea.hlsli"


VS_OUTPUT main(VS_INPUT input, uint instanceId : SV_InstanceID)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
    
    // 頂点座標を設定
    float3 offset = InstanceBuffer[instanceId];
    output.position = input.position + float4(offset, 0.0f);
    
    // UV値を設定
    output.uv = input.uv;
    
    return output;
}