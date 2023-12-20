#pragma once

#include <string>
#include <vector>
#include <d3d11.h>

#include "Animation.h"
#include "LightHelper.h"
#include "Vertex.h"

namespace resourceManager
{
	struct Subset
	{
		Subset() :
			Id(-1),
			VertexStart(0), VertexCount(0),
			FaceStart(0), FaceCount(0)
		{
		}

		UINT Id;
		UINT VertexStart;
		UINT VertexCount;
		UINT FaceStart;
		UINT FaceCount;
	};

	class Model
	{
	public:
		Model(ID3D11Device* d3dDevice, const std::string& fileName);
		~Model();

		void Draw(ID3D11DeviceContext* d3dContext);

	public:
		// material
		std::vector<common::Material> Materials;
		std::vector<ID3D11ShaderResourceView*> AlbedoSRVs;
		std::vector<ID3D11ShaderResourceView*> NormalSRVs;
		std::vector<ID3D11ShaderResourceView*> MetalnessSRVs;
		std::vector<ID3D11ShaderResourceView*> RoughnessSRVs;

		// mesh
		std::vector<vertex::PosNormalTexTan> Vertices;
		std::vector<UINT> Indices;
		ID3D11Buffer* VB;
		ID3D11Buffer* IB;
		DXGI_FORMAT IndexBufferFormat; // Always 32-bit
		UINT VertexStride;
		std::vector<Subset> SubsetTable;
	};
}