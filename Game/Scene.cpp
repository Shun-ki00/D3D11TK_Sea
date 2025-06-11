#include "pch.h"
#include "Game/Scene.h"
#include "Framework/CommonResources.h"
#include "Framework/DebugCamera.h"
#include "Framework/Microsoft/ReadData.h"
#include <WICTextureLoader.h>

/// <summary>
/// �R���X�g���N�^
/// </summary>
Scene::Scene()
{
	// �C���X�^���X���擾����
	m_commonResources = CommonResources::GetInstance();
	m_device          = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDevice();
	m_context         = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();
	m_commonStates    = CommonResources::GetInstance()->GetCommonStates();
}

/// <summary>
/// ����������
/// </summary>
void Scene::Initialize()
{
	// �J�����̍쐬
	m_camera = std::make_unique<DebugCamera>();
	m_camera->Initialize(1280, 720);

	// �e�N�X�`���̃��[�h
	DirectX::CreateWICTextureFromFile(
		m_device, L"Resources/Textures/Sea.png", nullptr, m_texture.ReleaseAndGetAddressOf());

	// �V�F�[�_�[�A�o�b�t�@�̍쐬
	this->CreateShaderWithBuffers();

	// ������
	m_time = 0.0f;
	m_tessellationIndex = 1.0f;
	m_isWireframe = false;

}

/// <summary>
/// �X�V����
/// </summary>
/// <param name="elapsedTime">�o�ߎ���</param>
void Scene::Update(const float& elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime);

	// �J�����̍X�V����
	m_camera->Update();
	m_commonResources->SetViewMatrix(m_camera->GetViewMatrix());

	// �萔�o�b�t�@�̍X�V����
	m_context->UpdateSubresource(m_seaNoiseConstBuffer.Get(), 0, nullptr, &m_seaNoiseConstBufferData, 0, 0);
	m_context->UpdateSubresource(m_gerstnerWaveConstBuffer.Get(), 0, nullptr, &m_gerstnerWaveConstBufferData, 0, 0);
	

}

