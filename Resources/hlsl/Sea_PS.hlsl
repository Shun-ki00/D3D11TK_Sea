#include "Sea.hlsli"

#define FLOW_VELOCITY 0.1f      // ������
#define SWING_SPEED 0.01f       // ���E�ɐU����
#define FN_UV_PATH1 0.08f       // �m�C�Y�̌��ʂɏ�Z����l
#define FN_UV_PATH2 0.06f       // �m�C�Y�̌��ʂɏ�Z����l
#define FN_UV_POWER 25          // uv�̃T�C�Y
#define FN_OCTAVES 10.0f        // �I�N�^�[�u
#define FN_PERSISTENCE 0.6f     // �����x
#define RANDOM_OFFSET 0.01f     // �����_���I�t�Z�b�g
// �^�C���T�C�Y�i��F10x10�j
#define TILE_COUNT 1

// �e�N�X�`��
Texture2D tex : register(t0);
// �T���v���[
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
    // uv���W�𐮐����Ə������ɕ���
    float2 p = floor(uv);
    float2 f = frac(uv);
    
    // f�̕�Ԋ֐����v�Z
    float2 u = f * f * (3.0 - 2.0 * f);
    
    // �אڂ���4�̐������W�ɑ΂��ă����_���l���擾����
    float2 v00 = random2(p + float2(0, 0));
    float2 v10 = random2(p + float2(1, 0));
    float2 v01 = random2(p + float2(0, 1));
    float2 v11 = random2(p + float2(1, 1));
    
    // ��ԂƐ��`��Ԃ��g�p���ăp�[�����m�C�Y�l���v�Z
    return lerp(
    lerp(dot(v00, f - float2(0, 0)), dot(v10, f - float2(1, 0)), u.x),
    lerp(dot(v01, f - float2(0, 1)), dot(v11, f - float2(1, 1)), u.x),
    u.y);
}

// �I���W�i���m�C�Y�𐶐�����֐�
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


// �g�ł��֐�
void waveUV(inout float2 uv)
{
    uv.x += TessellationFactor.y * FLOW_VELOCITY; // ���ړ�
    uv.y += cos(TessellationFactor.y) * SWING_SPEED; // �c�h��
}

float hash21(float2 p)
{
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return frac(p.x * p.y);
}


// �s�N�Z���V�F�[�_�[�{��
float4 main(PS_INPUT input) : SV_TARGET
{
   
    float2 uv = input.uv;

    
  // �^�C�����W
    float2 tileUV = uv * TILE_COUNT;
    int2 tileIndex = int2(floor(tileUV));

    // �e�^�C�����ł̃��[�J��UV�i0^1�j
    float2 localUV = frac(tileUV);

    // �^�C�����W�Ɋ�Â��� Y�����������_�����]
    float2 offsetSeed = float2(tileIndex);

    // Y�����]�F50%�̊m���Ŕ��]
    bool flipY = hash21(offsetSeed + 345.678) < 0.5;

    if (flipY)
        localUV.y = 1.0 - localUV.y;

    // �����_���I�t�Z�b�g����
    float2 offset = float2(
    hash21(offsetSeed),
    hash21(offsetSeed + 123.456)
    );

    // UV�ɃI�t�Z�b�g��������
    float2 uvWithOffset = localUV + offset * 0.1;

    // �m�C�Y���͂Ƀ^�C���I�t�Z�b�g�������āA�e�^�C���������_����
    float2 noiseUV = uvWithOffset * FN_UV_POWER + offset * 10.0;
    float p = originalNoise(noiseUV);

    // UV�ό`
    uvWithOffset += float2(
    p * FN_UV_PATH1 * cos(TessellationFactor.y),
    p * FN_UV_PATH2 * sin(TessellationFactor.y)
    );

    // �g�ł�
    waveUV(uvWithOffset);

    // �e�N�X�`���T���v�����O
    float4 output = tex.Sample(sam, uvWithOffset);
    
    output.rgb *= 0.8;
    
    return output;
}