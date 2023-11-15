#pragma once

#include "directxtk/SimpleMath.h"

#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)                                              \
	{                                                          \
		HRESULT hr = (x);                                      \
		if(FAILED(hr))                                         \
		{                                                      \
			DXTrace(__FILE__, (DWORD)__LINE__, hr, L#x, true); \
		}                                                      \
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
}