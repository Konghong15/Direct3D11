#pragma once

#include <array>
#include <vector>
#include <string>

#include <d3d11.h>

#include "Vertex.h"
#include "TextureBitFlag.h"
#include "Texture.h"
#include "Bone.h"

namespace resourceManager
{
	struct MeshData
	{
		std::vector<Vertex> Vertices;
		std::vector<unsigned int> Indices;
	};

	struct Mesh
	{
	public:
		void Release();
		
	public:
		unsigned int VertexCount = 0;
		unsigned int IndexCount = 0;
		ID3D11Buffer* VertexBuffer = nullptr;
		ID3D11Buffer* IndexBuffer = nullptr;
		TextureBitFlag TextureBitFlag = { 0, };
		std::array<Texture, static_cast<size_t>(eTextureBitFlag::Size)> Textures;
		std::vector<Bone> Bones;
	};
}