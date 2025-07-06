#include "pch.h"
#include "Game/Scene.h"
#include "Framework/CommonResources.h"
#include "Framework/DebugCamera.h"
#include "Framework/Microsoft/ReadData.h"
#include <WICTextureLoader.h>

/// <summary>
/// コンストラクタ
/// </summary>
Scene::Scene()
{
	// インスタンスを取得する
	m_commonResources = CommonResources::GetInstance();
	m_device          = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDevice();
	m_context         = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();
	m_commonStates    = CommonResources::GetInstance()->GetCommonStates();
}

/// <summary>
/// 初期化処理
/// </summary>
void Scene::Initialize()
{
	// カメラの作成
	m_camera = std::make_unique<DebugCamera>();
	m_camera->Initialize(1280, 720);

	// テクスチャのロード
	DirectX::CreateWICTextureFromFile(
		m_device, L"Resources/Textures/Sea.png", nullptr, m_texture.ReleaseAndGetAddressOf());

	// シェーダー、バッファの作成
	this->CreateShaderWithBuffers();

	this->CreateVertices();
	this->CreateInstanceData();

	// 頂点を生成
	//std::thread vertices(&Scene::CreateVertices, this);
	//// インデックスの生成
	//std::thread indices(&Scene::CreateIndices , this);

	//// 生成が終わるまで待つ
	//vertices.join();
	//indices.join();

	// 初期化
	m_time = 0.0f;
	m_tessellationIndex = 1.0f;
	m_isWireframe = false;

}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="elapsedTime">経過時間</param>
void Scene::Update(const float& elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime);

	// カメラの更新処理
	m_camera->Update();
	m_commonResources->SetViewMatrix(m_camera->GetViewMatrix());

	// 定数バッファの更新処理
	m_context->UpdateSubresource(m_seaNoiseConstBuffer.Get(), 0, nullptr, &m_seaNoiseConstBufferData, 0, 0);
	m_context->UpdateSubresource(m_gerstnerWaveConstBuffer.Get(), 0, nullptr, &m_gerstnerWaveConstBufferData, 0, 0);
	

}

