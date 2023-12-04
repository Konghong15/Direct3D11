
#include "Octree.h"

namespace ambientOcclusion
{
	OctreeNode::OctreeNode()
	{
		for (int i = 0; i < 8; ++i)
			Children[i] = 0;

		Bounds.Center = Vector3(0.0f, 0.0f, 0.0f);
		Bounds.Extents = Vector3(0.0f, 0.0f, 0.0f);

		IsLeaf = false;
	}
	OctreeNode::~OctreeNode()
	{
		for (int i = 0; i < 8; ++i)
			SafeDelete(Children[i]);
	}
	void OctreeNode::Subdivide(BoundingBox box[8])
	{
		Vector3 halfExtent(
			0.5f * Bounds.Extents.x,
			0.5f * Bounds.Extents.y,
			0.5f * Bounds.Extents.z);

		box[0].Center = Vector3(
			Bounds.Center.x + halfExtent.x,
			Bounds.Center.y + halfExtent.y,
			Bounds.Center.z + halfExtent.z);
		box[0].Extents = halfExtent;

		box[1].Center = Vector3(
			Bounds.Center.x - halfExtent.x,
			Bounds.Center.y + halfExtent.y,
			Bounds.Center.z + halfExtent.z);
		box[1].Extents = halfExtent;

		box[2].Center = Vector3(
			Bounds.Center.x - halfExtent.x,
			Bounds.Center.y + halfExtent.y,
			Bounds.Center.z - halfExtent.z);
		box[2].Extents = halfExtent;

		box[3].Center = Vector3(
			Bounds.Center.x + halfExtent.x,
			Bounds.Center.y + halfExtent.y,
			Bounds.Center.z - halfExtent.z);
		box[3].Extents = halfExtent;

		// "Bottom" four quadrants.
		box[4].Center = Vector3(
			Bounds.Center.x + halfExtent.x,
			Bounds.Center.y - halfExtent.y,
			Bounds.Center.z + halfExtent.z);
		box[4].Extents = halfExtent;

		box[5].Center = Vector3(
			Bounds.Center.x - halfExtent.x,
			Bounds.Center.y - halfExtent.y,
			Bounds.Center.z + halfExtent.z);
		box[5].Extents = halfExtent;

		box[6].Center = Vector3(
			Bounds.Center.x - halfExtent.x,
			Bounds.Center.y - halfExtent.y,
			Bounds.Center.z - halfExtent.z);
		box[6].Extents = halfExtent;

		box[7].Center = Vector3(
			Bounds.Center.x + halfExtent.x,
			Bounds.Center.y - halfExtent.y,
			Bounds.Center.z - halfExtent.z);
		box[7].Extents = halfExtent;
	}

	Octree::Octree()
		: mRoot(nullptr)
	{
	}
	Octree::~Octree()
	{
		SafeDelete(mRoot);
	}

	void Octree::Build(const std::vector<Vector3>& vertices, const std::vector<UINT>& indices)
	{
		mVertices = vertices;

		BoundingBox sceneBounds = buildAABB();

		mRoot = new OctreeNode();
		mRoot->Bounds = sceneBounds;

		buildOctree(mRoot, indices);
	}
	bool Octree::RayOctreeIntersect(Vector4 rayPos, Vector4 rayDir)
	{
		return rayOctreeIntersect(mRoot, rayPos, rayDir);
	}

	BoundingBox Octree::buildAABB()
	{
		XMVECTOR vmin = XMVectorReplicate(+MathHelper::Infinity);
		XMVECTOR vmax = XMVectorReplicate(-MathHelper::Infinity);
		for (size_t i = 0; i < mVertices.size(); ++i)
		{
			XMVECTOR P = XMLoadFloat3(&mVertices[i]);

			vmin = XMVectorMin(vmin, P);
			vmax = XMVectorMax(vmax, P);
		}

		BoundingBox bounds;
		XMVECTOR C = 0.5f * (vmin + vmax);
		XMVECTOR E = 0.5f * (vmax - vmin);

		XMStoreFloat3(&bounds.Center, C);
		XMStoreFloat3(&bounds.Extents, E);

		return bounds;
	}
	void Octree::buildOctree(OctreeNode* parent, const std::vector<UINT>& indices)
	{
		size_t triCount = indices.size() / 3;

		if (triCount < 60)
		{
			parent->IsLeaf = true;
			parent->Indices = indices;
		}
		else
		{
			parent->IsLeaf = false;

			BoundingBox subbox[8];
			parent->Subdivide(subbox);

			for (int i = 0; i < 8; ++i)
			{
				// Allocate a new subnode.
				parent->Children[i] = new OctreeNode();
				parent->Children[i]->Bounds = subbox[i];

				// Find triangles that intersect this node's bounding box.
				std::vector<UINT> intersectedTriangleIndices;
				for (size_t j = 0; j < triCount; ++j)
				{
					UINT i0 = indices[j * 3 + 0];
					UINT i1 = indices[j * 3 + 1];
					UINT i2 = indices[j * 3 + 2];

					XMVECTOR v0 = XMLoadFloat3(&mVertices[i0]);
					XMVECTOR v1 = XMLoadFloat3(&mVertices[i1]);
					XMVECTOR v2 = XMLoadFloat3(&mVertices[i2]);

					if (subbox[i].Intersects(v0, v1, v2))
					{
						intersectedTriangleIndices.push_back(i0);
						intersectedTriangleIndices.push_back(i1);
						intersectedTriangleIndices.push_back(i2);
					}
				}

				// Recurse.
				buildOctree(parent->Children[i], intersectedTriangleIndices);
			}
		}
	}
	bool Octree::rayOctreeIntersect(OctreeNode* parent, Vector4 rayPos, Vector4 rayDir)
	{
		if (!parent->IsLeaf)
		{
			for (int i = 0; i < 8; ++i)
			{
				float t;
				auto& boundBox = parent->Children[i]->Bounds;

				if (boundBox.Intersects(rayPos, rayDir, t))
				{
					if (rayOctreeIntersect(parent->Children[i], rayPos, rayDir))
					{
						return true;
					}
				}
			}

			return false;
		}
		else
		{
			size_t triCount = parent->Indices.size() / 3;

			for (size_t i = 0; i < triCount; ++i)
			{
				UINT i0 = parent->Indices[i * 3 + 0];
				UINT i1 = parent->Indices[i * 3 + 1];
				UINT i2 = parent->Indices[i * 3 + 2];

				XMVECTOR v0 = XMLoadFloat3(&mVertices[i0]);
				XMVECTOR v1 = XMLoadFloat3(&mVertices[i1]);
				XMVECTOR v2 = XMLoadFloat3(&mVertices[i2]);

				float t;
				Ray ray((Vector3)rayPos, (Vector3)rayDir);

				if (ray.Intersects(v0, v1, v2, t))
				{
					return true;
				}
			}

			return false;
		}
	}
}
