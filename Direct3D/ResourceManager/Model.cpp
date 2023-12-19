#include "Model.h"

#include "struct.h"
#include "Node.h"

namespace resourceManager
{
	void Model::Release()
	{
		RootNode->Release();
		delete RootNode;
		RootNode = nullptr;
	}

	void Model::Draw(ID3D11DeviceContext* d3dImmediateContext, ModelDrawInfo& modelDrawInfo, double progressTime)
	{
		RootNode->UpdateRecursive(progressTime, Animation);
		RootNode->DrawRecursive(d3dImmediateContext, modelDrawInfo);
	}
}