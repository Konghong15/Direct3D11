#pragma once

#include <directxtk/SimpleMath.h>
#include <vector>

namespace common
{
	class Waves
	{
	public:
		Waves();
		~Waves() = default;

		void Init(UINT m, UINT n, float dx, float dt, float speed, float damping);
		void Update(float dt);
		void Disturb(UINT i, UINT j, float magnitude);

		inline const DirectX::SimpleMath::Vector3& operator[](unsigned int i) const;

		inline UINT GetTriangleCount() const;
		inline UINT GetVertexCount() const;
		inline UINT GetColumnCount() const;
		inline UINT GetRowCount() const;

	private:
		UINT mNumRows;
		UINT mNumCols;

		UINT mVertexCount;
		UINT mTriangleCount;

		float mK1;
		float mK2;
		float mK3;

		float mTimeStep; // 시간 단계
		float mSpatialStep; // 공간 단계

		std::vector<DirectX::SimpleMath::Vector3> mPrevSolution;
		std::vector<DirectX::SimpleMath::Vector3> mCurSolution;
	};

	const DirectX::SimpleMath::Vector3& Waves::operator[](unsigned int i) const
	{
		return mCurSolution[i];
	}

	UINT Waves::GetTriangleCount() const
	{
		return mTriangleCount;
	}
	UINT Waves::GetVertexCount() const
	{
		return mVertexCount;
	}
	UINT Waves::GetColumnCount() const
	{
		return mNumCols;
	}
	UINT Waves::GetRowCount() const
	{
		return mNumRows;
	}
}