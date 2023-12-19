#pragma once

#include <array>
#include <vector>
#include <string>

#include <d3d11.h>

#include "Vertex.h"
#include "TextureBitFlag.h"
#include "Texture.h"
#include "Bone.h"

struct MeshData
{
	std::vector<Vertex> Vertices;
	std::vector<unsigned int> Indices;
};

struct Mesh
{
	unsigned int VertexCount = 0;
	unsigned int IndexCount = 0;
	ID3D11Buffer* VertexBuffer = nullptr;
	ID3D11Buffer* IndexBuffer = nullptr;
	TextureBitFlag TextureBitFlag = { 0, };
	std::array<Texture, static_cast<size_t>(eTextureBitFlag::Size)> Textures;
	std::vector<Bone> Bones;

	void Release()
	{
		VertexCount = 0;
		IndexCount = 0;
		TextureBitFlag = { 0, };

		if (VertexBuffer != nullptr)
		{
			VertexBuffer->Release();
			VertexBuffer = nullptr;
		}
		if (IndexBuffer != nullptr)
		{
			IndexBuffer->Release();
			IndexBuffer = nullptr;
		}
		for (auto& texture : Textures)
		{
			texture.Release();
		}

		Bones.clear();
	}
};