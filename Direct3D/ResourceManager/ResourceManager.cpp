#include "ResourceManager.h"

#include <cassert>
#include <filesystem>
#include <set>

#include <directxtk/WICTextureLoader.h>

#include "d3dUtil.h"
#include "Model.h"
#include "Node.h"

namespace resourceManager
{
	ResourceManager* ResourceManager::mInstance = nullptr;

	ResourceManager* ResourceManager::GetInstance()
	{
		if (mInstance != nullptr)
		{
			mInstance = new ResourceManager();
		}

		return mInstance;
	}

	void ResourceManager::DeleteInstance()
	{
		delete mInstance;
	}

	ResourceManager::~ResourceManager()
	{
		for (auto& SRV : mSRVs)
		{
			ReleaseCOM(SRV.second);
		}

		for (auto& model : mModels)
		{
			model.second->Release();
		}
	}

	void ResourceManager::Init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext)
	{
		md3dDevice = d3dDevice;
		md3dContext = d3dContext;
	}

	Model* ResourceManager::LoadModel(const std::string& fileName)
	{
		auto find = mModels.find(fileName);

		if (find != mModels.end())
		{
			return find->second;
		}

		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, 0);    // $assimp_fbx$ 노드 생성안함
		unsigned int importFlags = aiProcess_Triangulate |
			aiProcess_GenNormals |
			aiProcess_GenUVCoords |
			aiProcess_CalcTangentSpace |
			aiProcess_LimitBoneWeights |
			aiProcess_ConvertToLeftHanded;

		const aiScene* scene = importer.ReadFile(fileName, importFlags);
		if (scene == nullptr)
		{
			return false;
		}

		Model* model = new Model();
		model->RootNode = new Node();

		processNodeRecursive(scene->mRootNode, *scene, model->RootNode);
		processAnimation(scene, model);
		processBones(model);

		mModels.insert({ fileName, model });

		return model;
	}

	ID3D11ShaderResourceView* ResourceManager::LoadTexture(const std::string& fileName)
	{
		ID3D11ShaderResourceView* SRV = nullptr;

		auto find = mSRVs.find(fileName);

		if (find == mSRVs.end())
		{
			common::D3DHelper::CreateTextureFromFile(md3dDevice, common::D3DHelper::ToWString(fileName).c_str(), &SRV);
		}
		else
		{
			SRV = find->second;
		}

		return SRV;
	}

	void ResourceManager::processNodeRecursive(aiNode* node, const aiScene& scene, Node* parent) const
	{
		assert(node != nullptr);

		Node* myNode = new Node();
		myNode->Parent = parent;

		if (parent != nullptr)
		{
			parent->Children.push_back(myNode);
		}

		myNode->Name = node->mName.C_Str();
		myNode->BindPose = convertMatrix(node->mTransformation).Transpose();

		for (UINT i = 0; i < node->mNumMeshes; i++) {
			unsigned int meshIndex = node->mMeshes[i];
			aiMesh* mesh = scene.mMeshes[meshIndex];

			myNode->Meshes.push_back(processMesh(mesh, scene));
		}

		for (UINT i = 0; i < node->mNumChildren; i++) {
			processNodeRecursive(node->mChildren[i], scene, myNode);
		}
	}

	Mesh ResourceManager::processMesh(aiMesh* mesh, const aiScene& scene) const
	{
		constexpr unsigned int FACE_VERTEX_COUNT = 4;

		Mesh resultMesh;
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		vertices.reserve(mesh->mNumVertices);
		indices.reserve(mesh->mNumFaces * FACE_VERTEX_COUNT);

		for (UINT i = 0; i < mesh->mNumVertices; ++i) {
			Vertex vertex;

			auto position = mesh->mVertices[i];

			vertex.Position.x = position.x;
			vertex.Position.y = position.y;
			vertex.Position.z = position.z;
			vertex.Position.w = 1.f;

			if (mesh->mTextureCoords[0]) {
				vertex.UV.x = (float)mesh->mTextureCoords[0][i].x;
				vertex.UV.y = (float)mesh->mTextureCoords[0][i].y;
			}

			if (mesh->HasNormals())
			{
				auto normal = mesh->mNormals[i];

				vertex.Normal.x = normal.x;
				vertex.Normal.y = normal.y;
				vertex.Normal.z = normal.z;
			}

			if (mesh->HasTangentsAndBitangents())
			{
				auto tangent = mesh->mTangents[i];
				auto Binormal = mesh->mBitangents[i];

				vertex.Tangent.x = tangent.x;
				vertex.Tangent.y = tangent.y;
				vertex.Tangent.z = tangent.z;

				vertex.Binormal.x = Binormal.x;
				vertex.Binormal.x = Binormal.x;
				vertex.Binormal.x = Binormal.x;
			}

			vertices.push_back(vertex);
		}
		for (UINT i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];

			for (UINT j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		std::set<std::string> boneNameSet;

		for (UINT i = 0; i < mesh->mNumBones; ++i)
		{
			Bone bone;
			bone.Name = mesh->mBones[i]->mName.C_Str();

			if (boneNameSet.find(bone.Name) != boneNameSet.end())
			{
				continue;
			}

			bone.OffsetMatrix = convertMatrix(mesh->mBones[i]->mOffsetMatrix).Transpose();

			resultMesh.Bones.push_back(bone);

			for (UINT j = 0; j < mesh->mBones[i]->mNumWeights; ++j)
			{
				unsigned int vertexIndex = mesh->mBones[i]->mWeights[j].mVertexId;
				float vertexWeight = mesh->mBones[i]->mWeights[j].mWeight;

				for (UINT k = 0; k < 4; ++k)
				{
					if (vertices[vertexIndex].Indices[k] == Vertex::INVALID_INDEX)
					{
						vertices[vertexIndex].Indices[k] = i;
						vertices[vertexIndex].Weights[k] = vertexWeight;
						break;
					}
				}
			}
		}

		resultMesh.VertexCount = vertices.size();
		resultMesh.IndexCount = indices.size();

		// create Buffer
		HRESULT hr;

		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = static_cast<size_t>(sizeof(Vertex) * vertices.size());
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &vertices[0];

		hr = md3dDevice->CreateBuffer(&vbd, &initData, &resultMesh.VertexBuffer);
		if (FAILED(hr)) {
			assert(false);
		}

		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = static_cast<size_t>(sizeof(unsigned int) * indices.size());
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;

		initData.pSysMem = &indices[0];

		hr = md3dDevice->CreateBuffer(&ibd, &initData, &resultMesh.IndexBuffer);
		if (FAILED(hr)) {
			assert(false);
		}

		assert(mesh->mMaterialIndex >= 0);
		aiMaterial* material = scene.mMaterials[mesh->mMaterialIndex];

		auto generateTexture = [this, &scene, &material, &resultMesh](std::string name, aiTextureType type, eTextureBitFlag bitFlag)
		{
			std::vector<Texture> texture = loadMaterialTextures(material, type, name, scene);

			if (texture.size() > 0)
			{
				unsigned int key = static_cast<unsigned int>(bitFlag);

				resultMesh.TextureBitFlag.Value |= 1 << key;
				resultMesh.Textures[key] = texture[0];
			}
		};

		generateTexture("texture_diffuse", aiTextureType_DIFFUSE, eTextureBitFlag::Diffuse);
		generateTexture("texture_normals", aiTextureType_NORMALS, eTextureBitFlag::Normals);
		generateTexture("texture_specular", aiTextureType_SPECULAR, eTextureBitFlag::Specular);
		generateTexture("texture_specular", aiTextureType_OPACITY, eTextureBitFlag::Opacity);

		return resultMesh;
	}

	void ResourceManager::processAnimation(const aiScene* scene, Model* model) const
	{
		for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
		{
			auto totalFrame = scene->mAnimations[i]->mDuration;
			auto framePerSeconds = 1 / (scene->mAnimations[i]->mTicksPerSecond);

			model->Animation.Duration = totalFrame * framePerSeconds;
			for (unsigned int j = 0; j < scene->mAnimations[i]->mNumChannels; ++j)
			{
				AnimationNode animationNode;

				auto& currentChennel = scene->mAnimations[i]->mChannels[j];
				for (unsigned int k = 0; k < currentChennel->mNumPositionKeys; ++k)
				{
					KeyAnimation keyAnimation;

					keyAnimation.Time = currentChennel->mPositionKeys[k].mTime * framePerSeconds;

					keyAnimation.Position.x = currentChennel->mPositionKeys[k].mValue.x;
					keyAnimation.Position.y = currentChennel->mPositionKeys[k].mValue.y;
					keyAnimation.Position.z = currentChennel->mPositionKeys[k].mValue.z;

					keyAnimation.Rotation.x = currentChennel->mRotationKeys[k].mValue.x;
					keyAnimation.Rotation.y = currentChennel->mRotationKeys[k].mValue.y;
					keyAnimation.Rotation.z = currentChennel->mRotationKeys[k].mValue.z;
					keyAnimation.Rotation.w = currentChennel->mRotationKeys[k].mValue.w;

					keyAnimation.Scaling.x = currentChennel->mScalingKeys[k].mValue.x;
					keyAnimation.Scaling.y = currentChennel->mScalingKeys[k].mValue.y;
					keyAnimation.Scaling.z = currentChennel->mScalingKeys[k].mValue.z;

					animationNode.KeyAnimations.push_back(keyAnimation);
				}

				model->Animation.AnimationNodes.insert({ currentChennel->mNodeName.C_Str(), animationNode });
			}
		}
	}

	void ResourceManager::processBones(Model* model) const
	{
		assert(model != nullptr);

		std::map<std::string, const Node*> nodeMap;

		registerNodeRecursive(model->RootNode, &nodeMap);
		connectBoneNodeRecuresive(model->RootNode, nodeMap);
	}

	void ResourceManager::registerNodeRecursive(const Node* current, std::map<std::string, const Node*>* nodeMap) const
	{
		assert(current != nullptr);
		nodeMap->insert({ current->Name, current });

		for (auto* child : current->Children)
		{
			registerNodeRecursive(child, nodeMap);
		}
	}

	void ResourceManager::connectBoneNodeRecuresive(Node* current, const std::map<std::string, const Node*>& nodeMap) const
	{
		assert(current != nullptr);

		for (auto& mesh : current->Meshes)
		{
			for (auto& bone : mesh.Bones)
			{
				auto find = nodeMap.find(bone.Name);
				assert(find != nodeMap.end());

				bone.Node = find->second;
			}
		}

		for (auto* child : current->Children)
		{
			connectBoneNodeRecuresive(child, nodeMap);
		}
	}

	std::vector<Texture> ResourceManager::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene& scene) const
	{
		std::vector<Texture> textures;

		for (UINT i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			HRESULT hr;
			Texture texture;

			const aiTexture* embeddedTexture = scene.GetEmbeddedTexture(str.C_Str());

			if (embeddedTexture != nullptr)
			{
				texture.TextureRV = loadEmbeddedTexture(embeddedTexture);
			}
			else  // 내장 텍스처가 없으면 직접 찾아서 로딩한다
			{
				const char* fileName = strrchr(str.C_Str(), '\\') + 1;

				std::filesystem::path path = std::filesystem::current_path() / "..\\Resource\\Fbx" / fileName;
				hr = DirectX::CreateWICTextureFromFile(md3dDevice, path.c_str(), nullptr, &texture.TextureRV);

				if (FAILED(hr))
				{
					MessageBox(mhWnd, L"Texture couldn't be loaded", L"Error!", MB_ICONERROR | MB_OK);
				}
			}

			texture.Type = typeName;
			texture.Path = str.C_Str();
			textures.push_back(texture);
		}

		return textures;
	}

	ID3D11ShaderResourceView* ResourceManager::loadEmbeddedTexture(const aiTexture* embeddedTexture) const
	{
		HRESULT hr;
		ID3D11ShaderResourceView* texture = nullptr;

		if (embeddedTexture->mHeight != 0) // 높이가 0이면 압축된 형식을 의미한다.
		{
			// Load an uncompressed ARGB8888 embedded texture
			D3D11_TEXTURE2D_DESC desc;
			desc.Width = embeddedTexture->mWidth;
			desc.Height = embeddedTexture->mHeight;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			D3D11_SUBRESOURCE_DATA subresourceData;
			subresourceData.pSysMem = embeddedTexture->pcData;
			subresourceData.SysMemPitch = embeddedTexture->mWidth * 4;
			subresourceData.SysMemSlicePitch = embeddedTexture->mWidth * embeddedTexture->mHeight * 4;

			ID3D11Texture2D* texture2D = nullptr;
			hr = md3dDevice->CreateTexture2D(&desc, &subresourceData, &texture2D);
			if (FAILED(hr))
			{
				MessageBox(mhWnd, L"CreateTexture2D failed!", L"Error!", MB_ICONERROR | MB_OK);
			}

			hr = md3dDevice->CreateShaderResourceView(texture2D, nullptr, &texture);
			if (FAILED(hr))
			{
				MessageBox(mhWnd, L"CreateShaderResourceView failed!", L"Error!", MB_ICONERROR | MB_OK);
			}

			return texture;
		}

		// mHeight is 0, so try to load a compressed texture of mWidth bytes
		const size_t size = embeddedTexture->mWidth;

		hr = DirectX::CreateWICTextureFromMemory(md3dDevice, md3dImmediateContext, reinterpret_cast<const unsigned char*>(embeddedTexture->pcData), size, nullptr, &texture);

		if (FAILED(hr))
		{
			MessageBox(mhWnd, L"Texture couldn't be created from memory!", L"Error!", MB_ICONERROR | MB_OK);
		}

		return texture;
	}

	DirectX::SimpleMath::Matrix ResourceManager::convertMatrix(const aiMatrix4x4& aiMatrix) const
	{
		DirectX::SimpleMath::Matrix result;

		for (int i = 0; i < 4; ++i)
		{
			result.m[i][0] = aiMatrix[i][0];
			result.m[i][1] = aiMatrix[i][1];
			result.m[i][2] = aiMatrix[i][2];
			result.m[i][3] = aiMatrix[i][3];
		}

		return result;
	}

}