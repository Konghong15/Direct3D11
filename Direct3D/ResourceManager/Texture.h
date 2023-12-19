#pragma once

struct Texture {
	std::string Type;
	std::string Path;
	ID3D11ShaderResourceView* TextureRV = nullptr;

	void Release()
	{
		Type = "";
		Path = "";

		if (TextureRV != nullptr)
		{
			TextureRV->Release();
			TextureRV = nullptr;
		}
	}
};
