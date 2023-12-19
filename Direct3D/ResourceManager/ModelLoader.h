#pragma once

#include <vector>
#include <map>
#include <string>

#include <Windows.h>
#include <d3d11.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Model.h"

class ModelLoader
{
public:
	ModelLoader(HWND hwnd, ID3D11Device* d3dDevice, ID3D11DeviceContext* d3dImmdiateContext);
	~ModelLoader();
	ModelLoader(const ModelLoader&) = delete;
	ModelLoader& operator=(const ModelLoader&) = delete;

	bool Load(std::string key, std::string fileName);

	inline Model* GetModelOrNull(std::string key) const;

private:
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
	HWND mhWnd;
	ID3D11Device* md3dDevice;
	ID3D11DeviceContext* md3dImmediateContext;

	std::map<std::string, Model*> mModels;
};

Model* ModelLoader::GetModelOrNull(std::string key) const
{
	auto find = mModels.find(key);

	return find == mModels.end() ? nullptr : find->second;
}