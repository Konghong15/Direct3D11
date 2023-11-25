#include <cassert>
#include <directxtk/WICTextureLoader.h>
#include <directxtk/DDSTextureLoader.h>

#include "D3DSample.h"
#include "D3DUtil.h"
#include "MathHelper.h"
#include "GeometryGenerator.h"

namespace texturing
{
	D3DSample::D3DSample(HINSTANCE hInstance, UINT width, UINT height, std::wstring name)
		: D3DProcessor(hInstance, width, height, name)
		, mEyePosW(0, 0, 0)
		, mTheta(1.3f * MathHelper::Pi)
		, mPhi(0.4f * MathHelper::Pi)
		, mRadius(2.5f)
	{
		mLastMousePos.x = 0;
		mLastMousePos.y = 0;

		mBoxWorld = Matrix::Identity;
		mTexTransform = Matrix::Identity;
		mView = Matrix::Identity;
		mProj = Matrix::Identity;

		mDirLights[0].Ambient = { 0.3f, 0.3f, 0.3f, 1.0f };
		mDirLights[0].Diffuse = { 0.8f, 0.8f, 0.8f, 1.0f };
		mDirLights[0].Specular = { 0.6f, 0.6f, 0.6f, 16.0f };
		mDirLights[0].Direction = { 0.707f, -0.707f, 0.0f };

		mDirLights[1].Ambient = { 0.2f, 0.2f, 0.2f, 1.0f };
		mDirLights[1].Diffuse = { 1.4f, 1.4f, 1.4f, 1.0f };
		mDirLights[1].Specular = { 0.3f, 0.3f, 0.3f, 16.0f };
		mDirLights[1].Direction = { -0.707f, 0.0f, 0.707f };

		mDirLights[2].Ambient = { 0.0f, 0.0f, 0.0f, 1.0f };
		mDirLights[2].Diffuse = { 0.2f, 0.2f, 0.2f, 1.0f };
		mDirLights[2].Specular = { 0.0f, 0.0f, 0.0f, 1.0f };
		mDirLights[2].Direction = { 0.0f, -0.707f, -0.707f };

		mBoxMat.Ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
		mBoxMat.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		mBoxMat.Specular = { 0.6f, 0.6f, 0.6f, 16.0f };

		mLandMat.Ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
		mLandMat.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		mLandMat.Specular = { 0.2f, 0.2f, 0.2f, 16.0f };

		mWavesMat.Ambient = { 0.5f, 0.5f, 0.5f, 1.0f };
		mWavesMat.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
		mWavesMat.Specular = { 0.8f, 0.8f, 0.8f, 32.0f };

		mCBPerFrame.bUseTexutre = true;
		mCBPerFrame.LigthCount = 2;
	}

	D3DSample::~D3DSample()
	{
		ReleaseCOM(mPerObjectCB);
		ReleaseCOM(mPerFrameCB);

		ReleaseCOM(mBoxVB);
		ReleaseCOM(mBoxIB);

		ReleaseCOM(mLandVB);
		ReleaseCOM(mLandIB);

		ReleaseCOM(mWavesVB);
		ReleaseCOM(mWavesIB);

		ReleaseCOM(mGrassMapSRV);
		ReleaseCOM(mWavesMapSRV);
		ReleaseCOM(mDiffuseMapSRV);
		ReleaseCOM(mLinearSampleState);

		ReleaseCOM(mVertexShader);
		ReleaseCOM(mVertexShaderBuffer);
		ReleaseCOM(mPixelShader);
		ReleaseCOM(mInputLayout);
	}

