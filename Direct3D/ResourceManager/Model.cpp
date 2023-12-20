#include "Model.h"

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
	Model::Model(ID3D11Device* d3dDevice, const std::string& fileName)
	{
		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);    // $assimp_fbx$ 노드 생성안함
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

		std::function<void(aiNode*)> nodeRecursive = [&](aiNode* node)
			{
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
						vertex::PosNormalTexTan vertex;

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
						aiFace face = mesh->mFaces[i];

						for (UINT k = 0; k < face.mNumIndices; ++k)
						{
							Indices.push_back(face.mIndices[k]);
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
						const char* fileName = strrchr(texturePath.C_Str(), '\\') + 1;
						auto curPath = basePath / fileName;
						ID3D11ShaderResourceView* srv = ResourceManager::GetInstance()->LoadTexture(curPath.c_str());

						AlbedoSRVs.push_back(srv);
					}
					if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS)
					{
						const char* fileName = strrchr(texturePath.C_Str(), '\\') + 1;
						auto curPath = basePath / fileName;
						ID3D11ShaderResourceView* srv = ResourceManager::GetInstance()->LoadTexture(curPath.c_str());

						NormalSRVs.push_back(srv);
					}
					if (material->GetTexture(aiTextureType_METALNESS, 0, &texturePath) == AI_SUCCESS)
					{
						const char* fileName = strrchr(texturePath.C_Str(), '\\') + 1;
						auto curPath = basePath / fileName;
						ID3D11ShaderResourceView* srv = ResourceManager::GetInstance()->LoadTexture(curPath.c_str());

						MetalnessSRVs.push_back(srv);
					}
					if (material->GetTexture(aiTextureType_SHININESS, 0, &texturePath) == AI_SUCCESS)
					{
						const char* fileName = strrchr(texturePath.C_Str(), '\\') + 1;
						auto curPath = basePath / fileName;
						ID3D11ShaderResourceView* srv = ResourceManager::GetInstance()->LoadTexture(curPath.c_str());

						RoughnessSRVs.push_back(srv);
					}
				}

				for (UINT i = 0; i < node->mNumChildren; ++i)
				{
					nodeRecursive(node->mChildren[i]);
				}
			};

		nodeRecursive(scene->mRootNode);
	}

	Model::~Model()
	{
		ReleaseCOM(VB);
		ReleaseCOM(IB);
	}

	void Model::Draw(ID3D11DeviceContext* d3dContext)
	{
		UINT offset = 0;
		d3dContext->IASetIndexBuffer(IB, IndexBufferFormat, 0);
		d3dContext->IASetVertexBuffers(0, 1, &VB, &VertexStride, &offset);

		for (Subset& subset : SubsetTable)
		{
			d3dContext->DrawIndexed(subset.FaceCount * 3, subset.FaceStart * 3, 0);
		}
	}
}