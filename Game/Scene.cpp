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
	ConstBuffer cbuff;
	cbuff.matView = m_camera->GetViewMatrix().Transpose();
	cbuff.matProj = m_commonResources->GetProjectionMatrix().Transpose();
	cbuff.matWorld = world.Transpose();
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
	ID3D11Buffer* cb[2] = { m_constantBuffer.Get() , m_waveConstantBuffer.Get() };
	// �X���b�g1����3�Ɉꊇ�o�C���h
	m_context->VSSetConstantBuffers(0, 2, cb);
	m_context->HSSetConstantBuffers(0, 2, cb);
	m_context->DSSetConstantBuffers(0, 2, cb);
	m_context->PSSetConstantBuffers(0, 2, cb);

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
	else m_context->RSSetState(m_commonStates->CullClockwise());
	

	//	�V�F�[�_���Z�b�g����
	m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
	m_context->HSSetShader(m_hullShader.Get(), nullptr, 0);
	m_context->DSSetShader(m_domainShader.Get(), nullptr, 0);
	m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	// �e�N�X�`���̐ݒ�
	ID3D11ShaderResourceView* tex[1] = { m_texture.Get() };
	m_context->PSSetShaderResources(0, 1, tex);

	// �`��R�[��
	m_context->Draw(32, 0);

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
		ImGui::InputFloat("Direction X##1", &m_waveParams.direction1X);
		ImGui::InputFloat("Direction Z##1", &m_waveParams.direction1Z);
		ImGui::InputFloat("Amplitude##1", &m_waveParams.amplitude1);
		ImGui::InputFloat("Wave Length##1", &m_waveParams.waveLength1);
		ImGui::InputFloat("Speed##1", &m_waveParams.speed1);
		ImGui::InputFloat("Q Ratio##1", &m_waveParams.qRatio1);

		ImGui::Separator();
		ImGui::Text("Wave 2");
		ImGui::Checkbox("Active##2", &active2);
		ImGui::InputFloat("Direction X##2", &m_waveParams.direction2X);
		ImGui::InputFloat("Direction Z##2", &m_waveParams.direction2Z);
		ImGui::InputFloat("Amplitude##2", &m_waveParams.amplitude2);
		ImGui::InputFloat("Wave Length##2", &m_waveParams.waveLength2);
		ImGui::InputFloat("Speed##2", &m_waveParams.speed2);
		ImGui::InputFloat("Q Ratio##2", &m_waveParams.qRatio2);

		ImGui::Separator();
		ImGui::Text("Wave 3");
		ImGui::Checkbox("Active##3", &active3);
		ImGui::InputFloat("Direction X##3", &m_waveParams.direction3X);
		ImGui::InputFloat("Direction Z##3", &m_waveParams.direction3Z);
		ImGui::InputFloat("Amplitude##3", &m_waveParams.amplitude3);
		ImGui::InputFloat("Wave Length##3", &m_waveParams.waveLength3);
		ImGui::InputFloat("Speed##3", &m_waveParams.speed3);
		ImGui::InputFloat("Q Ratio##3", &m_waveParams.qRatio3);

		// �ύX�����l���擾
		m_waveParams.active1 = static_cast<float>(active1);
		m_waveParams.active2 = static_cast<float>(active2);
		m_waveParams.active3 = static_cast<float>(active3);
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
	desc.ByteWidth = sizeof(ConstBuffer);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	m_device->CreateBuffer(&desc, nullptr, m_constantBuffer.ReleaseAndGetAddressOf());

	WaveParams waveParams =
	{
		// ��g�i������蒷�g�j
		1.0f, -0.6f,  0.4f, 0.6f,    // �U���}����
		14.0f, 1.2f, 0.35f, 0.0f,    // ���g���E�ᑬ�E�ۂ�

		// ���g�i�ɂ₩�ȋt�����g�j
		1.0f, 0.5f, -0.5f, 0.3f,
		6.5f, 1.0f, 0.3f, 0.0f,

		// �הg�i���̗h�炬���x�j
		1.0f, 0.2f, -0.8f, 0.15f,
		2.8f, 0.7f, 0.25f, 0.0f
	};

	// �p�����[�^�̏����l
	m_waveParams = waveParams;

	ZeroMemory(&desc, sizeof(desc));
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.ByteWidth = sizeof(WaveParams);
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = 0;
	m_device->CreateBuffer(&desc, nullptr, &m_waveConstantBuffer);

	m_context->UpdateSubresource(m_waveConstantBuffer.Get(), 0, nullptr, &waveParams, 0, 0);


	// ���_�f�[�^���`�i4�̐���_�j
	const float SIZE = 30.0f;

	const int GRID_X = 8;
	const int GRID_Z = 4;
	const int VERTEX_COUNT = GRID_X * GRID_Z;

	DirectX::VertexPositionTexture vertex[VERTEX_COUNT];

	for (int z = 0; z < GRID_Z; ++z)
	{
		for (int x = 0; x < GRID_X; ++x)
		{
			int index = z * GRID_X + x;
			float fx = -SIZE + (2 * SIZE) * (float)x / (GRID_X - 1);
			float fz = SIZE - (2 * SIZE) * (float)z / (GRID_Z - 1);
			vertex[index] = {
				DirectX::XMFLOAT3(fx, 0.0f, fz),
				DirectX::XMFLOAT2((float)x / (GRID_X - 1), (float)z / (GRID_Z - 1))
			};
		}
	}


	// ���_�o�b�t�@�̐���
	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(DirectX::VertexPositionTexture) * _countof(vertex); // ���_�f�[�^�S�̂̃T�C�Y
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;

	// ���_�f�[�^�̏�����
	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = vertex;

	m_device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
}