/// <summary>
/// �`�揈��
/// </summary>
void Scene::Render() 
{
	// �^�C�}�[�̍X�V
	float elapsedTime = (float)m_commonResources->GetStepTimer()->GetElapsedSeconds();
	m_time += elapsedTime;

	// ���[���h�s��쐬
	DirectX::SimpleMath::Matrix world =
		DirectX::SimpleMath::Matrix::CreateTranslation(DirectX::SimpleMath::Vector3::Down);

	//	�V�F�[�_�[�ɓn���ǉ��̃o�b�t�@���쐬����B
	TransformConstBuffer cbuff;
	cbuff.matView = m_camera->GetViewMatrix().Transpose();
	cbuff.matProj = m_commonResources->GetProjectionMatrix().Transpose();
	cbuff.matWorld = world.Transpose();
	cbuff.cameraPosition = { m_camera->GetEyePosition().x,m_camera->GetEyePosition().y,m_camera->GetEyePosition().z ,0.0f };
	cbuff.TessellationFactor = DirectX::SimpleMath::Vector4(m_tessellationIndex, m_time, 2.0f, 0.0f);

	//	�󂯓n���p�o�b�t�@�̓��e�X�V(ConstBuffer����ID3D11Buffer�ւ̕ϊ��j
	m_context->UpdateSubresource(m_constantBuffer.Get(), 0, NULL, &cbuff, 0, 0);

	// ���̓��C�A�E�g��ݒ�
	m_context->IASetInputLayout(m_inputLayout.Get());

	// ���_�o�b�t�@��ݒ�
	ID3D11Buffer* buffers[] = { m_vertexBuffer.Get() };
	UINT stride[] = { sizeof(DirectX::VertexPositionTexture) };
	UINT offset[] = { 0 };
	m_context->IASetVertexBuffers(0, 1, buffers, stride, offset);

	//	�V�F�[�_�[�Ƀo�b�t�@��n��
	ID3D11Buffer* cb[3] = { m_constantBuffer.Get() , m_seaNoiseConstBuffer.Get() , m_gerstnerWaveConstBuffer.Get()};
	// �X���b�g1����3�Ɉꊇ�o�C���h
	m_context->VSSetConstantBuffers(0, 3, cb);
	m_context->HSSetConstantBuffers(0, 3, cb);
	m_context->DSSetConstantBuffers(0, 3, cb);
	m_context->PSSetConstantBuffers(0, 3, cb);

	// �T���v���[�X�e�[�g���s�N�Z���V�F�[�_�[�ɐݒ�
	ID3D11SamplerState* sampler[1] = { m_commonStates->LinearWrap() };
	m_context->PSSetSamplers(0, 1, sampler);

	// �u�����h�X�e�[�g��ݒ� (�������`��p)
	m_context->OMSetBlendState(m_commonStates->AlphaBlend(), nullptr, 0xFFFFFFFF);

	// �v���~�e�B�u�g�|���W�[��ݒ�
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST);

	//	�[�x�o�b�t�@�ɏ������ݎQ�Ƃ���
	m_context->OMSetDepthStencilState(m_commonStates->DepthDefault(), 0);

	// ���X�^���C�U�[�X�e�[�g�̐ݒ�
	if(m_isWireframe) m_context->RSSetState(m_commonStates->Wireframe());
	else m_context->RSSetState(m_commonStates->CullCounterClockwise());
	

	//	�V�F�[�_���Z�b�g����
	m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_context->HSSetShader(m_hullShader.Get(), nullptr, 0);
	m_context->DSSetShader(m_domainShader.Get(), nullptr, 0);
	m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	// �e�N�X�`���̐ݒ�
	ID3D11ShaderResourceView* tex[1] = { m_texture.Get() };
	m_context->PSSetShaderResources(0, 1, tex);

	// �`��R�[��
	m_context->Draw(static_cast<UINT>(32 * (200 * 200)), 0);

	//	�V�F�[�_�̓o�^������
	m_context->VSSetShader(nullptr, nullptr, 0);
	m_context->HSSetShader(nullptr, nullptr, 0);
	m_context->DSSetShader(nullptr, nullptr, 0);
	m_context->PSSetShader(nullptr, nullptr, 0);

	// �e�N�X�`�����\�[�X�����
	ID3D11ShaderResourceView* nullsrv[] = { nullptr };
	m_context->PSSetShaderResources(0, 1, nullsrv);

}


