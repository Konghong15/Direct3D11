#include "Mesh.h"

namespace resourceManager
{
	void Mesh::Release()
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
}