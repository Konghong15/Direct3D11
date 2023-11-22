#pragma once

#include <cassert>
#include <vector>
#include <string>

#include <directxtk/SimpleMath.h>
#include <directxtk/DDSTextureLoader.h>
#include <directxtk/WICTextureLoader.h>

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
				HR(DirectX::CreateDDSTextureFromFileEx(device
					, filenames[i].c_str()
					, 0
					, D3D11_USAGE_STAGING
					, 0
					, D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ
					, 0
					, DirectX::DX11::DDS_LOADER_DEFAULT
					, (ID3D11Resource**)&srcTex[i]
					, nullptr));
			}

			D3D11_TEXTURE2D_DESC texElementDesc;
			srcTex[0]->GetDesc(&texElementDesc);

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
					D3D11_MAPPED_SUBRESOURCE mappedTex2D;
					HR(context->Map(srcTex[i], j, D3D11_MAP_READ, 0, &mappedTex2D));

					context->UpdateSubresource(texArray
						, D3D11CalcSubresource(j, i, texElementDesc.MipLevels)
						, 0
						, mappedTex2D.pData
						, mappedTex2D.RowPitch
						, mappedTex2D.DepthPitch);

					context->Unmap(srcTex[i], j);
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
	};
}