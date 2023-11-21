#pragma once

#include <directxtk/SimpleMath.h>

#include "D3dProcessor.h"
#include "D3DUtil.h"
#include "MathHelper.h"
#include "LightHelper.h"

namespace stenciling
{
	using namespace common;
	using namespace DirectX::SimpleMath;

	struct Vertex
	{

	};

	struct CBPerObject
	{

	};

	struct CBPerFrame
	{

	};

	struct Object
	{

	};

	class D3DSample final : public D3DProcessor
	{
	public:
		D3DSample(HINSTANCE hInstance, UINT width, UINT height, std::wstring name);

		bool Init();
		void OnResize();
		void Update(float deltaTime) override;
		void Render() override;

		void OnMouseDown(WPARAM btnState, int x, int y);
		void OnMouseUp(WPARAM btnState, int x, int y);
		void OnMouseMove(WPARAM btnState, int x, int y);

	private:
		void drawObject(const Object& object);
		void buildConstantBuffer();
		void buildShader();
		void buildInputLayout();

		void buildRoomGeometryBuffers();
		void buildSkullGeometryBuffers();

	private:
		CBPerObject mCBPerObject;
		CBPerFrame mCBPerFrame;
		ID3D11Buffer* mPerObjectCB;
		ID3D11Buffer* mPerFrameCB;

		ID3D11VertexShader* mVertexShader;
		ID3DBlob* mVertexShaderBlob;
		ID3D11PixelShader* mPixelShader;
		ID3D11InputLayout* mInputLayout;

		ID3D11SamplerState* mLinearSampler;

		ID3D11Buffer* mRoomVB;

		ID3D11Buffer* mSkullVB;
		ID3D11Buffer* mSkullIB;

		ID3D11ShaderResourceView* mFloorDiffuseMapSRV;
		ID3D11ShaderResourceView* mWallDiffuseMapSRV;
		ID3D11ShaderResourceView* mMirrorDiffuseMapSRV;

		DirectionLight mDirLights[3];
		Material mRoomMat;
		Material mSkullMat;
		Material mMirrorMat;
		Material mShadowMat;

		Matrix mRoomWorld;
		Matrix mSkullWorld;

		UINT mSkullIndexCount;
		Vector3 mSkullTranslation;

		Matrix mView;
		Matrix mProj;

		Vector3 mEyePosW; // 시점 의존적인 조명 처리를 위해서

		float mTheta;
		float mPhi;
		float mRadius;

		POINT mLastMousePos;
	};
}