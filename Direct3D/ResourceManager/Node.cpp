#include "Node.h"

#include "ModelDrawInfo.h"

namespace resourceManager
{
	void Node::Release()
	{
		Parent = nullptr;
		Name = "";
		LocalMatrix = DirectX::SimpleMath::Matrix::Identity;
		WorldMatrix = DirectX::SimpleMath::Matrix::Identity;
		AnimationNodeIndex = INVALID_INDEX;

		for (auto& mesh : Meshes)
		{
			mesh.Release();
		}
		Meshes.clear();
		for (auto* child : Children)
		{
			child->Release();
			delete child;
		}
		Children.clear();
	}

	void Node::UpdateRecursive(double progressTime, const Animation& animation)
	{
		const auto& find = animation.AnimationNodes.find(Name);

		if (find != animation.AnimationNodes.end())
		{
			const auto& animationNode = find->second;
			assert(animation.Duration != 0.0);
			LocalMatrix = animationNode.Evaluate(fmod(progressTime, animation.Duration));
		}
		else
		{
			LocalMatrix = BindPose;
		}

		if (Parent == nullptr)
		{
			WorldMatrix = LocalMatrix;
		}
		else
		{
			WorldMatrix = LocalMatrix * Parent->WorldMatrix;
		}

		for (auto* child : Children)
		{
			child->UpdateRecursive(progressTime, animation);
		}
	}

	void Node::DrawRecursive(ID3D11DeviceContext* d3dImmediateContext, ModelDrawInfo& modelDrawInfo)
	{
		for (size_t i = 0; i < Meshes.size(); ++i)
		{
			UINT stride = sizeof(Vertex);
			UINT offset = 0;

			d3dImmediateContext->IASetVertexBuffers(0, 1, &Meshes[i].VertexBuffer, &stride, &offset);
			d3dImmediateContext->IASetIndexBuffer(Meshes[i].IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

			modelDrawInfo.VSConstantBufferInfo.WorldTransform = WorldMatrix.Transpose();
			size_t j;
			for (j = 0; j < Meshes[i].Bones.size(); ++j)
			{
				DirectX::SimpleMath::Matrix currentMatirx = Meshes[i].Bones[j].OffsetMatrix * Meshes[i].Bones[j].Node->WorldMatrix;
				modelDrawInfo.VSConstantBufferInfo.BoneTransform[j] = currentMatirx.Transpose();
			}

			modelDrawInfo.PSConstantBufferInfo.TextureBitFlag = Meshes[i].TextureBitFlag;

			d3dImmediateContext->UpdateSubresource(modelDrawInfo.VSConstantBuffer, 0, nullptr, &modelDrawInfo.VSConstantBufferInfo, 0, 0);
			d3dImmediateContext->UpdateSubresource(modelDrawInfo.PSConstantBuffer, 0, nullptr, &modelDrawInfo.PSConstantBufferInfo, 0, 0);

			for (size_t j = 0; j < static_cast<size_t>(eTextureBitFlag::Size); ++j)
			{
				if ((Meshes[i].TextureBitFlag.Value >> j) & 1)
				{
					auto textureRV = Meshes[i].Textures[j].TextureRV;
					assert(textureRV != nullptr);
					d3dImmediateContext->PSSetShaderResources(j, 1, &textureRV);
				}
			}

			d3dImmediateContext->DrawIndexed(Meshes[i].IndexCount, 0, 0);
		}

		for (auto* child : Children)
		{
			child->DrawRecursive(d3dImmediateContext, modelDrawInfo);
		}
	}
}