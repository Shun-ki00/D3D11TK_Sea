#include "Sea.hlsli"


// 頂点シェーダー本体
// 頂点シェーダーからハルシェーダーへ
VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

   // ローカル空間 → ワールド空間
    float4 worldPos = mul(input.position, matWorld);
    
    // ワールド空間 → ビュー空間 → クリップ空間
    output.position = mul(worldPos, matView);
    output.position = mul(output.position, matProj);

    
    //output.position = float4(input.position, 1.0f);
    
    // ローカル空間 → ワールド空間
   // output.position = mul(float4(input.position, 1.0f), matWorld);
    
    output.position = input.position;
    // 頂点色とUV座標をそのまま渡す
    output.uv = input.uv;

    return output;
}