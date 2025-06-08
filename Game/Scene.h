#pragma once
#include <future>
#include "Framework/DebugCamera.h"
#include "Framework/ConstantBuffer.h"

class CommonResources;
class DebugCamera;

class Scene
{
private:

	// 定数バッファ
	struct ConstBuffer
	{
		DirectX::SimpleMath::Matrix		matWorld;
		DirectX::SimpleMath::Matrix		matView;
		DirectX::SimpleMath::Matrix		matProj;
		DirectX::SimpleMath::Vector4    TessellationFactor;
	};

	// 波のパラメータ
	struct WaveParams
	{
		// Wave1
		float active1, direction1X, direction1Z, amplitude1;
		float waveLength1,speed1, qRatio1, pad1;

		// Wave2
		float active2, direction2X, direction2Z, amplitude2;
		float waveLength2, speed2, qRatio2, pad2;

		// Wave3
		float active3, direction3X, direction3Z, amplitude3;
		float waveLength3, speed3, qRatio3, pad3;
	};


public:

	// コンストラクタ
	Scene();
	// デストラクタ
	~Scene() = default;

public:

	// 初期化処理
	void Initialize();
	// 更新処理
	void Update(const float& elapsedTime);
	// 描画処理
	void Render();
	// 終了処理
	void Finalize();

	// デバッグウィンドウ
	void ImguiWindow();

private:

	// シェーダーやバッファの作成
	void CreateShaderWithBuffers();
	
private:

	// 共有リソース
	CommonResources* m_commonResources;

	// デバッグカメラ
	std::unique_ptr<DebugCamera> m_camera;

	// デバイス
	ID3D11Device1* m_device;
	// コンテキスト
	ID3D11DeviceContext1* m_context;
	// コモンステート
	DirectX::CommonStates* m_commonStates;

	// 入力レイアウト
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	// 頂点シェーダー
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	// ハルシェーダー
	Microsoft::WRL::ComPtr<ID3D11HullShader> m_hullShader;
	// ドメインシェーダー
	Microsoft::WRL::ComPtr<ID3D11DomainShader> m_domainShader;
	// ピクセルシェーダー
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;

	// 頂点バッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
	// 波パラメータ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_waveConstantBuffer;

	// テクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;

	// 波のパラメータデータ
	WaveParams m_waveParams;
	// テッセレーション係数
	float m_tessellationIndex;
	// ラスタライザーモード設定
	bool m_isWireframe;

	// 経過時間
	float m_time;



};