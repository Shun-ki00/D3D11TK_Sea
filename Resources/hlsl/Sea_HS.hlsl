#include "Sea.hlsli"


// ハルシェーダー本体
HS_CONSTANT_OUTPUT HS_ConstantFunction(InputPatch<HS_INPUT, 32> patch, uint patchID : SV_PrimitiveID)
{
    
    HS_CONSTANT_OUTPUT output;

    // エッジの細分化レベルを設定
    output.EdgeTess[0] = TessellationFactor.x; // 左
    output.EdgeTess[1] = TessellationFactor.x; // 上
    output.EdgeTess[2] = TessellationFactor.x; // 右
    output.EdgeTess[3] = TessellationFactor.x; // 下

    // 内部の細分化レベル（U方向とV方向）
    output.InsideTess[0] = TessellationFactor.x; // U方向
    output.InsideTess[1] = TessellationFactor.x; // V方向

  
    return output;
}



// 各制御点の出力
[domain("quad")] // パッチの形状（"tri"は三角形）"quad"（四角形）"isoline"（線状）
[partitioning("integer")] // 細分化の方式（整数テッセレーション）"fractional_odd"（奇数分割）"fractional_even"（偶数分割）
[outputtopology("triangle_cw")] // 出力トポロジー（時計回りの三角形）任意の正の整数値（通常は3（三角形）または4（四角形））
[outputcontrolpoints(32)] // 出力する制御点の数（3点パッチ）
[patchconstantfunc("HS_ConstantFunction")] // テッセレーション定数計算関数の指定
HS_OUTPUT main(InputPatch<HS_INPUT, 32> patch, uint controlPointID : SV_OutputControlPointID)
{
    HS_OUTPUT output;

    // 入力をそのまま出力（制御点を加工しない場合）
    output.position = patch[controlPointID].position;
    output.uv = patch[controlPointID].uv;

    return output;
}