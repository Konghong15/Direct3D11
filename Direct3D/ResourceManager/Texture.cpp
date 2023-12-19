#include "Texture.h"

namespace resourceManager
{
	void Texture::Release()
	{
		Type = "";
		Path = "";

		if (TextureRV != nullptr)
		{
			TextureRV->Release();
			TextureRV = nullptr;
		}
	}
}