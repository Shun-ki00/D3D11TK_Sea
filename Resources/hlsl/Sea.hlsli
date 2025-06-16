
// === ���_�V�F�[�_�[ ===

// ���͍\����
struct VS_INPUT
{
    float4 position : SV_POSITION; // ���_�ʒu
    float2 uv       : TEXCOORD; // UV���W
};
// �o�͍\����
struct VS_OUTPUT
{
    float4 position : SV_POSITION; // ���_�ʒu
    float2 uv       : TEXCOORD;    // UV���W
};


// === �n���V�F�[�_�[ ===

// ���͍\����
struct HS_INPUT
{
    float4 position : SV_POSITION; // ���_�̈ʒu
    float2 uv       : TEXCOORD;    // UV���W
};
// �o�͍\����
struct HS_OUTPUT
{
    float4 position : SV_POSITION; // ���_�̈ʒu
    float2 uv : TEXCOORD; // UV���W
};
// �e�b�Z���[�V�����萔
struct HS_CONSTANT_OUTPUT
{
    float EdgeTess[4]   : SV_TessFactor;       // �e�G�b�W�̃e�b�Z���[�V�����W��
    float InsideTess[2] : SV_InsideTessFactor; // �����̃e�b�Z���[�V�����W��
};


// === �h���C���V�F�[�_�[ ===

// ���͍\����
struct DS_INPUT
{
    float2 uvw : SV_DomainLocation; // �e�b�Z���[�^�[�Ő������ꂽ�o�����[�p�����[�^�iUV��ԁj
};
// �o�͍\����
struct DS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    
    float4 positionWS : POSITION;
};


// ���́F���_�V�F�[�_�[���瑗���Ă���f�[�^
struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    
    float4 positionWS : POSITION;
};


// �ϊ��p�f�[�^
cbuffer TransformConstBuffer : register(b0)
{
    matrix matWorld;
    matrix matView;
    matrix matProj;
    float4 cameraPosition;
    float4 TessellationFactor; 
};

// �C�̃m�C�Y�f�[�^
cbuffer SeaNoiseConstBuffer : register(b1)
{
    float flowVelocity  : packoffset(c0.x);
    float swingSpeed    : packoffset(c0.y);
    float fnUVPath1     : packoffset(c0.z);
    float fnUVPath2     : packoffset(c0.w);
    float fnUVPower     : packoffset(c1.x);
    float fnOctaves     : packoffset(c1.y);
    float fnPersistence : packoffset(c1.z);
    float pading        : packoffset(c1.w);
}

// �Q���X�g�i�g�̃f�[�^
cbuffer GerstnerWaveConstBuffer : register(b2)
{   
    // Wave1
    float active1     : packoffset(c0.x);
    float direction1X : packoffset(c0.y);
    float direction1Z : packoffset(c0.z);
    float amplitude1  : packoffset(c0.w);

    float waveLength1 : packoffset(c1.x);
    float speed1      : packoffset(c1.y);
    float qRatio1     : packoffset(c1.z);
    float pad1        : packoffset(c1.w);

    // Wave2
    float active2     : packoffset(c2.x);
    float direction2X : packoffset(c2.y);
    float direction2Z : packoffset(c2.z);
    float amplitude2  : packoffset(c2.w);

    float waveLength2 : packoffset(c3.x);
    float speed2      : packoffset(c3.y);
    float qRatio2     : packoffset(c3.z);
    float pad2        : packoffset(c3.w);

    // Wave3
    float active3     : packoffset(c4.x);
    float direction3X : packoffset(c4.y);
    float direction3Z : packoffset(c4.z);
    float amplitude3  : packoffset(c4.w);

    float waveLength3 : packoffset(c5.x);
    float speed3      : packoffset(c5.y);
    float qRatio3     : packoffset(c5.z);
    float pad3        : packoffset(c5.w);
}