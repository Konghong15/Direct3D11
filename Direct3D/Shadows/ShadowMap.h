#pragma once

#include <directxtk/SimpleMath.h>

#include "D3DUtil.h"

namespace shadows
{
	class ShadowMap
	{
	public:
		ShadowMap(ID3D11Device* device, UINT width, UINT height);
		~ShadowMap();

		ID3D11ShaderResourceView* DepthMapSRV();

		void BindDsvAndSetNullRenderTarget(ID3D11DeviceContext* dc);

	private:
		ShadowMap(const ShadowMap& rhs);
		ShadowMap& operator=(const ShadowMap& rhs);

	private:
		UINT mWidth;
		UINT mHeight;

		ID3D11ShaderResourceView* mDepthMapSRV;
		ID3D11DepthStencilView* mDepthMapDSV;

		D3D11_VIEWPORT mViewport;
	};
}