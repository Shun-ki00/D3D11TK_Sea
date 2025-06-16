#include "Sea.hlsli"


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
    
    for (float i = 0; i < fnOctaves; ++i)
    {
        output += perlinNoise(uv * frequency) * amplitude;
        amplitude *= fnPersistence;
        frequency *= 2.0f;
    }
    
    return output;
}


// �g�ł��֐�
void waveUV(inout float2 uv)
{
    uv.x += TessellationFactor.y * flowVelocity; // ���ړ�
    uv.y += cos(TessellationFactor.y) * swingSpeed; // �c�h��
}


// �s�N�Z���V�F�[�_�[�{��
float4 main(PS_INPUT input) : SV_TARGET
{
    // �����_���ȃI�t�Z�b�g�𐶐�
    float2 uv = input.uv;
       
    float2 offset = perlinNoise(uv );
    uv += offset;
    
    
    //float2 offset = perlinNoise(uv);
    //uv += offset;
    
    // �I���W�i���m�C�Y
    float p = originalNoise(uv * fnUVPower);
    uv += float2(p * fnUVPath1 * cos(TessellationFactor.y), p * fnUVPath2 * sin(TessellationFactor.y));
    
    // �g�ł��֐�
    waveUV(uv);
    
    // �e�N�X�`�����T���v�����O
    float4 output = tex.Sample(sam, uv);
    
    
    return output;
}