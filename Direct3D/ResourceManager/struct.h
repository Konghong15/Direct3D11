#pragma once

#include <vector>
#include <string>
#include <map>

#include <d3d11.h>
#include <directxtk/SimpleMath.h>

struct Texture {
	std::string Type;
	std::string Path;
	ID3D11ShaderResourceView* TextureRV;
};

enum class eTextureBitFlag
{
	Diffuse,
	Normals,
	Specular,
	Opacity,
	Size
};

union TextureBitFlag
{
	unsigned int Value;
	struct
	{
		unsigned char BHasDiffuseMap : 1;
		unsigned char BHasNormalMap : 1;
		unsigned char BHasSpecularMap : 1;
		unsigned char BHasOpacityMap : 1;
	};
};

struct Vertex
{
	DirectX::SimpleMath::Vector4 Position;
	DirectX::SimpleMath::Vector3 Normal;
	DirectX::SimpleMath::Vector3 Tangent;
	DirectX::SimpleMath::Vector3 Binormal;
	DirectX::SimpleMath::Vector2 UV;
};

struct VSConstantBufferInfo
{
	DirectX::SimpleMath::Matrix WorldTransform;
	DirectX::SimpleMath::Matrix WVPTransform;
	DirectX::SimpleMath::Vector4 CameraPosition;
};

struct PSConstantBufferInfo
{
	union LightBitFlag
	{
		unsigned int Value;
		struct
		{
			unsigned char BIsAppliedAmbient : 1;
			unsigned char BIsAppliedDiffuse : 1;
			unsigned char BIsAppliedSpecular : 1;
			unsigned char BIsAppliedNormalMap : 1;
			unsigned char BIsAppliedSpecularMap : 1;
		};
	};

	DirectX::SimpleMath::Vector4 LightDirection;
	DirectX::SimpleMath::Vector4 LightColor;
	unsigned int SpecularPow;
	LightBitFlag LightBitFlag;
	TextureBitFlag TextureBitFlag;
	unsigned int unused[1];
};

struct Mesh
{
	std::vector<Vertex> Vertices;
	std::vector<UINT> Indices;
	std::map<unsigned int, Texture> Texture;
	TextureBitFlag TextureBitFlag;

	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;

	void Init(ID3D11Device* dev)
	{
		HRESULT hr;

		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = static_cast<UINT>(sizeof(Vertex) * Vertices.size());
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = &Vertices[0];

		hr = dev->CreateBuffer(&vbd, &initData, &VertexBuffer);
		if (FAILED(hr)) {
			assert(false);
		}

		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = static_cast<UINT>(sizeof(UINT) * Indices.size());
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;

		initData.pSysMem = &Indices[0];

		hr = dev->CreateBuffer(&ibd, &initData, &IndexBuffer);
		if (FAILED(hr)) {
			assert(false);
		}
	}

	void Draw(ID3D11DeviceContext* devcon) const
	{
		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		devcon->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
		devcon->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		auto applyTexture = [&](eTextureBitFlag textureBitFlag)
			{
				auto find = Texture.find(static_cast<int>(textureBitFlag));
				if (find != Texture.end())
				{
					devcon->PSSetShaderResources(static_cast<int>(textureBitFlag), 1, &(find->second.TextureRV));
				}
			};

		for (int i = 0; i < static_cast<int>(eTextureBitFlag::Size); ++i)
		{
			applyTexture(static_cast<eTextureBitFlag>(i));
		}

		devcon->DrawIndexed(static_cast<UINT>(Indices.size()), 0, 0);
	}
};