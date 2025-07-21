#pragma once
#include <future>
#include "Framework/DebugCamera.h"
#include "Framework/ConstantBuffer.h"

class CommonResources;
class DebugCamera;

class Scene
{
public:

	// �C���X�^���X�f�[�^�̐�
	static const int WIDTH; 
	static const int HEIGHT;
	// �^�C���T�C�Y
	static const float TILE_SIZE;

private:

	// �ϊ��p�萔�o�b�t�@
	struct TransformConstBuffer
	{
		DirectX::SimpleMath::Matrix		matWorld;
		DirectX::SimpleMath::Matrix		matView;
		DirectX::SimpleMath::Matrix		matProj;
		DirectX::SimpleMath::Vector4    cameraPosition;
		DirectX::SimpleMath::Vector4    TessellationFactor;
	};

	// �C���X�^���X�f�[�^
	struct InstanceData
	{
		DirectX::SimpleMath::Vector3 worldMatrix;
	};

	// �C�̃m�C�Y�p�̒萔�o�b�t�@
	struct SeaNoiseConstBuffer
	{
		float flowVelocity;    // ������
		float swingSpeed;      // ���E�̐U��
		float fnUVPath1;       // �m�C�Y�̉e�� (x��)
		float fnUVPath2;       // �m�C�Y�̉e�� (y��)

		float fnUVPower;       // UV�X�P�[��
		float fnOctaves;       // �m�C�Y�̃I�N�^�[�u��
		float fnPersistence;   // �m�C�Y�����x
		float padding;         // �p�f�B���O
	};

	// �Q���X�g�i�g�̒萔�o�b�t�@
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

	// �R���X�g���N�^
	Scene();
	// �f�X�g���N�^
	~Scene() = default;

public:

	// ����������
	void Initialize();
	// �X�V����
	void Update(const float& elapsedTime);
	// �`�揈��
	void Render();
	// �I������
	void Finalize();

	// �f�o�b�O�E�B���h�E
	void ImguiWindow();

private:

	// ���_�̐���
	void CreateVertices();
	// �C���f�N�X�̐���
	void CreateIndices();
	// �C���X�^���X�f�[�^�̍쐬
	void CreateInstanceData();

	// �V�F�[�_�[��o�b�t�@�̍쐬
	void CreateShaderWithBuffers();
	
private:

	// ���L���\�[�X
	CommonResources* m_commonResources;

	// �f�o�b�O�J����
	std::unique_ptr<DebugCamera> m_camera;

	// �f�o�C�X
	ID3D11Device1* m_device;
	// �R���e�L�X�g
	ID3D11DeviceContext1* m_context;
	// �R�����X�e�[�g
	DirectX::CommonStates* m_commonStates;

	// ���̓��C�A�E�g
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;

	// ���_�V�F�[�_�[
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
	// �n���V�F�[�_�[
	Microsoft::WRL::ComPtr<ID3D11HullShader> m_hullShader;
	// �h���C���V�F�[�_�[
	Microsoft::WRL::ComPtr<ID3D11DomainShader> m_domainShader;
	// �s�N�Z���V�F�[�_�[
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;

	// ���_�o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	// �C���f�b�N�X�o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	// �C���X�^���X�o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_instanceStructuredBuffer;

	// �萔�o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

	// �C�̃m�C�Y�p�̒萔�o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_seaNoiseConstBuffer;
	// �Q���X�g�i�g�̃o�b�t�@
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_gerstnerWaveConstBuffer;

	// �e�N�X�`��
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	// �C���X�^���X�̃V�F�[�_�[���\�[�X�r���[
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_instanceSRV;

	// �|���̒��_�Q
	std::vector<DirectX::VertexPositionTexture> m_vertices;
	// �|���̃C���f�b�N�X�o�b�t�@
	std::vector<uint16_t> m_indices;
	// �C���X�^���X�f�[�^
	std::vector<InstanceData> m_instanceData;

	// �C�̃m�C�Y�p�̒萔�o�b�t�@
	SeaNoiseConstBuffer m_seaNoiseConstBufferData;
	// �Q���X�g�i�g�̃o�b�t�@�f�[�^
	GerstnerWaveConstBuffer m_gerstnerWaveConstBufferData;
	// �e�b�Z���[�V�����W��
	float m_tessellationIndex;
	// ���X�^���C�U�[���[�h�ݒ�
	bool m_isWireframe;

	// �o�ߎ���
	float m_time;



};