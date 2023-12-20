#pragma once

#include "D3dProcessor.h"

namespace resourceManager
{
	using namespace common;

	class D3DSample final : public D3DProcessor
	{
	public:
		D3DSample(HINSTANCE hInstance, UINT width, UINT height, std::wstring name);
		~D3DSample() override;

		bool Init() override;
		void Update(float deltaTime) override;
		void Render() override;
	};
}