#pragma once

#include <string>
#include <d3d11.h>

namespace resourceManager
{
	struct Texture
	{
	public:
		void Release();

	public:
		std::string Type;
		std::string Path;
		ID3D11ShaderResourceView* TextureRV = nullptr;
	};
}