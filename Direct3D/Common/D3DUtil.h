#pragma once

#include <cassert>
#include <vector>
#include <string>

#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <directxtk/SimpleMath.h>
#include <directxtk/DDSTextureLoader.h>
#include <directxtk/WICTextureLoader.h>

#include "MathHelper.h"

#pragma comment(lib, "d3dcompiler.lib")

#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)                                               \
	{                                                       \
		HRESULT hr = (x);                                   \
		assert(SUCCEEDED(hr));								\
	}
#endif

#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 

#define ReleaseCOM(x) { if(x){ x->Release(); x = 0; } }
#define SafeDelete(x) { delete x; x = 0; }

namespace common
{
	constexpr DirectX::SimpleMath::Color White = { 1.0f, 1.0f, 1.0f, 1.0f };
	constexpr DirectX::SimpleMath::Color Black = { 0.0f, 0.0f, 0.0f, 1.0f };
	constexpr DirectX::SimpleMath::Color Red = { 1.0f, 0.0f, 0.0f, 1.0f };
	constexpr DirectX::SimpleMath::Color Green = { 0.0f, 1.0f, 0.0f, 1.0f };
	constexpr DirectX::SimpleMath::Color Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	constexpr DirectX::SimpleMath::Color Yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
	constexpr DirectX::SimpleMath::Color Cyan = { 0.0f, 1.0f, 1.0f, 1.0f };
	constexpr DirectX::SimpleMath::Color Magenta = { 1.0f, 0.0f, 1.0f, 1.0f };
	constexpr DirectX::SimpleMath::Color Silver = { 0.75f, 0.75f, 0.75f, 1.0f };
	constexpr DirectX::SimpleMath::Color LightSteelBlue = { 0.69f, 0.77f, 0.87f, 1.0f };

