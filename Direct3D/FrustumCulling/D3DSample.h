#pragma once

#include <vector>
#include <directxtk/SimpleMath.h>

#include "D3dProcessor.h"
#include "Camera.h"

namespace initalization
{
	class Basic32;

	using namespace common;
	using namespace DirectX;
	using namespace SimpleMath;

	class D3DSample final : public D3DProcessor
	{
	public:
		D3DSample(HINSTANCE hInstance, UINT width, UINT height, std::wstring name);
		~D3DSample();

		bool Init();
		void OnResize();
		void Update(float deltaTime) override;
		void Render() override;

		void OnMouseDown(WPARAM btnState, int x, int y);
		void OnMouseUp(WPARAM btnState, int x, int y);
		void OnMouseMove(WPARAM btnState, int x, int y);

	private:
		void initBasic32();
		void buildSkullGeometry();

	private:
		enum { SKULL_COUNT_SQRT_3 = 10 };
		enum { INTERVAL = 50 };

		Basic32* mBasic32;
		Camera mCam;
		POINT mLastMousePos;

		ID3D11Buffer* mSkullVB;
		ID3D11Buffer* mSkullIB;
		BoundingBox mSkullBoundingBox;
		UINT mSkullIndexCount;

		std::vector<Matrix> mObjectWorlds;
		bool mbIsOnCulling;
	};
}