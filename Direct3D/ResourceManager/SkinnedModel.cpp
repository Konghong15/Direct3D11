#include "SkinnedModel.h"

#include <functional>
#include <cassert>
#include <filesystem>
#include <string>
#include <set>
#include <directxtk/WICTextureLoader.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "d3dUtil.h"
#include "ResourceManager.h"


namespace resourceManager
{
	extern DirectX::SimpleMath::Matrix convertMatrix(const aiMatrix4x4& aiMatrix);

	SkinnedModel::SkinnedModel(ID3D11Device* d3dDevice, const std::string& fileName)
	{
		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);    // $assimp_fbx$ 드 생.성안함
		unsigned int importFlags = aiProcess_Triangulate |
			aiProcess_GenNormals |
			aiProcess_GenUVCoords |
			aiProcess_CalcTangentSpace |
			aiProcess_LimitBoneWeights |
			aiProcess_ConvertToLeftHanded;

		const aiScene* scene = importer.ReadFile(fileName, importFlags);
		UINT id = 0;

		Vertices.reserve(1024);
		Indices.reserve(1024);

		std::function<void(aiNode*, int)> nodeRecursive = [this, scene, &nodeRecursive, &id](aiNode* node, int parentIndex)
			{
				BoneHierarchy.push_back(parentIndex);

				for (UINT i = 0; i < node->mNumMeshes; ++i)
				{
					unsigned int meshIndex = node->mMeshes[i];
					aiMesh* mesh = scene->mMeshes[meshIndex];

					Subset subset;
					subset.Id = id++;
					subset.VertexCount = mesh->mNumVertices;
					subset.FaceCount = mesh->mNumFaces;

					if (subset.Id > 0)
					{
						const Subset& prev = SubsetTable[subset.Id - 1];
						subset.VertexStart = prev.VertexStart + prev.VertexCount;
						subset.FaceStart = prev.FaceStart + prev.FaceCount;
					}

					SubsetTable.push_back(subset);

					for (UINT j = 0; j < mesh->mNumVertices; ++j)
					{
						vertex::PosNormalTexTanSkinned vertex;

						if (mesh->HasPositions())
						{
							vertex.Pos.x = mesh->mVertices[j].x;
							vertex.Pos.y = mesh->mVertices[j].y;
							vertex.Pos.z = mesh->mVertices[j].z;
						}

						if (mesh->HasNormals())
						{
							vertex.Normal.x = mesh->mNormals[j].x;
							vertex.Normal.y = mesh->mNormals[j].y;
							vertex.Normal.z = mesh->mNormals[j].z;
						}

						if (mesh->HasTangentsAndBitangents())
						{
							vertex.TangentU.x = mesh->mTangents[j].x;
							vertex.TangentU.y = mesh->mTangents[j].y;
							vertex.TangentU.z = mesh->mTangents[j].z;
						}

						if (mesh->HasTextureCoords(0))
						{
							vertex.Tex.x = (float)mesh->mTextureCoords[0][j].x;
							vertex.Tex.y = (float)mesh->mTextureCoords[0][j].y;
						}

						Vertices.push_back(vertex);
					}

					for (UINT j = 0; j < mesh->mNumFaces; ++j) {
						aiFace face = mesh->mFaces[j];

						for (UINT k = 0; k < face.mNumIndices; ++k)
						{
							Indices.push_back(face.mIndices[k]);
						}
					}

					// 메쉬에 저장된 본 팔레트 정보가 내가 중위 순회로 처리한 것과 동일한지가 중요하다.
					// 일단 하나 체크해본 결과 같음
					// 애초에 본 애니메이션 하는 캐릭터라면 메쉬는 반드시 하나이려나?
					for (UINT j = 0; j < mesh->mNumBones; ++j)
					{
						BoneOffsetMatrix.push_back(convertMatrix(mesh->mBones[j]->mOffsetMatrix).Transpose());

						for (UINT k = 0; k < mesh->mBones[j ]->mNumWeights; ++k)
						{
							unsigned int vertexIndex = mesh->mBones[j]->mWeights[k].mVertexId + subset.VertexStart;
							float vertexWeight = mesh->mBones[j]->mWeights[k].mWeight;

							for (UINT l = 0; l < 4; ++l)
							{
								if (Vertices[vertexIndex].Indices[l] == vertex::PosNormalTexTanSkinned::INVALID_INDEX)
								{
									Vertices[vertexIndex].Indices[l] = j;
									Vertices[vertexIndex].Weights[l] = vertexWeight;
									break;
								}
							}
						}
					}

					aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

					aiString texturePath;
					std::filesystem::path basePath = std::filesystem::current_path() / "textures";

					std::vector<bool> hasTexture(21);
					for (int i = 0; i < 21; ++i)
					{
						if (material->GetTexture((aiTextureType)i, 0, &texturePath) == AI_SUCCESS)
						{
							hasTexture[i] = true;
						}
					}

					if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
					{
						const char* fileName = strrchr(texturePath.C_Str(), '/') + 1;
						auto curPath = basePath / fileName;
						ID3D11ShaderResourceView* srv = ResourceManager::GetInstance()->LoadTexture(curPath.c_str());

						SRVs[static_cast<size_t>(eMaterialTexture::Diffuse)].push_back(srv);
					}
					else
					{
						SRVs[static_cast<size_t>(eMaterialTexture::Diffuse)].push_back(nullptr);
					}
					if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS)
					{
						const char* fileName = strrchr(texturePath.C_Str(), '/') + 1;
						auto curPath = basePath / fileName;
						ID3D11ShaderResourceView* srv = ResourceManager::GetInstance()->LoadTexture(curPath.c_str());

						SRVs[static_cast<size_t>(eMaterialTexture::Normal)].push_back(srv);
					}
					else
					{
						SRVs[static_cast<size_t>(eMaterialTexture::Normal)].push_back(nullptr);
					}
					if (material->GetTexture(aiTextureType_SPECULAR, 0, &texturePath) == AI_SUCCESS)
					{
						const char* fileName = strrchr(texturePath.C_Str(), '/') + 1;
						auto curPath = basePath / fileName;
						ID3D11ShaderResourceView* srv = ResourceManager::GetInstance()->LoadTexture(curPath.c_str());

						SRVs[static_cast<size_t>(eMaterialTexture::Specular)].push_back(srv);
					}
					else
					{
						SRVs[static_cast<size_t>(eMaterialTexture::Specular)].push_back(nullptr);

					}
					if (material->GetTexture(aiTextureType_OPACITY, 0, &texturePath) == AI_SUCCESS)
					{
						const char* fileName = strrchr(texturePath.C_Str(), '/') + 1;
						auto curPath = basePath / fileName;
						ID3D11ShaderResourceView* srv = ResourceManager::GetInstance()->LoadTexture(curPath.c_str());

						SRVs[static_cast<size_t>(eMaterialTexture::Opacity)].push_back(srv);
					}
					else
					{
						SRVs[static_cast<size_t>(eMaterialTexture::Opacity)].push_back(nullptr);
					}
				}

