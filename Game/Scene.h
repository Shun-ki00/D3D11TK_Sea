#pragma once
#include <future>
#include "Framework/DebugCamera.h"
#include "Framework/ConstantBuffer.h"

class CommonResources;
class DebugCamera;

class Scene
{
public:

	// インスタンスデータの数
	static const int WIDTH; 
	static const int HEIGHT;
	// タイルサイズ
	static const float TILE_SIZE;

private:

	// 変換用定数バッファ
	struct TransformConstBuffer
	{
		DirectX::SimpleMath::Matrix		matWorld;
		DirectX::SimpleMath::Matrix		matView;
		DirectX::SimpleMath::Matrix		matProj;
		DirectX::SimpleMath::Vector4    cameraPosition;
		DirectX::SimpleMath::Vector4    TessellationFactor;
	};

	// インスタンスデータ
	struct InstanceData
	{
		DirectX::SimpleMath::Vector3 worldMatrix;
	};

	// 海のノイズ用の定数バッファ
	struct SeaNoiseConstBuffer
	{
		float flowVelocity;    // 流れる力
		float swingSpeed;      // 左右の振れ
		float fnUVPath1;       // ノイズの影響 (x軸)
		float fnUVPath2;       // ノイズの影響 (y軸)

		float fnUVPower;       // UVスケール
		float fnOctaves;       // ノイズのオクターブ数
		float fnPersistence;   // ノイズ持続度
		float padding;         // パディング
	};

	// ゲルストナ波の定数バッファ
	struct GerstnerWaveConstBuffer
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

	// 頂点の生成
	void CreateVertices();
	// インデクスの生成
	void CreateIndices();
	// インスタンスデータの作成
	void CreateInstanceData();

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
	// インデックスバッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	// インスタンスバッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_instanceStructuredBuffer;

	// 定数バッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

	// 海のノイズ用の定数バッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_seaNoiseConstBuffer;
	// ゲルストナ波のバッファ
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_gerstnerWaveConstBuffer;

	// テクスチャ
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	// インスタンスのシェーダーリソースビュー
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_instanceSRV;

	// 板ポリの頂点群
	std::vector<DirectX::VertexPositionTexture> m_vertices;
	// 板ポリのインデックスバッファ
	std::vector<uint16_t> m_indices;
	// インスタンスデータ
	std::vector<InstanceData> m_instanceData;

	// 海のノイズ用の定数バッファ
	SeaNoiseConstBuffer m_seaNoiseConstBufferData;
	// ゲルストナ波のバッファデータ
	GerstnerWaveConstBuffer m_gerstnerWaveConstBufferData;
	// テッセレーション係数
	float m_tessellationIndex;
	// ラスタライザーモード設定
	bool m_isWireframe;

	// 経過時間
	float m_time;



};