#include "Sea.hlsli"

#define FLOW_VELOCITY 0.1f      // 流れる力
#define SWING_SPEED 0.01f       // 左右に振れる力
#define FN_UV_PATH1 0.08f       // ノイズの結果に乗算する値
#define FN_UV_PATH2 0.06f       // ノイズの結果に乗算する値
#define FN_UV_POWER 25          // uvのサイズ
#define FN_OCTAVES 10.0f        // オクターブ
#define FN_PERSISTENCE 0.6f     // 持続度
#define RANDOM_OFFSET 0.01f     // ランダムオフセット
// タイルサイズ（例：10x10）
#define TILE_COUNT 1

// テクスチャ
Texture2D tex : register(t0);
// サンプラー
SamplerState sam : register(s0);

float random(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898f, 78.233f))) * 43758.5453f);
}

float2 random2(float2 st)
{
    st = float2(dot(st, float2(127.1f, 311.7f)),
                dot(st, float2(269.5f, 183.3f)));
    return -1.0f + 2.0f * frac(sin(st) * 43758.5453f);
}

float perlinNoise(float2 uv)
{
    // uv座標を整数部と少数部に分割
    float2 p = floor(uv);
    float2 f = frac(uv);
    
    // fの補間関数を計算
    float2 u = f * f * (3.0 - 2.0 * f);
    
    // 隣接する4つの整数座標に対してランダム値を取得する
    float2 v00 = random2(p + float2(0, 0));
    float2 v10 = random2(p + float2(1, 0));
    float2 v01 = random2(p + float2(0, 1));
    float2 v11 = random2(p + float2(1, 1));
    
    // 補間と線形補間を使用してパーリンノイズ値を計算
    return lerp(
    lerp(dot(v00, f - float2(0, 0)), dot(v10, f - float2(1, 0)), u.x),
    lerp(dot(v01, f - float2(0, 1)), dot(v11, f - float2(1, 1)), u.x),
    u.y);
}

// オリジナルノイズを生成する関数
float originalNoise(float2 uv)
{
    float output = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    
    for (float i = 0; i < FN_OCTAVES; ++i)
    {
        output += perlinNoise(uv * frequency) * amplitude;
        amplitude *= FN_PERSISTENCE;
        frequency *= 2.0f;
    }
    
    return output;
}


// 波打ち関数
void waveUV(inout float2 uv)
{
    uv.x += TessellationFactor.y * FLOW_VELOCITY; // 横移動
    uv.y += cos(TessellationFactor.y) * SWING_SPEED; // 縦揺れ
}

float hash21(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return frac(p.x * p.y);
}


// ピクセルシェーダー本体
float4 main(PS_INPUT input) : SV_TARGET
{
   
    float2 uv = input.uv;

    
  // タイル座標
    float2 tileUV = uv * TILE_COUNT;
    int2 tileIndex = int2(floor(tileUV));

    // 各タイル内でのローカルUV（0^1）
    float2 localUV = frac(tileUV);

    // タイル座標に基づいて Y軸だけランダム反転
    float2 offsetSeed = float2(tileIndex);

    // Y軸反転：50%の確率で反転
    bool flipY = hash21(offsetSeed + 345.678) < 0.5;

    if (flipY)
        localUV.y = 1.0 - localUV.y;

    // ランダムオフセット生成
    float2 offset = float2(
    hash21(offsetSeed),
    hash21(offsetSeed + 123.456)
    );

    // UVにオフセットを加える
    float2 uvWithOffset = localUV + offset * 0.1;

    // ノイズ入力にタイルオフセットを加えて、各タイルをランダム化
    float2 noiseUV = uvWithOffset * FN_UV_POWER + offset * 10.0;
    float p = originalNoise(noiseUV);

    // UV変形
    uvWithOffset += float2(
    p * FN_UV_PATH1 * cos(TessellationFactor.y),
    p * FN_UV_PATH2 * sin(TessellationFactor.y)
    );

    // 波打ち
    waveUV(uvWithOffset);

    // テクスチャサンプリング
    float4 output = tex.Sample(sam, uvWithOffset);
    
    output.rgb *= 0.8;
    
    return output;
}