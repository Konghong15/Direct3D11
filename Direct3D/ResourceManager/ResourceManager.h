#pragma once

#include <map>
#include <string>

#include <d3d11.h>

namespace resourceManager
{
	class Model;

	class ResourceManager
	{
	public:
		ResourceManager();
		~ResourceManager();

		static ResourceManager* GetInstance();
		static void DeleteInstance();

		Model* LoadModel(const std::string& fileName);
		ID3D11ShaderResourceView* LoadTexture(const std::string& fileName);

	private:
		static ResourceManager* mInstance;

		std::map<std::string, ID3D11ShaderResourceView*> mSRVs;
		std::map<std::string, Model*> mModels;
	};
}