				int nodeIndex = BoneHierarchy.size();
				for (UINT i = 0; i < node->mNumChildren; ++i)
				{
					nodeRecursive(node->mChildren[i], nodeIndex);
				}
			};
		nodeRecursive(scene->mRootNode, 0);

		importer.FreeScene();

		HRESULT hr;

		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = static_cast<size_t>(sizeof(vertex::PosNormalTexTanSkinned) * Vertices.size());
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &Vertices[0];

		hr = d3dDevice->CreateBuffer(&vbd, &initData, &VB);
		if (FAILED(hr)) {
			assert(false);
		}

		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = static_cast<size_t>(sizeof(unsigned int) * Indices.size());
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;

		initData.pSysMem = &Indices[0];

		hr = d3dDevice->CreateBuffer(&ibd, &initData, &IB);
		if (FAILED(hr)) {
			assert(false);
		}

		D3D11_BUFFER_DESC cbd;
		cbd.Usage = D3D11_USAGE_DEFAULT;
		cbd.ByteWidth = sizeof(int) * static_cast<size_t>(eMaterialTexture::Size);
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = 0;
		cbd.MiscFlags = 0;

		hr = d3dDevice->CreateBuffer(&cbd, NULL, &CB);
		if (FAILED(hr)) {
			assert(false);
		}
	}

	void SkinnedModel::Draw(ID3D11DeviceContext* d3dContext)
	{

	}

	void SkinnedModel::GetFinalTransforms(const std::string& clipName, float timePos, std::vector<DirectX::SimpleMath::Matrix>* matrixPalette) const
	{
		using namespace DirectX::SimpleMath;

		const size_t BONE_COUNT = BoneOffsetMatrix.size();

		std::vector<Matrix> toParents(BONE_COUNT);
		std::vector<Matrix> toRoots(BONE_COUNT);

		auto find = Animations.find(clipName);

		for (size_t i = 0; i < BONE_COUNT; ++i)
		{
			toParents[i] = find->second.AnimationNodes[i].Evaluate(timePos);
		}

		toRoots[0] = toParents[0];

		for (UINT i = 1; i < BONE_COUNT; ++i)
		{
			Matrix toParent = toParents[i];

			int parentIndex = BoneHierarchy[i];
			Matrix parentToRoot = toRoots[parentIndex];

			toRoots[i] = toParent * parentToRoot;
		}

		for (UINT i = 0; i < BONE_COUNT; ++i)
		{
			(*matrixPalette)[i] = BoneOffsetMatrix[i] * toRoots[i];
		}
	}
}