#include "Sea.hlsli"


// �n���V�F�[�_�[�{��
HS_CONSTANT_OUTPUT HS_ConstantFunction(InputPatch<HS_INPUT, 32> patch, uint patchID : SV_PrimitiveID)
{
    HS_CONSTANT_OUTPUT output;
    
    float3 worldPatch1 = mul(patch[0].position, matWorld);
    float3 worldPatch2 = mul(patch[1].position, matWorld);
    float3 worldPatch3 = mul(patch[2].position, matWorld);
    
    // �p�b�`���S���W�����[���h���W�Ōv�Z
    float3 patchCenter = (worldPatch1 + worldPatch2 + worldPatch3) / 3.0f;

   // XZ�������擾
    float2 camXZ = cameraPosition.xz;
    float2 patchXZ = patchCenter.xz;
    float distXZ = distance(camXZ, patchXZ);

    // �p�����[�^����
    float maxRange = 1100.0f; // �����J�n?�ő�͈�
    float normalizedDist = saturate(distXZ / maxRange);

    // ����`�����i�w���֐��^�j
    float decay = pow(1.0f - normalizedDist, 2.0f); // �񎟌���

    // �o�͔͈� 64 �� 1 �փ}�b�s���O
    float tessFactor = lerp(1.0f, 64.0f, decay);
    
    // 2. �������ɂ��␳�i200�ȏ� �� 1/2�A���� �� 1.0�j
    float yDelta = abs(patchCenter.y - cameraPosition.y);
    float heightMask = lerp(1.0f, 0.1f, step(200.0f, yDelta)); // 200�ȏ�� 0.5

    // 3. ���ʂɓK�p
    tessFactor *= heightMask;
    
    // �G�b�W�̍ו������x����ݒ�
    output.EdgeTess[0] = tessFactor; // ��
    output.EdgeTess[1] = tessFactor; // ��
    output.EdgeTess[2] = tessFactor; // �E
    output.EdgeTess[3] = tessFactor; // ��

    // �����̍ו������x���iU������V�����j
    output.InsideTess[0] = tessFactor; // U����
    output.InsideTess[1] = tessFactor; // V����

  
    return output;
}



// �e����_�̏o��
[domain("quad")] // �p�b�`�̌`��i"tri"�͎O�p�`�j"quad"�i�l�p�`�j"isoline"�i����j
[partitioning("integer")] // �ו����̕����i�����e�b�Z���[�V�����j"fractional_odd"�i������j"fractional_even"�i���������j
[outputtopology("triangle_cw")] // �o�̓g�|���W�[�i���v���̎O�p�`�j�C�ӂ̐��̐����l�i�ʏ��3�i�O�p�`�j�܂���4�i�l�p�`�j�j
[outputcontrolpoints(32)] // �o�͂��鐧��_�̐��i3�_�p�b�`�j
[patchconstantfunc("HS_ConstantFunction")] // �e�b�Z���[�V�����萔�v�Z�֐��̎w��
HS_OUTPUT main(InputPatch<HS_INPUT, 32> patch, uint controlPointID : SV_OutputControlPointID)
{
    HS_OUTPUT output;

    // ���͂����̂܂܏o�́i����_�����H���Ȃ��ꍇ�j
    output.position = patch[controlPointID].position;
    output.uv = patch[controlPointID].uv;

    return output;
}