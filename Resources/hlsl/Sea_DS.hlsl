#include "Sea.hlsli"

static const float PI = 3.1415926f;

// �g�̃p�����[�^���i�[����\����
struct WaveParam
{
    float Q;        // �X�ΌW��
    float A;        // �U��
    float WA;       // �p�U��
    float sinTheta; // �g�̊p�x
    float cosTheta; // �g�̊p�x
    float2 D;       // �����x�N�g��
};

// �g�p�����[�^�����������A�K�v�Ȏ��O�v�Z���s��
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

// �g�ɂ�钸�_�ψʂ��v�Z
float3 CalculateShift(const WaveParam p)
{
    return float3(
        p.Q * p.A * p.D.x * p.cosTheta,
        p.Q * p.A * p.D.y * p.cosTheta,
        p.A * p.sinTheta
    );
}


// �����̔g���������AXY�ʒu�Ɋ�Â��ŏI�I�Ȓ��_�̕ψʂ�Ԃ�
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


// �h���C���V�F�[�_�[�{��
[domain("quad")]
DS_OUTPUT main(
    DS_INPUT input,
    const OutputPatch<HS_OUTPUT, 4> patch,
    const HS_CONSTANT_OUTPUT patchConstants
)
{
    DS_OUTPUT output;

    float2 uv = input.uvw.xy;
    float3 worldPosition = float3(0.0f, 0.0f, 0.0f);
    float2 uvOut = float2(0.0f, 0.0f);

    // ����_���
    const int X = 2;
    const int Z = 2;

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
    
    // �g�v�Z�iXZ�x�[�X�j
    float2 xy = worldPosition.xz;
    float t = TessellationFactor.y;
    
    // �g�̌v�Z
    worldPosition = P(xy, t).xzy;
    
    // ���[���h���W�ϊ�
    float4 pos = mul(float4(worldPosition, 1.0f), matWorld);
    output.positionWS = pos;
    
    pos = mul(pos, matView);
    pos = mul(pos, matProj);

    output.position = pos;
    output.uv = uvOut;

    return output;
}