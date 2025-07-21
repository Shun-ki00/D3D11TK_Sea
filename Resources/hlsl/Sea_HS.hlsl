#include "Sea.hlsli"

static const float MAX_TESSELLATION_RANGE = 1100.0f;

static const float MIN_TESSELLATION = 1.0f;
static const float MAX_TESSELLATION = 10.0f;

static const float HEIGHT_THRESHOLD = 200.0f;
static const float HEIGHT_PENALTY = 0.1f;


// ハルシェーダー本体
HS_CONSTANT_OUTPUT HS_ConstantFunction(InputPatch<HS_INPUT, 4> patch, uint patchID : SV_PrimitiveID)
{
    HS_CONSTANT_OUTPUT output;
    
    // パッチのワールド座標変換
    float3 worldPatch1 = mul(patch[0].position, matWorld);
    float3 worldPatch2 = mul(patch[1].position, matWorld);
    float3 worldPatch3 = mul(patch[2].position, matWorld);
    
    // パッチ中心座標をワールド座標で計算
    float3 patchCenter = (worldPatch1 + worldPatch2 + worldPatch3) / 3.0f;
    
    // XZ距離を取得
    float2 cameraXZ = cameraPosition.xz;
    float2 patchXZ = patchCenter.xz;
    float horizontalDistance = distance(cameraXZ, patchXZ);
    
    // 距離を0-1に正規化
    float normalizedDistance = saturate(horizontalDistance / MAX_TESSELLATION_RANGE);
    
    // 遠いほど急激に減少
    float distanceDecay = pow(1.0f - normalizedDistance, 2.0f);
    
    // テッセレーション係数を1-64の範囲でマッピング
    float baseTessellation = lerp(MIN_TESSELLATION, MAX_TESSELLATION, distanceDecay);
    
     // カメラとパッチの垂直距離（Y軸方向）
    float verticalDistance = abs(patchCenter.y - cameraPosition.y);
    
    // 高度差が200以上の場合は大幅に削減（0.1倍）、未満なら維持（1.0倍）
    float heightMultiplier = lerp(1.0f, HEIGHT_PENALTY, step(HEIGHT_THRESHOLD, verticalDistance));

    float finalTessellation = baseTessellation * heightMultiplier;
    
    // ===== 全エッジと内部に同じ係数を適用 =====
    
    output.EdgeTess[0] = finalTessellation; // 左エッジ
    output.EdgeTess[1] = finalTessellation; // 上エッジ
    output.EdgeTess[2] = finalTessellation; // 右エッジ
    output.EdgeTess[3] = finalTessellation; // 下エッジ
    
    output.InsideTess[0] = finalTessellation; // U方向内部分割
    output.InsideTess[1] = finalTessellation; // V方向内部分割

    return output;
}

// 各制御点の出力
[domain("quad")] 
[partitioning("integer")] 
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HS_ConstantFunction")] 
HS_OUTPUT main(InputPatch<HS_INPUT, 4> patch, uint controlPointID : SV_OutputControlPointID)
{
    HS_OUTPUT output;

    // 入力をそのまま出力（制御点を加工しない場合）
    output.position = patch[controlPointID].position;
    output.uv = patch[controlPointID].uv;
   
    return output;
}