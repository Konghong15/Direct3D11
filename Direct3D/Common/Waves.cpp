#include <algorithm>

#include "Waves.h"

namespace common
{
	Waves::Waves()
		: mNumRows(0)
		, mNumCols(0)
		, mVertexCount(0)
		, mTriangleCount(0)
		, mK1(0.0f)
		, mK2(0.0f)
		, mK3(0.0f)
		, mTimeStep(0.0f)
		, mSpatialStep(0.0f)
	{
	}

	void Waves::Init(UINT m, UINT n, float dx, float dt, float speed, float damping)
	{
		mNumRows = m;
		mNumCols = n;

		mVertexCount = m * n;
		mTriangleCount = (m - 1) * (n - 1) * 2;

		mTimeStep = dt;
		mSpatialStep = dx;

		float d = damping * dt + 2.0f;
		float e = (speed * speed) * (dt * dt) / (dx * dx);
		mK1 = (damping * dt - 2.0f) / d;
		mK2 = (4.0f - 8.0f * e) / d;
		mK3 = (2.0f * e) / d;

		mPrevSolution.clear();
		mCurSolution.clear();

		mPrevSolution.resize(m * n);
		mCurSolution.resize(m * n);

		float halfWidth = (n - 1) * dx * 0.5f;
		float halfDepth = (m - 1) * dx * 0.5f;

		for (UINT i = 0; i < m; ++i)
		{
			float z = halfDepth - i * dx;

			for (UINT j = 0; j < n; ++j)
			{
				float x = -halfWidth + j * dx;

				mPrevSolution[i * n + j] = { x, 0.0f, z };
				mCurSolution[i * n + j] = { x, 0.0f, z };
			}
		}
	}

	void Waves::Update(float dt)
	{
		static float t = 0;

		t += dt;

		if (t >= mTimeStep)
		{
			for (DWORD i = 1; i < mNumRows - 1; ++i)
			{
				for (DWORD j = 1; j < mNumCols - 1; ++j)
				{
					mPrevSolution[i * mNumCols + j].y =
						mK1 * mPrevSolution[i * mNumCols + j].y +
						mK2 * mCurSolution[i * mNumCols + j].y +
						mK3 * (mCurSolution[(i + 1) * mNumCols + j].y +
							mCurSolution[(i - 1) * mNumCols + j].y +
							mCurSolution[i * mNumCols + j + 1].y +
							mCurSolution[i * mNumCols + j - 1].y);
				}
			}

			std::swap(mPrevSolution, mCurSolution);

			t = 0.0f;
		}
	}

	void Waves::Disturb(UINT i, UINT j, float magnitude)
	{
		assert(i > 1 && i < mNumRows - 2);
		assert(j > 1 && j < mNumCols - 2);

		float halfMag = 0.5f * magnitude;

		mCurSolution[i * mNumCols + j].y += magnitude;
		mCurSolution[i * mNumCols + j + 1].y += halfMag;
		mCurSolution[i * mNumCols + j - 1].y += halfMag;
		mCurSolution[(i + 1) * mNumCols + j].y += halfMag;
		mCurSolution[(i - 1) * mNumCols + j].y += halfMag;
	}
}