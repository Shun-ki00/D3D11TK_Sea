#include "Sea.hlsli"

static const float PI = 3.1415926f;

//------------------------------------------
// 構造体（Unity の Param に対応）
struct WaveParam
{
    float Q;
    float A;
    float WA;
    float sinTheta;
    float cosTheta;
    float2 D;
};

//------------------------------------------
// 波パラメータ初期化（角度と補助値計算）
WaveParam InitWaveParam(float2 xy, float QRatio, float A, float2 D, float t, float L, float S)
{
    WaveParam param = (WaveParam) 0;

    if (A <= 0.0001f || L <= 0.0001f || all(D == 0))
        return param;

    float w = 2.0f * PI / L;
    float phi = S * w;

    param.Q = (1.0f / (w * A)) * QRatio;
    param.A = A;
    param.WA = w * A;
    param.D = normalize(D);

    float theta = w * dot(param.D, xy) + phi * t;
    param.sinTheta = sin(theta);
    param.cosTheta = cos(theta);

    return param;
}

//------------------------------------------
// 波形変位（Shift計算）
float3 CalculateShift(const WaveParam p)
{
    return float3(
        p.Q * p.A * p.D.x * p.cosTheta,
        p.Q * p.A * p.D.y * p.cosTheta,
        p.A * p.sinTheta
    );
}

//------------------------------------------
// 頂点変位
float3 P(float2 xy, float t)
{
    float2 D1 = float2(direction1X, direction1Z);
    float2 D2 = float2(direction2X, direction2Z);
    float2 D3 = float2(direction3X, direction3Z);

    WaveParam p1 = InitWaveParam(xy, qRatio1, amplitude1, D1, t, waveLength1, speed1);
    WaveParam p2 = InitWaveParam(xy, qRatio2, amplitude2, D2, t, waveLength2, speed2);
    WaveParam p3 = InitWaveParam(xy, qRatio3, amplitude3, D3, t, waveLength3, speed3);

    return float3(xy, 0.0f)
        + CalculateShift(p1) * active1
        + CalculateShift(p2) * active2
        + CalculateShift(p3) * active3;
}


//------------------------------------------
// ドメインシェーダ本体
[domain("quad")]
DS_OUTPUT main(
    DS_INPUT input,
    const OutputPatch<HS_OUTPUT, 32> patch,
    const HS_CONSTANT_OUTPUT patchConstants
)
{
    DS_OUTPUT output;

    float2 uv = input.uvw.xy;
    float3 worldPosition = float3(0.0f, 0.0f, 0.0f);
    float2 uvOut = float2(0.0f, 0.0f);

    // 制御点補間（8×4）
    const int X = 8;
    const int Z = 4;

    for (int z = 0; z < Z; ++z)
    {
        float wz = 1.0f - abs(uv.y * (Z - 1) - z);
        for (int x = 0; x < X; ++x)
        {
            float wx = 1.0f - abs(uv.x * (X - 1) - x);
            float w = saturate(wx) * saturate(wz);

            int idx = z * X + x;
            worldPosition += patch[idx].position * w;
            uvOut += patch[idx].uv * w;
        }
    }

    // 波計算（XZベース）
    float2 xy = worldPosition.xz;
    float t = TessellationFactor.y;

    WaveParam p1 = InitWaveParam(xy, qRatio1, amplitude1, float2(direction1X, direction1Z), t, waveLength1, speed1);
    WaveParam p2 = InitWaveParam(xy, qRatio2, amplitude2, float2(direction2X, direction2Z), t, waveLength2, speed2);
    WaveParam p3 = InitWaveParam(xy, qRatio3, amplitude3, float2(direction3X, direction3Z), t, waveLength3, speed3);

    worldPosition = P(xy, t).xzy;

    // ワールド→クリップ変換
    float4 pos = mul(float4(worldPosition, 1.0f), matWorld);
    pos = mul(pos, matView);
    pos = mul(pos, matProj);

    output.position = pos;
    output.uv = uvOut;

    return output;
}