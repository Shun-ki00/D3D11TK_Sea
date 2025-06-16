#include "Sea.hlsli"


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
    
    for (float i = 0; i < fnOctaves; ++i)
    {
        output += perlinNoise(uv * frequency) * amplitude;
        amplitude *= fnPersistence;
        frequency *= 2.0f;
    }
    
    return output;
}


// 波打ち関数
void waveUV(inout float2 uv)
{
    uv.x += TessellationFactor.y * flowVelocity; // 横移動
    uv.y += cos(TessellationFactor.y) * swingSpeed; // 縦揺れ
}


// ピクセルシェーダー本体
float4 main(PS_INPUT input) : SV_TARGET
{
    // ランダムなオフセットを生成
    float2 uv = input.uv;
       
    float2 offset = perlinNoise(uv );
    uv += offset;
    
    
    //float2 offset = perlinNoise(uv);
    //uv += offset;
    
    // オリジナルノイズ
    float p = originalNoise(uv * fnUVPower);
    uv += float2(p * fnUVPath1 * cos(TessellationFactor.y), p * fnUVPath2 * sin(TessellationFactor.y));
    
    // 波打ち関数
    waveUV(uv);
    
    // テクスチャをサンプリング
    float4 output = tex.Sample(sam, uv);
    
    
    return output;
}