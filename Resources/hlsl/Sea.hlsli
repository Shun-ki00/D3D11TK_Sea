
// === 頂点シェーダー ===

// 入力構造体
struct VS_INPUT
{
    float4 position : SV_POSITION; // 頂点位置
    float2 uv       : TEXCOORD; // UV座標
};
// 出力構造体
struct VS_OUTPUT
{
    float4 position : SV_POSITION; // 頂点位置
    float2 uv       : TEXCOORD;    // UV座標
};


// === ハルシェーダー ===

// 入力構造体
struct HS_INPUT
{
    float4 position : SV_POSITION; // 頂点の位置
    float2 uv       : TEXCOORD;    // UV座標
};
// 出力構造体
struct HS_OUTPUT
{
    float4 position : SV_POSITION; // 頂点の位置
    float2 uv : TEXCOORD; // UV座標
};
// テッセレーション定数
struct HS_CONSTANT_OUTPUT
{
    float EdgeTess[4]   : SV_TessFactor;       // 各エッジのテッセレーション係数
    float InsideTess[2] : SV_InsideTessFactor; // 内側のテッセレーション係数
};


// === ドメインシェーダー ===

// 入力構造体
struct DS_INPUT
{
    float2 uvw : SV_DomainLocation; // テッセレーターで生成されたバリューパラメータ（UV空間）
};
// 出力構造体
struct DS_OUTPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};


// 入力：頂点シェーダーから送られてくるデータ
struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};


// 定数バッファ
cbuffer ConstBuffer : register(b0)
{
    matrix matWorld;
    matrix matView;
    matrix matProj;
    float4 TessellationFactor; 
};


cbuffer WaveParams : register(b1)
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

// 環境光定数バッファ
cbuffer AmbientLightParameters : register(b3)
{
    float3 c_ambientLightColor : packoffset(c0); // 環境光の色
    float f_ambientLightIntensity : packoffset(c0.w); // 環境光の強さ
}