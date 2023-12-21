#include "ResourceManager.h"

#include <cassert>
#include <set>

#include <directxtk/WICTextureLoader.h>

#include "d3dUtil.h"
#include "Model.h"
#include "SkinnedModel.h"

namespace resourceManager
{
	ResourceManager* ResourceManager::mInstance = nullptr;

	ResourceManager* ResourceManager::GetInstance()
	{
		if (mInstance == nullptr)
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
			delete model.second;
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

		Model* model = new Model(md3dDevice, fileName);

		mModels.insert({ fileName, model });

		return model;
	}

	Model* ResourceManager::LoadModel(const std::wstring& fileName)
	{
		return LoadModel(common::D3DHelper::convert_from_wstring(fileName));
	}

	SkinnedModel* ResourceManager::LoadSkinnedModel(const std::string& fileName)
	{
		auto find = mSkinnedModels.find(fileName);

		if (find != mSkinnedModels.end())
		{
			return find->second;
		}

		SkinnedModel* model = new SkinnedModel(md3dDevice, fileName);

		mSkinnedModels.insert({ fileName, model });

		return model;
	}

	SkinnedModel* ResourceManager::LoadSkinnedModel(const std::wstring& fileName)
	{
		return LoadSkinnedModel(common::D3DHelper::convert_from_wstring(fileName));
	}

	ID3D11ShaderResourceView* ResourceManager::LoadTexture(const std::string& fileName)
	{
		return LoadTexture(common::D3DHelper::convert_to_wstring(fileName));
	}

	ID3D11ShaderResourceView* ResourceManager::LoadTexture(const std::wstring& fileName)
	{
		ID3D11ShaderResourceView* SRV = nullptr;

		auto find = mSRVs.find(common::D3DHelper::convert_from_wstring(fileName));

		if (find == mSRVs.end())
		{
			common::D3DHelper::CreateTextureFromFile(md3dDevice, fileName.c_str(), &SRV);

			mSRVs.insert({ common::D3DHelper::convert_from_wstring(fileName), SRV });
		}
		else
		{
			SRV = find->second;
		}

		return SRV;
	}
}