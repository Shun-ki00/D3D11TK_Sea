#include "Sea.hlsli"

static const float MAX_TESSELLATION_RANGE = 1100.0f;

static const float MIN_TESSELLATION = 1.0f;
static const float MAX_TESSELLATION = 10.0f;

static const float HEIGHT_THRESHOLD = 200.0f;
static const float HEIGHT_PENALTY = 0.1f;


// �n���V�F�[�_�[�{��
HS_CONSTANT_OUTPUT HS_ConstantFunction(InputPatch<HS_INPUT, 4> patch, uint patchID : SV_PrimitiveID)
{
    HS_CONSTANT_OUTPUT output;
    
    // �p�b�`�̃��[���h���W�ϊ�
    float3 worldPatch1 = mul(patch[0].position, matWorld);
    float3 worldPatch2 = mul(patch[1].position, matWorld);
    float3 worldPatch3 = mul(patch[2].position, matWorld);
    
    // �p�b�`���S���W�����[���h���W�Ōv�Z
    float3 patchCenter = (worldPatch1 + worldPatch2 + worldPatch3) / 3.0f;
    
    // XZ�������擾
    float2 cameraXZ = cameraPosition.xz;
    float2 patchXZ = patchCenter.xz;
    float horizontalDistance = distance(cameraXZ, patchXZ);
    
    // ������0-1�ɐ��K��
    float normalizedDistance = saturate(horizontalDistance / MAX_TESSELLATION_RANGE);
    
    // �����قǋ}���Ɍ���
    float distanceDecay = pow(1.0f - normalizedDistance, 2.0f);
    
    // �e�b�Z���[�V�����W����1-64�͈̔͂Ń}�b�s���O
    float baseTessellation = lerp(MIN_TESSELLATION, MAX_TESSELLATION, distanceDecay);
    
     // �J�����ƃp�b�`�̐��������iY�������j
    float verticalDistance = abs(patchCenter.y - cameraPosition.y);
    
    // ���x����200�ȏ�̏ꍇ�͑啝�ɍ팸�i0.1�{�j�A�����Ȃ�ێ��i1.0�{�j
    float heightMultiplier = lerp(1.0f, HEIGHT_PENALTY, step(HEIGHT_THRESHOLD, verticalDistance));

    float finalTessellation = baseTessellation * heightMultiplier;
    
    // ===== �S�G�b�W�Ɠ����ɓ����W����K�p =====
    
    output.EdgeTess[0] = finalTessellation; // ���G�b�W
    output.EdgeTess[1] = finalTessellation; // ��G�b�W
    output.EdgeTess[2] = finalTessellation; // �E�G�b�W
    output.EdgeTess[3] = finalTessellation; // ���G�b�W
    
    output.InsideTess[0] = finalTessellation; // U������������
    output.InsideTess[1] = finalTessellation; // V������������

    return output;
}

// �e����_�̏o��
[domain("quad")] 
[partitioning("integer")] 
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("HS_ConstantFunction")] 
HS_OUTPUT main(InputPatch<HS_INPUT, 4> patch, uint controlPointID : SV_OutputControlPointID)
{
    HS_OUTPUT output;

    // ���͂����̂܂܏o�́i����_�����H���Ȃ��ꍇ�j
    output.position = patch[controlPointID].position;
    output.uv = patch[controlPointID].uv;
   
    return output;
}