/// <summary>
/// 描画処理
/// </summary>
void Scene::Render() 
{
	// タイマーの更新
	float elapsedTime = (float)m_commonResources->GetStepTimer()->GetElapsedSeconds();
	m_time += elapsedTime;

	// ワールド行列作成
	DirectX::SimpleMath::Matrix world =
		DirectX::SimpleMath::Matrix::CreateTranslation(DirectX::SimpleMath::Vector3::Zero);

	//	シェーダーに渡す追加のバッファを作成する。
	TransformConstBuffer cbuff;
	cbuff.matView = m_camera->GetViewMatrix().Transpose();
	cbuff.matProj = m_commonResources->GetProjectionMatrix().Transpose();
	cbuff.matWorld = world.Transpose();
	cbuff.cameraPosition = { m_camera->GetEyePosition().x,m_camera->GetEyePosition().y,m_camera->GetEyePosition().z ,0.0f };
	cbuff.TessellationFactor = DirectX::SimpleMath::Vector4(m_tessellationIndex, m_time, 2.0f, 0.0f);

	//	受け渡し用バッファの内容更新(ConstBufferからID3D11Bufferへの変換）
	m_context->UpdateSubresource(m_constantBuffer.Get(), 0, NULL, &cbuff, 0, 0);

	// 入力レイアウトを設定
	m_context->IASetInputLayout(m_inputLayout.Get());

	// 頂点バッファを設定
	ID3D11Buffer* buffers[] = { m_vertexBuffer.Get() };
	UINT stride[] = { sizeof(DirectX::VertexPositionTexture) };
	UINT offset[] = { 0 };
	m_context->IASetVertexBuffers(0, 1, buffers, stride, offset);

	// プリミティブトポロジー（三角形リスト）
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	//	シェーダーにバッファを渡す
	ID3D11Buffer* cb[3] = { m_constantBuffer.Get() , m_seaNoiseConstBuffer.Get() , m_gerstnerWaveConstBuffer.Get()};
	// スロット1から3に一括バインド
	m_context->VSSetConstantBuffers(0, 3, cb);
	m_context->HSSetConstantBuffers(0, 3, cb);
	m_context->DSSetConstantBuffers(0, 3, cb);
	m_context->PSSetConstantBuffers(0, 3, cb);

	// サンプラーステートをピクセルシェーダーに設定
	ID3D11SamplerState* sampler[1] = { m_commonStates->LinearWrap() };
	m_context->PSSetSamplers(0, 1, sampler);

	// ブレンドステートを設定 (半透明描画用)
	m_context->OMSetBlendState(m_commonStates->AlphaBlend(), nullptr, 0xFFFFFFFF);

	//	深度バッファに書き込み参照する
	m_context->OMSetDepthStencilState(m_commonStates->DepthDefault(), 0);

	// ラスタライザーステートの設定
	if(m_isWireframe) m_context->RSSetState(m_commonStates->Wireframe());
	else m_context->RSSetState(m_commonStates->CullCounterClockwise());
	

	//	シェーダをセットする
	m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_context->HSSetShader(m_hullShader.Get(), nullptr, 0);
	m_context->DSSetShader(m_domainShader.Get(), nullptr, 0);
	m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	// テクスチャの設定
	ID3D11ShaderResourceView* tex[1] = { m_texture.Get() };
	m_context->PSSetShaderResources(0, 1, tex);

	// StructuredBuffer: インスタンスのワールド行列
	ID3D11ShaderResourceView* srvs[] = { m_instanceSRV.Get() };
	m_context->VSSetShaderResources(1, 1, srvs);

	// 描画呼び出し（インスタンス描画）
	UINT instanceCount = static_cast<UINT>(m_instanceData.size());
	m_context->DrawInstanced(4, instanceCount, 0, 0);

	//	シェーダの登録を解除
	m_context->VSSetShader(nullptr, nullptr, 0);
	m_context->HSSetShader(nullptr, nullptr, 0);
	m_context->DSSetShader(nullptr, nullptr, 0);
	m_context->PSSetShader(nullptr, nullptr, 0);

	// テクスチャリソースを解放
	ID3D11ShaderResourceView* nullsrv[] = { nullptr };
	m_context->PSSetShaderResources(0, 1, nullsrv);
	m_context->VSSetShaderResources(1, 1, nullsrv);

}


