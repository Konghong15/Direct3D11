#pragma once

#include <directxtk/SimpleMath.h>
#include <Windows.h>

#include "D3dProcessor.h"

namespace drawing
{
	using namespace common;

	struct Vertex
	{
		DirectX::SimpleMath::Vector3 Position;
		DirectX::SimpleMath::Vector4 Color;
	};

	class D3DSample final : public D3DProcessor
	{
	public:
		D3DSample(HINSTANCE hInstance, UINT width, UINT height, std::wstring name);
		~D3DSample();

		bool Init() override;
		void OnResize() override;

		void Update(float deltaTime) override;
		void Render() override;

		void OnMouseDown(WPARAM btnState, int x, int y) override;
		void OnMouseUp(WPARAM btnState, int x, int y) override;
		void OnMouseMove(WPARAM btnState, int x, int y) override;

	private:
		void buildGeometryBuffers();
		void buildShader();
		void buildVertexLayout();

	private:
		ID3D11Buffer* mBoxVB;
		ID3D11Buffer* mBoxIB;
		ID3D11Buffer* mBoxCB;

		ID3D11VertexShader* mVertexShader;
		ID3D11PixelShader* mPixelShader;

		ID3D11InputLayout* mInputLayout;

		DirectX::SimpleMath::Matrix mWorld;
		DirectX::SimpleMath::Matrix mView;
		DirectX::SimpleMath::Matrix mProj;

		float mTheta;
		float mPhi;
		float mRadius;

		POINT mLastMousePos;
	};
}