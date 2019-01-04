#ifndef __DIRECTX_HLSL__
#define __DIRECTX_HLSL__

#include<string>

namespace directx
{
	static const std::string HLSL_DEFAULT_SKINED_MESH = "\
		float4 lightDirection;  \r\n\
		float4 materialAmbient : MATERIALAMBIENT;\r\n\
		float4 materialDiffuse : MATERIALDIFFUSE;\r\n\
		float fogNear;\r\n\
		float fogFar;\r\n\
		static const int MAX_MATRICES = 80;\r\n\
		float4x3 mWorldMatrixArray[MAX_MATRICES] : WORLDMATRIXARRAY;\r\n\
		float4x4 mViewProj : VIEWPROJECTION;\r\n\
		\r\n\
		struct VS_INPUT\r\n\
		{\r\n\
			float4  pos          : POSITION;\r\n\
			float4  blendWeights : BLENDWEIGHT;\r\n\
			float4  blendIndices : BLENDINDICES;\r\n\
			float3  normal       : NORMAL;\r\n\
			float2  tex0         : TEXCOORD0;\r\n\
		};\r\n\
			 \r\n\
		struct VS_OUTPUT\r\n\
		{\r\n\
			float4  pos     : POSITION;\r\n\
			float4  diffuse : COLOR;\r\n\
			float2  tex0    : TEXCOORD0;\r\n\
			float   fog     : FOG;\r\n\
		};\r\n\
		\r\n\
		float3 CalcDiffuse(float3 normal)\r\n\
		{\r\n\
			float res;\r\n\
			res = max(0.0f, dot(normal, lightDirection.xyz));   \r\n\
			return (res);\r\n\
		}\r\n\
		\r\n\
		VS_OUTPUT DefaultTransform(VS_INPUT i, uniform int numBones)\r\n\
		{\r\n\
			VS_OUTPUT o;\r\n\
			float3 pos = 0.0f;\r\n\
			float3 normal = 0.0f;    \r\n\
			float lastWeight = 0.0f;\r\n\
		\r\n\
			float blendWeights[4] = (float[4])i.blendWeights;\r\n\
			int idxAry[4] = (int[4])i.blendIndices;\r\n\
		\r\n\
			for (int iBone = 0; iBone < numBones-1; iBone++)\r\n\
			{\r\n\
				lastWeight = lastWeight + blendWeights[iBone];\r\n\
				pos += mul(i.pos, mWorldMatrixArray[idxAry[iBone]]) * blendWeights[iBone];\r\n\
				normal += mul(i.normal, mWorldMatrixArray[idxAry[iBone]]) * blendWeights[iBone];\r\n\
			}\r\n\
			lastWeight = 1.0f - lastWeight; \r\n\
		\r\n\
			pos += (mul(i.pos, mWorldMatrixArray[idxAry[numBones-1]]) * lastWeight);\r\n\
			normal += (mul(i.normal, mWorldMatrixArray[idxAry[numBones-1]]) * lastWeight); \r\n\
			o.pos = mul(float4(pos.xyz, 1.0f), mViewProj);\r\n\
		\r\n\
		\r\n\
			normal = normalize(normal);\r\n\
			o.diffuse.xyz = materialAmbient.xyz + CalcDiffuse(normal) * materialDiffuse.xyz;\r\n\
			o.diffuse.w = materialAmbient.w * materialDiffuse.w;\r\n\
		\r\n\
			o.tex0 = i.tex0;\r\n\
			o.fog = 1.0f - (o.pos.z - fogNear) / (fogFar - fogNear);\r\n\
		\r\n\
			return o;\r\n\
		}\r\n\
		\r\n\
		technique BasicTec\r\n\
		{\r\n\
			pass P0\r\n\
			{\r\n\
				VertexShader = compile vs_2_0 DefaultTransform(4);\r\n\
			}\r\n\
		}\r\n\
		\r\n\
	";


}

#endif