void Scene::ImguiWindow()
{

	if (ImGui::Begin("Wave Settings"))
	{
		ImGui::Text("Tessellation");
		ImGui::InputFloat("Tessellation Factor", &m_tessellationIndex, 1.0f, 1.0f);

		ImGui::Checkbox("WireFrame", &m_isWireframe);

		// チェックボックス用
		static bool active1 = true;
		static bool active2 = true;
		static bool active3 = true;

		ImGui::Separator();
		ImGui::Text("Wave 1");
		ImGui::Checkbox("Active##1", &active1);
		ImGui::InputFloat("Direction X##1", &m_gerstnerWaveConstBufferData.direction1X);
		ImGui::InputFloat("Direction Z##1", &m_gerstnerWaveConstBufferData.direction1Z);
		ImGui::InputFloat("Amplitude##1", &m_gerstnerWaveConstBufferData.amplitude1);
		ImGui::InputFloat("Wave Length##1", &m_gerstnerWaveConstBufferData.waveLength1);
		ImGui::InputFloat("Speed##1", &m_gerstnerWaveConstBufferData.speed1);
		ImGui::InputFloat("Q Ratio##1", &m_gerstnerWaveConstBufferData.qRatio1);

		ImGui::Separator();
		ImGui::Text("Wave 2");
		ImGui::Checkbox("Active##2", &active2);
		ImGui::InputFloat("Direction X##2", &m_gerstnerWaveConstBufferData.direction2X);
		ImGui::InputFloat("Direction Z##2", &m_gerstnerWaveConstBufferData.direction2Z);
		ImGui::InputFloat("Amplitude##2", &m_gerstnerWaveConstBufferData.amplitude2);
		ImGui::InputFloat("Wave Length##2", &m_gerstnerWaveConstBufferData.waveLength2);
		ImGui::InputFloat("Speed##2", &m_gerstnerWaveConstBufferData.speed2);
		ImGui::InputFloat("Q Ratio##2", &m_gerstnerWaveConstBufferData.qRatio2);

		ImGui::Separator();
		ImGui::Text("Wave 3");
		ImGui::Checkbox("Active##3", &active3);
		ImGui::InputFloat("Direction X##3", &m_gerstnerWaveConstBufferData.direction3X);
		ImGui::InputFloat("Direction Z##3", &m_gerstnerWaveConstBufferData.direction3Z);
		ImGui::InputFloat("Amplitude##3", &m_gerstnerWaveConstBufferData.amplitude3);
		ImGui::InputFloat("Wave Length##3", &m_gerstnerWaveConstBufferData.waveLength3);
		ImGui::InputFloat("Speed##3", &m_gerstnerWaveConstBufferData.speed3);
		ImGui::InputFloat("Q Ratio##3", &m_gerstnerWaveConstBufferData.qRatio3);

		// 変更した値を取得
		m_gerstnerWaveConstBufferData.active1 = static_cast<float>(active1);
		m_gerstnerWaveConstBufferData.active2 = static_cast<float>(active2);
		m_gerstnerWaveConstBufferData.active3 = static_cast<float>(active3);

		//--- 追加: 海のノイズパラメータ設定 ---
		ImGui::Separator();
		ImGui::Text("Sea Noise");
		ImGui::InputFloat("Flow Velocity", &m_seaNoiseConstBufferData.flowVelocity);
		ImGui::InputFloat("Swing Speed", &m_seaNoiseConstBufferData.swingSpeed);
		ImGui::InputFloat("UV Path 1", &m_seaNoiseConstBufferData.fnUVPath1);
		ImGui::InputFloat("UV Path 2", &m_seaNoiseConstBufferData.fnUVPath2);
		ImGui::InputFloat("UV Power", &m_seaNoiseConstBufferData.fnUVPower);
		ImGui::InputFloat("Octaves", &m_seaNoiseConstBufferData.fnOctaves);
		ImGui::InputFloat("Persistence", &m_seaNoiseConstBufferData.fnPersistence);

	}
	ImGui::End();
}

/// <summary>
/// 終了処理
/// </summary>
void Scene::Finalize() {}


