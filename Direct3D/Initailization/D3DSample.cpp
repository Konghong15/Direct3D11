#include <cassert>

#include "D3DSample.h"

namespace initalization
{
	D3DSample::D3DSample(HINSTANCE hInstance, UINT width, UINT height, std::wstring name)
		: D3DProcessor(hInstance, width, height, name)
	{
	}

	void D3DSample::Update(float deltaTime)
	{

	}

	void D3DSample::Render()
	{
		assert(md3dImmediateContext);
		assert(mSwapChain);

		float color[] = { 1.0f, 1.0f, 0.0f, 1.0f };

		md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, color);
		md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		mSwapChain->Present(0, 0);
	}
}