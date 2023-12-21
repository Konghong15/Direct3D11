#pragma once

#include <array>
#include <vector>
#include <string>

#include <d3d11.h>

#include "Animation.h"
#include "Subset.h"
#include "eMaterialTexture.h"
#include "LightHelper.h"
#include "Vertex.h"

namespace resourceManager
{
	class SkinnedModel
	{
	public:
		SkinnedModel(ID3D11Device* d3dDevice, const std::string& fileName);

		void Draw(ID3D11DeviceContext* d3dContext);

		// �� �ȷ�Ʈ ����� �ַ��� ��� ���� ������ ������ �����Ѵ�.
		void GetFinalTransforms(const std::string& clipName, float timePos, std::vector<DirectX::SimpleMath::Matrix>* matrixPalette) const;

	public:
		// material
		std::vector<common::Material> Materials;
		std::array<std::vector<ID3D11ShaderResourceView*>, static_cast<size_t>(eMaterialTexture::Size)> SRVs;
		ID3D11Buffer* CB;

		// mesh
		std::vector<vertex::PosNormalTexTanSkinned> Vertices;
		std::vector<UINT> Indices;
		ID3D11Buffer* VB;
		ID3D11Buffer* IB;
		DXGI_FORMAT IndexBufferFormat; // �׻� 32��Ʈ ������ 
		UINT VertexStride;
		std::vector<Subset> SubsetTable;

		// scene data
		DirectX::BoundingBox BoundingBox;
		DirectX::BoundingSphere BoundingSphere;

		// bone
		std::vector<int> BoneHierarchy;
		std::vector<DirectX::SimpleMath::Matrix> BoneOffsetMatrix;
		ID3D11Buffer* BoneCB;

		// animation 
		std::map<std::string, AnimationClip> Animations;
	};
}