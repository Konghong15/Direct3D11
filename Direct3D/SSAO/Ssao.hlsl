cbuffer cbPerFrame : register(b0)
{
	float4x4 gViewToTexSpace; // Proj*Texture
	float4   gOffsetVectors[14];
	float4   gFrustumCorners[4];
};
 
Texture2D gNormalDepthMap : register(t0);
Texture2D gRandomVecMap : register(t1);
 
SamplerState samNormalDepth : register(s0);
SamplerState samRandomVec : register(s1);

struct VertexIn
{
	float3 PosL            : POSITION;
	float3 ToFarPlaneIndex : NORMAL;
	float2 Tex             : TEXCOORD;
};

struct VertexOut
{
    float4 PosH       : SV_POSITION;
    float3 ToFarPlane : TEXCOORD0;
	float2 Tex        : TEXCOORD1;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// 생성 자체를 정규화된 값을 했기 때문에 그냥 쓴다
	vout.PosH = float4(vin.PosL, 1.0f);
	// 각 노말의 x에는 플레인 인덱스가 들어있다.
	vout.ToFarPlane = gFrustumCorners[vin.ToFarPlaneIndex.x].xyz;
	vout.Tex = vin.Tex;
	
    return vout;
}

float OcclusionFunction(float distZ)
{
	int gSampleCount = 14;
	float    gSurfaceEpsilon     = 0.05f;
	float    gOcclusionRadius    = 0.5f;
	float    gOcclusionFadeStart = 0.2f;
	float    gOcclusionFadeEnd   = 2.0f;

	float occlusion = 0.0f;
	if(distZ > gSurfaceEpsilon)
	{
		float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;
		occlusion = saturate( (gOcclusionFadeEnd-distZ)/fadeLength );
	}
	
	return occlusion;	
}

float4 PS(VertexOut pin) : SV_Target
{
	int gSampleCount = 14;
	float    gOcclusionRadius    = 0.5f;
	float    gOcclusionFadeStart = 0.2f;
	float    gOcclusionFadeEnd   = 2.0f;
	float    gSurfaceEpsilon     = 0.05f;

	// 현재 픽셀의 노말과 깊이값을 읽어온다.
	float4 normalDepth = gNormalDepthMap.SampleLevel(samNormalDepth, pin.Tex, 0.0f);
	float3 n = normalDepth.xyz;
	float pz = normalDepth.w;
	
	// 뷰 공간에서의 좌표를 구해온다.
	float3 p = (pz / pin.ToFarPlane.z) * pin.ToFarPlane;
	
	// 랜덤한 벡터를 읽어온다.
	float3 randVec = 2.0f*gRandomVecMap.SampleLevel(samRandomVec, 4.0f *pin.Tex, 0.0f).rgb - 1.0f;

	float occlusionSum = 0.0f;

	[unroll]
	for(int i = 0; i < gSampleCount; ++i)
	{
		// 미리 만들어둔 잘 분포된 벡터들을 렌덤한 넘으로 반사시켜 무작위성을 추가한다.
		float3 offset = reflect(gOffsetVectors[i].xyz, randVec);
	
		// offset과 n의 부호를 가져온다. 0이면 0 양수면 1 음수면 -1
		float flip = sign( dot(offset, n) );
		
		// offset
		float3 q = p + flip * gOcclusionRadius * offset;
		
		float4 projQ = mul(float4(q, 1.0f), gViewToTexSpace);
		projQ /= projQ.w;

		float rz = gNormalDepthMap.SampleLevel(samNormalDepth, projQ.xy, 0.0f).a;
		float3 r = (rz / q.z) * q;
		
		float distZ = p.z - r.z;
		float dp = max(dot(n, normalize(r - p)), 0.0f);
		float occlusion = dp * OcclusionFunction(distZ);
		
		occlusionSum += occlusion;
	}
	
	occlusionSum /= gSampleCount;
	
	float access = 1.0f - occlusionSum;

	// Sharpen the contrast of the SSAO map to make the SSAO affect more dramatic.
	return saturate(pow(access, 4.0f));
}