	bool D3DSample::Init()
	{
		if (!D3DProcessor::Init())
		{
			return false;
		}

		HR(DirectX::CreateDDSTextureFromFile(md3dDevice, L"../Resource/Textures/WoodCrate01.dds", NULL, &mDiffuseMapSRV));
		HR(DirectX::CreateDDSTextureFromFile(md3dDevice, L"../Resource/Textures/grass.dds", NULL, &mGrassMapSRV));
		HR(DirectX::CreateDDSTextureFromFile(md3dDevice, L"../Resource/Textures/water2.dds", NULL, &mWavesMapSRV));

		buildGeometryBuffers();
		buildShader();
		buildVertexLayout();
		buildConstantBuffer();
		buildLandGeometryBuffers();
		buildWaveGeometryBuffers();

		return true;
	}
	void D3DSample::OnResize()
	{
		D3DProcessor::OnResize();

		mProj = DirectX::XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, GetAspectRatio(), 1.f, 1000.f);
	}

	void D3DSample::Update(float deltaTime)
	{
		float x = mRadius * sinf(mPhi) * cosf(mTheta);
		float z = mRadius * sinf(mPhi) * sinf(mTheta);
		float y = mRadius * cosf(mPhi);

		mEyePosW = { x, y, z }; // ���� ��ġ

		Vector4 pos = { x, y, z, 1.0f };
		Vector4 target = Vector4::Zero;
		Vector4 up = { 0.0f, 1.0f, 0.0f, 0.0f };

		mView = DirectX::XMMatrixLookAtLH(pos, target, up);

		static float t_base = 0.0f;
		if ((mTimer.GetTotalTime() - t_base) >= 0.25f)
		{
			t_base += 0.25f;

			DWORD i = 5 + rand() % (mWaves.GetRowCount() - 10);
			DWORD j = 5 + rand() % (mWaves.GetColumnCount() - 10);

			float r = MathHelper::RandF(1.0f, 2.0f);

			mWaves.Disturb(i, j, r);
		}

		mWaves.Update(deltaTime);

		D3D11_MAPPED_SUBRESOURCE mappedData;
		HR(md3dImmediateContext->Map(mWavesVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));

		Vertex* v = reinterpret_cast<Vertex*>(mappedData.pData);
		for (UINT i = 0; i < mWaves.GetVertexCount(); ++i)
		{
			v[i].Pos = mWaves[i];
			v[i].Normal = mWaves.GetNormal(i);

			// Derive tex-coords in [0,1] from position.
			v[i].Tex.x = 0.5f + mWaves[i].x / mWaves.GetWidth();
			v[i].Tex.y = 0.5f - mWaves[i].z / mWaves.GetDepth();
		}

		md3dImmediateContext->Unmap(mWavesVB, 0);

		mWaterTexOffset.y += 0.5f * deltaTime;
		mWaterTexOffset.x += 1.f * deltaTime;
		mWaterTexTransform = Matrix::CreateScale(5.f, 5.f, 0.f) * Matrix::CreateTranslation(mWaterTexOffset.x, mWaterTexOffset.y, 0.f);
	}

	void D3DSample::Render()
	{
		assert(md3dImmediateContext);
		assert(mSwapChain);

		float color[] = { 1.0f, 1.0f, 0.0f, 1.0f };

		md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, color);
		md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		md3dImmediateContext->IASetInputLayout(mInputLayout);

		md3dImmediateContext->VSSetShader(mVertexShader, NULL, 0);

		md3dImmediateContext->PSSetShader(mPixelShader, NULL, 0);
		md3dImmediateContext->PSSetSamplers(0, 1, &mLinearSampleState);

		md3dImmediateContext->VSSetConstantBuffers(0, 1, &mPerObjectCB);

		md3dImmediateContext->PSSetConstantBuffers(0, 1, &mPerFrameCB);
		md3dImmediateContext->PSSetConstantBuffers(1, 1, &mPerObjectCB);

		memcpy(mCBPerFrame.DirLight, mDirLights, sizeof(mCBPerFrame.DirLight));
		mCBPerFrame.EyePosW = mEyePosW;

		md3dImmediateContext->UpdateSubresource(mPerFrameCB, 0, NULL, &mCBPerFrame, 0, 0);

		Object object;

		object = { mWavesVB, mWavesIB, mWaves.GetTriangleCount() * 3, mWavesMapSRV, &mWavesWorld, &mWaterTexTransform, &mWavesMat };
		drawObject(object);

		object = { mLandVB, mLandIB, mLandIndexCount, mGrassMapSRV, &mLandWorld, &mGrassTexTransform, &mLandMat };
		drawObject(object);

		Matrix identity = Matrix::Identity;
		object = { mBoxVB, mBoxIB, mBoxIndexCount, mDiffuseMapSRV, &mBoxWorld, &identity, &mBoxMat };
		drawObject(object);

		mSwapChain->Present(0, 0);
	}

	void D3DSample::drawObject(const Object& object)
	{
		UINT stride = sizeof(Vertex);
		UINT offset = 0;

		md3dImmediateContext->IASetVertexBuffers(0, 1, &object.VertexBuffer, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(object.IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		mCBPerObject.World = *object.WorldMat;
		mCBPerObject.WorldInvTranspose = MathHelper::InverseTranspose(*object.WorldMat);
		mCBPerObject.Tex = *object.TextureMat;
		mCBPerObject.WorldViewProj = *object.WorldMat * mView * mProj;

		mCBPerObject.World = mCBPerObject.World.Transpose();
		mCBPerObject.WorldInvTranspose = mCBPerObject.WorldInvTranspose.Transpose();
		mCBPerObject.Tex = mCBPerObject.Tex.Transpose();
		mCBPerObject.WorldViewProj = mCBPerObject.WorldViewProj.Transpose();

		mCBPerObject.Material = *object.Material;

		md3dImmediateContext->UpdateSubresource(mPerObjectCB, 0, NULL, &mCBPerObject, 0, 0);

		md3dImmediateContext->PSSetShaderResources(0, 1, &object.SRV);

		md3dImmediateContext->DrawIndexed(object.IndexCount, 0, 0);
	}

	void D3DSample::OnMouseDown(WPARAM btnState, int x, int y)
	{
		mLastMousePos.x = x;
		mLastMousePos.y = y;

		SetCapture(mhWnd);
	}

	void D3DSample::OnMouseUp(WPARAM btnState, int x, int y)
	{
		ReleaseCapture();
	}

	void D3DSample::OnMouseMove(WPARAM btnState, int x, int y)
	{
		if ((btnState & MK_LBUTTON) != 0)
		{
			// Make each pixel correspond to a quarter of a degree.
			float dx = DirectX::XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
			float dy = DirectX::XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

			// Update angles based on input to orbit camera around box.
			mTheta += dx;
			mPhi += dy;

			// Restrict the angle mPhi.
			mPhi = common::MathHelper::Clamp(mPhi, 0.1f, DirectX::XM_PI - 0.1f);
		}
		else if ((btnState & MK_RBUTTON) != 0)
		{
			// Make each pixel correspond to 0.005 unit in the scene.
			float dx = 0.05f * static_cast<float>(x - mLastMousePos.x);
			float dy = 0.05f * static_cast<float>(y - mLastMousePos.y);

			// Update the camera radius based on input.
			mRadius += dx - dy;

			// Restrict the radius.
			mRadius = common::MathHelper::Clamp(mRadius, 3.0f, 200.f);
		}

		mLastMousePos.x = x;
		mLastMousePos.y = y;
	}

	void D3DSample::buildGeometryBuffers()
	{
		GeometryGenerator::MeshData box;

		GeometryGenerator geoGen;
		geoGen.CreateBox(1.0f, 1.0f, 1.0f, &box);

		mBoxVertexOffset = 0;
		mBoxIndexCount = box.Indices.size();
		mBoxIndexOffset = 0;

		UINT totalVertexCount = box.Vertices.size();
		UINT totalIndexCount = mBoxIndexCount;

		std::vector<Vertex> vertices(totalVertexCount);

		UINT k = 0;
		for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
		{
			vertices[k].Pos = box.Vertices[i].Position;
			vertices[k].Normal = box.Vertices[i].Normal;
			vertices[k].Tex = box.Vertices[i].TexC;
		}

		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex) * totalVertexCount;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA vinitData;
		vinitData.pSysMem = &vertices[0];
		HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));

		std::vector<UINT> indices;
		indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());

		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = &indices[0];
		HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));
	}

	void D3DSample::buildShader()
	{
		D3D11_SAMPLER_DESC sampleDesc = {};
		sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampleDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampleDesc.MinLOD = 0;
		sampleDesc.MaxLOD = D3D11_FLOAT32_MAX;

		HR(md3dDevice->CreateSamplerState(&sampleDesc, &mLinearSampleState));

		HR(CompileShaderFromFile(L"../Resource/Shader/TexturingVS.hlsl", "main", "vs_5_0", &mVertexShaderBuffer));

		HR(md3dDevice->CreateVertexShader(
			mVertexShaderBuffer->GetBufferPointer(),
			mVertexShaderBuffer->GetBufferSize(),
			NULL,
			&mVertexShader));

		ID3DBlob* pixelShaderBuffer = nullptr;
		HR(CompileShaderFromFile(L"../Resource/Shader/TexturingPS.hlsl", "main", "ps_5_0", &pixelShaderBuffer));

		HR(md3dDevice->CreatePixelShader(
			pixelShaderBuffer->GetBufferPointer(),
			pixelShaderBuffer->GetBufferSize(),
			NULL,
			&mPixelShader));

		ReleaseCOM(pixelShaderBuffer);
	}

	void D3DSample::buildVertexLayout()
	{
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		HR(md3dDevice->CreateInputLayout(
			inputElementDesc,
			ARRAYSIZE(inputElementDesc),
			mVertexShaderBuffer->GetBufferPointer(),
			mVertexShaderBuffer->GetBufferSize(),
			&mInputLayout
		));
	}

	void D3DSample::buildConstantBuffer()
	{
		D3D11_BUFFER_DESC cbd;
		cbd.Usage = D3D11_USAGE_DEFAULT;
		static_assert(sizeof(CBPerObject) % 16 == 0, "must be align");
		cbd.ByteWidth = sizeof(CBPerObject);
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = 0;
		cbd.MiscFlags = 0;
		HR(md3dDevice->CreateBuffer(&cbd, NULL, &mPerObjectCB));

		static_assert(sizeof(CBPerFrame) % 16 == 0, "must be align");
		cbd.ByteWidth = sizeof(CBPerFrame);
		HR(md3dDevice->CreateBuffer(&cbd, NULL, &mPerFrameCB));
	}

	float D3DSample::getHillHeight(float x, float z) const
	{
		return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
	}

	Vector3 D3DSample::getHillNormal(float x, float z) const
	{
		Vector3 n(
			-0.03f * z * cosf(0.1f * x) - 0.3f * cosf(0.1f * z),
			1.0f,
			-0.3f * sinf(0.1f * x) + 0.03f * x * sinf(0.1f * z));
		n.Normalize();

		return n;
	}

	void D3DSample::buildLandGeometryBuffers()
	{
		GeometryGenerator::MeshData grid;

		GeometryGenerator geoGen;

		geoGen.CreateGrid(160.0f, 160.0f, 50, 50, &grid);

		mLandIndexCount = grid.Indices.size();

		std::vector<Vertex> vertices(grid.Vertices.size());
		for (size_t i = 0; i < grid.Vertices.size(); ++i)
		{
			Vector3 p = grid.Vertices[i].Position;

			p.y = getHillHeight(p.x, p.z);

			vertices[i].Pos = p;
			vertices[i].Normal = getHillNormal(p.x, p.z);
			vertices[i].Tex = grid.Vertices[i].TexC;
		}

		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex) * grid.Vertices.size();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA vinitData;
		vinitData.pSysMem = &vertices[0];
		HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mLandVB));

		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * mLandIndexCount;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = &grid.Indices[0];
		HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mLandIB));
	}

	void D3DSample::buildWaveGeometryBuffers()
	{
		mWaves.Init(160, 160, 1.0f, 0.03f, 3.25f, 0.4f);

		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_DYNAMIC;
		vbd.ByteWidth = sizeof(Vertex) * mWaves.GetVertexCount();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vbd.MiscFlags = 0;
		HR(md3dDevice->CreateBuffer(&vbd, 0, &mWavesVB));


		std::vector<UINT> indices(3 * mWaves.GetTriangleCount()); // 3 indices per face

		UINT m = mWaves.GetRowCount();
		UINT n = mWaves.GetColumnCount();
		int k = 0;
		for (UINT i = 0; i < m - 1; ++i)
		{
			for (DWORD j = 0; j < n - 1; ++j)
			{
				indices[k] = i * n + j;
				indices[k + 1] = i * n + j + 1;
				indices[k + 2] = (i + 1) * n + j;

				indices[k + 3] = (i + 1) * n + j;
				indices[k + 4] = i * n + j + 1;
				indices[k + 5] = (i + 1) * n + j + 1;

				k += 6; // next quad
			}
		}

		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * indices.size();
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = &indices[0];
		HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mWavesIB));
	}
}