	class D3DHelper
	{
	public:
		static ID3D11ShaderResourceView* CreateTexture2DArraySRV(ID3D11Device* device
			, ID3D11DeviceContext* context
			, const std::vector<std::wstring>& filenames)
		{
			assert(device != nullptr);
			assert(context != nullptr);
			assert(filenames.size() != 0);

			const UINT SIZE = filenames.size();
			std::vector<ID3D11Texture2D*> srcTex(SIZE);

			for (UINT i = 0; i < SIZE; ++i)
			{
				HR(DirectX::CreateDDSTextureFromFile(device
					, filenames[i].c_str()
					, (ID3D11Resource**)&srcTex[i]
					, nullptr));
			}

			D3D11_TEXTURE2D_DESC texElementDesc;
			srcTex[0]->GetDesc(&texElementDesc);

			// 첫번째 텍스처는 압축 텍스처라서 형식이 잘못되어서 이상하게 나온 거구나.. 압축 텍스처 풀려면 디테일한 설정해서 로딩해야 한다.
			// assert(memcmp(&texElementDesc, &texElementDesc1, sizeof(texElementDesc)));
			// assert(memcmp(&texElementDesc, &texElementDesc2, sizeof(texElementDesc)));
			// assert(memcmp(&texElementDesc, &texElementDesc3, sizeof(texElementDesc)));

			D3D11_TEXTURE2D_DESC texArrayDesc;
			texArrayDesc.Width = texElementDesc.Width;
			texArrayDesc.Height = texElementDesc.Height;
			texArrayDesc.MipLevels = texElementDesc.MipLevels;
			texArrayDesc.ArraySize = SIZE;
			texArrayDesc.Format = texElementDesc.Format;
			texArrayDesc.SampleDesc.Count = 1;
			texArrayDesc.SampleDesc.Quality = 0;
			texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
			texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texArrayDesc.CPUAccessFlags = 0;
			texArrayDesc.MiscFlags = 0;

			ID3D11Texture2D* texArray = 0;
			HR(device->CreateTexture2D(&texArrayDesc, 0, &texArray));

			for (UINT i = 0; i < SIZE; ++i)
			{
				for (UINT j = 0; j < texElementDesc.MipLevels; ++j)
				{
					context->CopySubresourceRegion(texArray, D3D11CalcSubresource(j, i, texElementDesc.MipLevels), 0, 0, 0, srcTex[i], j, NULL);
				}
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
			viewDesc.Format = texArrayDesc.Format;
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Texture2DArray.MostDetailedMip = 0;
			viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
			viewDesc.Texture2DArray.FirstArraySlice = 0;
			viewDesc.Texture2DArray.ArraySize = SIZE;

			ID3D11ShaderResourceView* texArraySRV = nullptr;
			HR(device->CreateShaderResourceView(texArray, &viewDesc, &texArraySRV));

			ReleaseCOM(texArray);

			for (UINT i = 0; i < SIZE; ++i)
			{
				ReleaseCOM(srcTex[i]);
			}

			return texArraySRV;
		}

		static HRESULT CreateTextureFromFile(ID3D11Device* d3dDevice, const wchar_t* szFileName, ID3D11ShaderResourceView** textureView)
		{
			HRESULT hr = S_OK;

			hr = DirectX::CreateDDSTextureFromFile(d3dDevice, szFileName, nullptr, textureView);
			if (FAILED(hr))
			{
				hr = DirectX::CreateWICTextureFromFile(d3dDevice, szFileName, nullptr, textureView);
				if (FAILED(hr))
				{
					return hr;
				}
			}
			return S_OK;
		}

		static HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
		{
			HRESULT hr = S_OK;

			DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
			// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
			// Setting this flag improves the shader debugging experience, but still allows 
			// the shaders to be optimized and to run exactly the way they will run in 
			// the release configuration of this program.
			dwShaderFlags |= D3DCOMPILE_DEBUG;

			// Disable optimizations to further improve shader debugging
			dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

			ID3DBlob* pErrorBlob = nullptr;
			hr = D3DCompileFromFile(szFileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderModel,
				dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
			if (FAILED(hr))
			{
				if (pErrorBlob)
				{
					MessageBoxA(NULL, (char*)pErrorBlob->GetBufferPointer(), "CompileShaderFromFile", MB_OK);
					pErrorBlob->Release();
				}
				return hr;
			}
			if (pErrorBlob) pErrorBlob->Release();

			return S_OK;
		}

		static void ExtractFrustumPlanes(DirectX::SimpleMath::Vector4 planes[6], DirectX::SimpleMath::Matrix M)
		{
			using namespace DirectX::SimpleMath;

			//
			// Left
			//
			planes[0].x = M(0, 3) + M(0, 0);
			planes[0].y = M(1, 3) + M(1, 0);
			planes[0].z = M(2, 3) + M(2, 0);
			planes[0].w = M(3, 3) + M(3, 0);

			//
			// Right
			//
			planes[1].x = M(0, 3) - M(0, 0);
			planes[1].y = M(1, 3) - M(1, 0);
			planes[1].z = M(2, 3) - M(2, 0);
			planes[1].w = M(3, 3) - M(3, 0);

			//
			// Bottom
			//
			planes[2].x = M(0, 3) + M(0, 1);
			planes[2].y = M(1, 3) + M(1, 1);
			planes[2].z = M(2, 3) + M(2, 1);
			planes[2].w = M(3, 3) + M(3, 1);

			//
			// Top
			//
			planes[3].x = M(0, 3) - M(0, 1);
			planes[3].y = M(1, 3) - M(1, 1);
			planes[3].z = M(2, 3) - M(2, 1);
			planes[3].w = M(3, 3) - M(3, 1);

			//
			// Near
			//
			planes[4].x = M(0, 2);
			planes[4].y = M(1, 2);
			planes[4].z = M(2, 2);
			planes[4].w = M(3, 2);

			//
			// Far
			//
			planes[5].x = M(0, 3) - M(0, 2);
			planes[5].y = M(1, 3) - M(1, 2);
			planes[5].z = M(2, 3) - M(2, 2);
			planes[5].w = M(3, 3) - M(3, 2);

			// Normalize the plane equations.
			for (int i = 0; i < 6; ++i)
			{
				planes[i].Normalize();
			}
		}

		static ID3D11ShaderResourceView* CreateRandomTexture1DSRV(ID3D11Device* device)
		{
			using namespace DirectX::SimpleMath;

			Vector4 randomValues[1024];

			for (int i = 0; i < 1024; ++i)
			{
				randomValues[i].x = MathHelper::RandF(-1.0f, 1.0f);
				randomValues[i].y = MathHelper::RandF(-1.0f, 1.0f);
				randomValues[i].z = MathHelper::RandF(-1.0f, 1.0f);
				randomValues[i].w = MathHelper::RandF(-1.0f, 1.0f);
			}

			D3D11_SUBRESOURCE_DATA initData;
			initData.pSysMem = randomValues;
			initData.SysMemPitch = 1024 * sizeof(Vector4);
			initData.SysMemSlicePitch = 0;

			//
			// Create the texture.
			//
			D3D11_TEXTURE1D_DESC texDesc;
			texDesc.Width = 1024;
			texDesc.MipLevels = 1;
			texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			texDesc.Usage = D3D11_USAGE_IMMUTABLE;
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texDesc.CPUAccessFlags = 0;
			texDesc.MiscFlags = 0;
			texDesc.ArraySize = 1;

			ID3D11Texture1D* randomTex = 0;
			HR(device->CreateTexture1D(&texDesc, &initData, &randomTex));

			//
			// Create the resource view.
			//
			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
			viewDesc.Format = texDesc.Format;
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
			viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
			viewDesc.Texture1D.MostDetailedMip = 0;

			ID3D11ShaderResourceView* randomTexSRV = 0;
			HR(device->CreateShaderResourceView(randomTex, &viewDesc, &randomTexSRV));

			ReleaseCOM(randomTex);

			return randomTexSRV;
		}

		static std::wstring ToWString(const std::string& s)
		{
			std::wstring wsTmp(s.begin(), s.end());
			return wsTmp;
		}
	};
}