#pragma once

#include <map>
#include <string>
#include <Windows.h>
#include <d3d11.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace resourceManager
{
	struct Model;
	struct Node;

	class ResourceManager
	{
	public:
		static ResourceManager* GetInstance();
		static void DeleteInstance();

		void Init(ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dContext);

		Model* LoadModel(const std::string& fileName);
		ID3D11ShaderResourceView* LoadTexture(const std::string& fileName);

	private:
		ResourceManager() = default;
		~ResourceManager();

		void processNodeRecursive(aiNode* node, const aiScene& scene, Node* parent) const;
		Mesh processMesh(aiMesh* mesh, const aiScene& scene) const;
		void processAnimation(const aiScene* scene, Model* model) const;
		void processBones(Model* model) const;

		std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene& scene) const;
		ID3D11ShaderResourceView* loadEmbeddedTexture(const aiTexture* embeddedTexture) const;

		void registerNodeRecursive(const Node* current, std::map<std::string, const Node*>* nodeMap) const;
		void connectBoneNodeRecuresive(Node* current, const std::map<std::string, const Node*>& nodeMap) const;

		DirectX::SimpleMath::Matrix convertMatrix(const aiMatrix4x4& aiMatrix) const;


	private:
		static ResourceManager* mInstance;

		ID3D11Device* md3dDevice;
		ID3D11DeviceContext* md3dContext;

		std::map<std::string, ID3D11ShaderResourceView*> mSRVs;
		std::map<std::string, Model*> mModels;
	};
}