void Scene::CreateShaderWithBuffers()
{

	// シェーダを読み込むためのblob
	std::vector<uint8_t> blob;

	// 頂点シェーダをロードする
	blob = DX::ReadData(L"Resources/Shaders/cso/Sea_VS.cso");
	DX::ThrowIfFailed(
		m_device->CreateVertexShader(blob.data(), blob.size(), nullptr, m_vertexShader.ReleaseAndGetAddressOf())
	);

	//	インプットレイアウトの作成
	m_device->CreateInputLayout(
		DirectX::VertexPositionTexture::InputElements,
		DirectX::VertexPositionTexture::InputElementCount,
		blob.data(), blob.size(),
		m_inputLayout.GetAddressOf());

	// ハルシェーダーをロードする
	blob = DX::ReadData(L"Resources/Shaders/cso/Sea_HS.cso");
	DX::ThrowIfFailed(
		m_device->CreateHullShader(blob.data(), blob.size(), nullptr, m_hullShader.ReleaseAndGetAddressOf())
	);

	// ドメインシェーダーをロードする
	blob = DX::ReadData(L"Resources/Shaders/cso/Sea_DS.cso");
	DX::ThrowIfFailed(
		m_device->CreateDomainShader(blob.data(), blob.size(), nullptr, m_domainShader.ReleaseAndGetAddressOf())
	);

	// ピクセルシェーダをロードする
	blob = DX::ReadData(L"Resources/Shaders/cso/Sea_PS.cso");
	DX::ThrowIfFailed(
		m_device->CreatePixelShader(blob.data(), blob.size(), nullptr, m_pixelShader.ReleaseAndGetAddressOf())
	);


	// 定数バッファの作成
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(TransformConstBuffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	m_device->CreateBuffer(&desc, nullptr, m_constantBuffer.ReleaseAndGetAddressOf());

	GerstnerWaveConstBuffer waveParams =
	{
		// 大波（うねり）
		1.2f, -0.8f,  0.6f, 0.9f,
		12.0f, 1.4f, 0.4f, 0.0f,

		// 中波（反射干渉）
		0.9f,  0.6f, -0.7f, 0.6f,
		6.0f,  1.6f, 0.5f, 0.0f,

		// 小波（細かい風のさざ波）
		0.6f, -0.3f, -1.1f, 0.3f,
		2.5f, 2.0f,  0.65f, 0.0f
	};

	// パラメータの初期値
	m_gerstnerWaveConstBufferData = waveParams;

	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(GerstnerWaveConstBuffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	m_device->CreateBuffer(&desc, nullptr, m_gerstnerWaveConstBuffer.ReleaseAndGetAddressOf());

	m_context->UpdateSubresource(m_gerstnerWaveConstBuffer.Get(), 0, nullptr, &m_gerstnerWaveConstBufferData, 0, 0);


	SeaNoiseConstBuffer seaNoise =
	{
		0.1f,
		0.01f,
		0.08f,
		0.06f,
		25.0f,
		10.0f,
		0.6f,
		0.01f
	};

	m_seaNoiseConstBufferData = seaNoise;

	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(SeaNoiseConstBuffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	m_device->CreateBuffer(&desc, nullptr, m_seaNoiseConstBuffer.ReleaseAndGetAddressOf());

	m_context->UpdateSubresource(m_seaNoiseConstBuffer.Get(), 0, nullptr, &m_seaNoiseConstBufferData, 0, 0);
}


/// <summary>
/// 頂点を生成
/// </summary>
void Scene::CreateVertices()
{
	// クリア
	m_vertices.clear();
	// 必要な分配列を用意
	m_vertices.reserve(4);

	constexpr int gridWidth = 2;
	constexpr int gridHeight = 2;
	constexpr float tileSize = 10.0f;

	m_vertices.reserve(gridWidth * gridHeight);

	for (int z = 0; z < gridHeight; ++z)
	{
		for (int x = 0; x < gridWidth; ++x)
		{
			float fx = static_cast<float>(x) / (gridWidth - 1);
			float fz = static_cast<float>(z) / (gridHeight - 1);

			float posX = (fx - 0.5f) * tileSize * (gridWidth - 1);
			float posZ = (fz - 0.5f) * tileSize * (gridHeight - 1);

			m_vertices.push_back({
				DirectX::SimpleMath::Vector3(posX, 0.0f, posZ),
				DirectX::SimpleMath::Vector2(fx, fz)
				});
		}
	}

	// 頂点バッファの作成
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(DirectX::VertexPositionTexture) * m_vertices.size());
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexBufferData = {};
	vertexBufferData.pSysMem = m_vertices.data();

	m_device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, m_vertexBuffer.ReleaseAndGetAddressOf());
}


void Scene::CreateInstanceData()
{
	m_instanceData.clear();

	constexpr int width = 2000;
	constexpr int height = 2000;
	constexpr float tileSize = 10.0f; // 各板ポリの幅

	m_instanceData.clear();
	m_instanceData.reserve(width * height);

	// 原点を中心に配置するためのオフセット
	const float offsetX = (width - 1) * tileSize * 0.5f;
	const float offsetZ = (height - 1) * tileSize * 0.5f;

	for (int y = 0; y < height; ++y)
	{
		for (int x = 0; x < width; ++x)
		{
			float worldX = x * tileSize - offsetX;
			float worldZ = y * tileSize - offsetZ;

			InstanceData instance = {};
			instance.worldMatrix = DirectX::SimpleMath::Vector3(worldX, 0.0f, worldZ);
			m_instanceData.push_back(instance);
		}
	}


	// StructuredBuffer 用バッファ作成
	D3D11_BUFFER_DESC desc = {};
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = UINT(sizeof(InstanceData) * m_instanceData.size());
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.StructureByteStride = sizeof(InstanceData);
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = m_instanceData.data();

	m_device->CreateBuffer(&desc, &initData, m_instanceStructuredBuffer.ReleaseAndGetAddressOf());

	// Shader Resource View を作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.ElementWidth = static_cast<UINT>(m_instanceData.size());

	m_device->CreateShaderResourceView(m_instanceStructuredBuffer.Get(), &srvDesc, m_instanceSRV.ReleaseAndGetAddressOf());

}

