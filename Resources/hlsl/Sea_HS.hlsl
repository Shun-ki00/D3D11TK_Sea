#include "Sea.hlsli"


// ハルシェーダー本体
HS_CONSTANT_OUTPUT HS_ConstantFunction(InputPatch<HS_INPUT, 32> patch, uint patchID : SV_PrimitiveID)
{
    HS_CONSTANT_OUTPUT output;
    
    float3 worldPatch1 = mul(patch[0].position, matWorld);
    float3 worldPatch2 = mul(patch[1].position, matWorld);
    float3 worldPatch3 = mul(patch[2].position, matWorld);
    
    // パッチ中心座標をワールド座標で計算
    float3 patchCenter = (worldPatch1 + worldPatch2 + worldPatch3) / 3.0f;

   // XZ距離を取得
    float2 camXZ = cameraPosition.xz;
    float2 patchXZ = patchCenter.xz;
    float distXZ = distance(camXZ, patchXZ);

    // パラメータ調整
    float maxRange = 1100.0f; // 減衰開始?最大範囲
    float normalizedDist = saturate(distXZ / maxRange);

    // 非線形減衰（指数関数型）
    float decay = pow(1.0f - normalizedDist, 2.0f); // 二次減衰

    // 出力範囲 64 → 1 へマッピング
    float tessFactor = lerp(1.0f, 64.0f, decay);
    
    // 2. 高さ差による補正（200以上 → 1/2、未満 → 1.0）
    float yDelta = abs(patchCenter.y - cameraPosition.y);
    float heightMask = lerp(1.0f, 0.1f, step(200.0f, yDelta)); // 200以上で 0.5

    // 3. 結果に適用
    tessFactor *= heightMask;
    
    // エッジの細分化レベルを設定
    output.EdgeTess[0] = tessFactor; // 左
    output.EdgeTess[1] = tessFactor; // 上
    output.EdgeTess[2] = tessFactor; // 右
    output.EdgeTess[3] = tessFactor; // 下

    // 内部の細分化レベル（U方向とV方向）
    output.InsideTess[0] = tessFactor; // U方向
    output.InsideTess[1] = tessFactor; // V方向

  
    return output;
}



// 各制御点の出力
[domain("quad")] // パッチの形状（"tri"は三角形）"quad"（四角形）"isoline"（線状）
[partitioning("integer")] // 細分化の方式（整数テッセレーション）"fractional_odd"（奇数分割）"fractional_even"（偶数分割）
[outputtopology("triangle_cw")] // 出力トポロジー（時計回りの三角形）任意の正の整数値（通常は3（三角形）または4（四角形））
[outputcontrolpoints(32)] // 出力する制御点の数（3点パッチ）
[patchconstantfunc("HS_ConstantFunction")] // テッセレーション定数計算関数の指定
HS_OUTPUT main(InputPatch<HS_INPUT, 32> patch, uint controlPointID : SV_OutputControlPointID)
{
    HS_OUTPUT output;

    // 入力をそのまま出力（制御点を加工しない場合）
    output.position = patch[controlPointID].position;
    output.uv = patch[controlPointID].uv;

    return output;
}