void Scene::ImguiWindow()
{

	if (ImGui::Begin("Wave Settings"))
	{
		ImGui::Text("Tessellation");
		ImGui::InputFloat("Tessellation Factor", &m_tessellationIndex, 1.0f, 1.0f);

		ImGui::Checkbox("WireFrame", &m_isWireframe);

		// �`�F�b�N�{�b�N�X�p
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

		// �ύX�����l���擾
		m_gerstnerWaveConstBufferData.active1 = static_cast<float>(active1);
		m_gerstnerWaveConstBufferData.active2 = static_cast<float>(active2);
		m_gerstnerWaveConstBufferData.active3 = static_cast<float>(active3);

		//--- �ǉ�: �C�̃m�C�Y�p�����[�^�ݒ� ---
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
/// �I������
/// </summary>
void Scene::Finalize() {}


void Scene::CreateShaderWithBuffers()
{

	// �V�F�[�_��ǂݍ��ނ��߂�blob
	std::vector<uint8_t> blob;

	// ���_�V�F�[�_�����[�h����
	blob = DX::ReadData(L"Resources/Shaders/cso/Sea_VS.cso");
	DX::ThrowIfFailed(
		m_device->CreateVertexShader(blob.data(), blob.size(), nullptr, m_vertexShader.ReleaseAndGetAddressOf())
	);

	//	�C���v�b�g���C�A�E�g�̍쐬
	m_device->CreateInputLayout(
		DirectX::VertexPositionTexture::InputElements,
		DirectX::VertexPositionTexture::InputElementCount,
		blob.data(), blob.size(),
		m_inputLayout.GetAddressOf());

	// �n���V�F�[�_�[�����[�h����
	blob = DX::ReadData(L"Resources/Shaders/cso/Sea_HS.cso");
	DX::ThrowIfFailed(
		m_device->CreateHullShader(blob.data(), blob.size(), nullptr, m_hullShader.ReleaseAndGetAddressOf())
	);

	// �h���C���V�F�[�_�[�����[�h����
	blob = DX::ReadData(L"Resources/Shaders/cso/Sea_DS.cso");
	DX::ThrowIfFailed(
		m_device->CreateDomainShader(blob.data(), blob.size(), nullptr, m_domainShader.ReleaseAndGetAddressOf())
	);

	// �s�N�Z���V�F�[�_�����[�h����
	blob = DX::ReadData(L"Resources/Shaders/cso/Sea_PS.cso");
	DX::ThrowIfFailed(
		m_device->CreatePixelShader(blob.data(), blob.size(), nullptr, m_pixelShader.ReleaseAndGetAddressOf())
	);


	// �萔�o�b�t�@�̍쐬
	D3D11_BUFFER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(TransformConstBuffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	m_device->CreateBuffer(&desc, nullptr, m_constantBuffer.ReleaseAndGetAddressOf());

	GerstnerWaveConstBuffer waveParams =
	{
		// ��g�i���˂�j
		1.2f, -0.8f,  0.6f, 0.9f,
		12.0f, 1.4f, 0.4f, 0.0f,

		// ���g�i���ˊ��j
		0.9f,  0.6f, -0.7f, 0.6f,
		6.0f,  1.6f, 0.5f, 0.0f,

		// ���g�i�ׂ������̂����g�j
		0.6f, -0.3f, -1.1f, 0.3f,
		2.5f, 2.0f,  0.65f, 0.0f
	};

	// �p�����[�^�̏����l
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


	const int PATCH_COUNT_X = 200;
	const int PATCH_COUNT_Z = 200;
	const int PATCH_TOTAL = PATCH_COUNT_X * PATCH_COUNT_Z;
	const int VERTICES_PER_PATCH = 32;
	const int TOTAL_VERTICES = PATCH_TOTAL * VERTICES_PER_PATCH;

	std::vector<DirectX::VertexPositionTexture> vertices(TOTAL_VERTICES);

	// 1�̃p�b�`�̃T�C�Y
	const float PATCH_SIZE = 20.0f; // �e�p�b�`�̗̈�i���[���h���W�Łj
	const float HALF_EXTENT_X = PATCH_COUNT_X * PATCH_SIZE * 0.5f;
	const float HALF_EXTENT_Z = PATCH_COUNT_Z * PATCH_SIZE * 0.5f;

	for (int patchZ = 0; patchZ < PATCH_COUNT_Z; ++patchZ) {
		for (int patchX = 0; patchX < PATCH_COUNT_X; ++patchX) {

			int patchIndex = patchZ * PATCH_COUNT_X + patchX;
			int baseVertexIndex = patchIndex * VERTICES_PER_PATCH;

			float offsetX = patchX * PATCH_SIZE - HALF_EXTENT_X;
			float offsetZ = patchZ * PATCH_SIZE - HALF_EXTENT_Z;

			for (int z = 0; z < 4; ++z) {
				for (int x = 0; x < 8; ++x) {
					int localIndex = z * 8 + x;

					float fx = offsetX + PATCH_SIZE * (float)x / 7.0f; 
					float fz = offsetZ + PATCH_SIZE * (float)z / 3.0f;

					float u = (float)x / 7.0f;
					float v = (float)z / 3.0f;

					vertices[baseVertexIndex + localIndex] = {
						DirectX::XMFLOAT3(fx, 0.0f, fz),
						DirectX::XMFLOAT2(u, v)
					};
				}
			}
		}
	}


	// ���_�o�b�t�@�̐���
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = static_cast<UINT>(sizeof(DirectX::VertexPositionTexture) * vertices.size()); 
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;

	// ���_�f�[�^�̏�����
	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = vertices.data();

	m_device->CreateBuffer(&vertexBufferDesc, &vertexData, m_vertexBuffer.ReleaseAndGetAddressOf());
}


