#include <cassert>

#include "D3DSample.h"
#include "D3DUtil.h"

namespace drawing
{
	D3DSample::D3DSample(HINSTANCE hInstance, UINT width, UINT height, std::wstring name)
		: D3DProcessor(hInstance, width, height, name)
		, mBoxVB(nullptr)
		, mBoxIB(nullptr)
		, mBoxCB(nullptr)
		, mVertexShader(nullptr)
		, mPixelShader(nullptr)
		, mInputLayout(nullptr)
		, mWorld(DirectX::SimpleMath::Matrix::Identity)
		, mView(DirectX::SimpleMath::Matrix::Identity)
		, mProj(DirectX::SimpleMath::Matrix::Identity)
		, mTheta(1.5f * DirectX::XM_PI)
		, mPhi(0.25f * DirectX::XM_PI)
		, mRadius(5.f)
		, mLastMousePos({ 0, 0 })
	{
	}

	D3DSample::~D3DSample()
	{
		ReleaseCOM(mBoxVB);
		ReleaseCOM(mBoxIB);
		ReleaseCOM(mBoxCB);
		ReleaseCOM(mVertexShader);
		ReleaseCOM(mPixelShader);
		ReleaseCOM(mInputLayout);
	}

	bool D3DSample::Init()
	{
		if (!D3DProcessor::Init())
		{
			return false;
		}

		buildGeometryBuffers();
		buildShader();
		buildVertexLayout();

		return true;
	}


	void D3DSample::OnResize()
	{
		D3DProcessor::OnResize();

		mProj = DirectX::XMMatrixPerspectiveFovLH(DirectX::XM_PIDIV4, GetAspectRatio(), 1.f, 1000.f);
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

	void D3DSample::OnMouseDown(WPARAM btnState, int x, int y) {}
	void D3DSample::OnMouseUp(WPARAM btnState, int x, int y) {}
	void D3DSample::OnMouseMove(WPARAM btnState, int x, int y) {}

	void D3DSample::buildGeometryBuffers()
	{
		Vertex vertices[] =
		{
			{ {-1.0f, -1.0f, -1.0f }, common::White   },
			{ {-1.0f, +1.0f, -1.0f }, common::Black   },
			{ {+1.0f, +1.0f, -1.0f }, common::Red     },
			{ {+1.0f, -1.0f, -1.0f }, common::Green   },
			{ {-1.0f, -1.0f, +1.0f }, common::Blue    },
			{ {-1.0f, +1.0f, +1.0f }, common::Yellow  },
			{ {+1.0f, +1.0f, +1.0f }, common::Cyan    },
			{ {+1.0f, -1.0f, +1.0f }, common::Magenta }
		};

		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE; // 변경되지 않는 버퍼
		vbd.ByteWidth = sizeof(vertices); //  sizeof(Vertex) * 8;
		vbd.CPUAccessFlags = 0; // CPU 접근 read, write할지, 0은 암것도 안한다.
		vbd.MiscFlags = 0; // 기타 플래그
		vbd.StructureByteStride = 0; // 구조화된 버퍼?

		D3D11_SUBRESOURCE_DATA vinitData; // 초기값 설정을 위한 구조체
		vinitData.pSysMem = vertices;

		HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));
	}

	void D3DSample::buildShader() {}
	void D3DSample::buildVertexLayout() {}
}