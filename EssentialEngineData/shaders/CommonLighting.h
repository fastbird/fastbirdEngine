/*
 -----------------------------------------------------------------------------
 This source file is part of fastbird engine
 For the latest info, see http://www.jungwan.net/
 
 Copyright (c) 2013-2015 Jungwan Byun
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 -----------------------------------------------------------------------------
*/

#include "EssentialEngineData/shaders/CommonDefines.h"

// using same slot : 5
Texture2D gShadowMap : register(t8);
Texture2D gGGXMap : register(t9);

void BlinnPhongShading(float3 lightColor, float3 normal, float3 toLight,
    float3 toView, float shininess, out float3 diffuse, out float3 specular)
{ 
    float ldn= dot(toLight, normal);
	diffuse = lightColor * saturate(ldn);
	specular = float3(0, 0, 0);
	
	float3 h = normalize(toView + toLight);
	float ndh = dot(normal,h);
	float specularTerm = pow(saturate(ndh), shininess);
	//float specularPow = exp2(specularT*11.0 + 2.0);
	//float specularNorm = (specularPow+8.0) / 8.0;    
	specular = lightColor * specularTerm;	
}

// Specular F
// referenct :  http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
float3 Fresnel(	float vdh, float3 F0)
{
	//float sphg = pow(2.0, (-5.55473*vdh - 6.98316) * vdh);
	//float sphg = pow(1.0f - vdh, 5);
	float sphg = exp2(-8.65617024533378044416*vdh); // equivalent to pow(1.0 - vdh, 5.0)
	return F0 + (float3(1.0, 1.0, 1.0) - F0) * sphg;
}

// Normal Distrubution function (NDF) = Specular D
// Disney's GGX/Trowbridge-Reitz
float NormalDistrib(float ndh, float alpha)
{
	float tmp = alpha / (ndh*ndh*(alpha*alpha-1.0)+1.0);
	return tmp * tmp * INV_PI;
}

// Specular G
// specular geometric attenuation term based on Schlick model with different k.
// w is either toLight or toView
float G1(float ndw, float k)
{	
	return 1.0 / ( ndw*(1.0-k) + k ); // k > 0
}

// float Visibility(float ndl, float ndv, float alpha)
// {
	// // visibility is a Cook-Torrance geometry function divided by (n.l)*(n.v)
	// float k = alpha * 0.5;
	// return G1(ndl,k)*G1(ndv,k);
// }

// reference : http://www.filmicworlds.com/2014/04/21/optimizing-ggx-shaders-with-dotlh/
// ldh == vdh
float NewVisibility(float ldh, float alpha){
	float k = alpha * 0.5;
	float k2 = k*k;
	float invK2 = 1.0f - k2;
	float vis = rcp(ldh*ldh*invK2 + k2);
	return vis;
}

float Pow4(float v){
	return v*v*v*v;
}
// Cook-Torrance microfacet specular shading model
float3 CookTorrance(float vdh, float ndh, float ndl, float3 Ks, float roughness)
{	
	float D = gGGXMap.Sample(gLinearSampler, float2(Pow4(ndh), roughness)).x;
	float2 fvHelper = gGGXMap.Sample(gLinearSampler, float2(vdh, roughness)).yz;
	float3 FV = Ks*fvHelper.x + (1.0 - Ks)*fvHelper.y;
	float3 specular = D * FV * ndl * 0.2;
	return specular;
	
// reference : http://www.filmicworlds.com/2014/04/21/optimizing-ggx-shaders-with-dotlh/
	// float alpha = roughness * roughness;
          // // F : Frenel
	// return Fresnel(vdh,Ks)
	   
	   // // D : GGX Distribution
	// * ( NormalDistrib(ndh,alpha)
		
		// // V : Schlick approximation of Smith solved with GGX
	// * NewVisibility(vdh,alpha) * 0.25);
}

float3 CookTorranceContrib(
	float vdh,
	float ndh,
	float ndl,
	float3 Ks,
	float roughness)
{
// This is the contribution when using importance sampling with the GGX based
// sample distribution. This means ct_contrib = ct_brdf / ggx_probability
	float alpha = roughness*roughness;
	return Fresnel(vdh,Ks) * (NewVisibility(vdh,alpha) * vdh * ndl / ndh ) * 0.25;
}

vec3 ImportanceSampleLambert(vec2 hamOffset, vec3 tan, vec3 bi, vec3 normal)
{
	float cosT = sqrt(hamOffset.y);
	float sinT = sqrt(1.0 - hamOffset.y);
	float phi = TWO_PI * hamOffset.x;	

	return (sinT*cos(phi)) * tan + (sinT*sin(phi)) * bi + cosT * normal;
}

float ProbabilityLambert(vec3 Ln, vec3 Nn)
{
	return max(0.0, dot(Nn, Ln) * INV_PI);
}

float Distortion(vec3 Wn)
{
	// Computes the inverse of the solid angle of the (differential) pixel in
	// the environment map pointed at by Wn
	float sinT = max(0.0000001, sqrt(1.0 - Wn.y*Wn.y));
	return 1.0 / sinT;
}

float ComputeLOD(vec3 Ln, float p)
{
	return max(0.0, (ENV_MAX_LOD - 1.5) - 0.5 * (log(float(ENV_SAMPLES)) + log(p * Distortion(Ln))) * INV_LOG2);
}

vec3 ImportanceSampleGGX(vec2 hamOffset, vec3 tan, vec3 bi, vec3 normal, float alpha)
{
	float cosT = sqrt((1.0 - hamOffset.y) / (1.0 + (alpha*alpha - 1.0)*hamOffset.y));
	float sinT = sqrt(1.0 - cosT*cosT);
	float phi = 2.0*PI*hamOffset.x;
	return (sinT*cos(phi)) * tan + (sinT*sin(phi)) * bi + cosT * normal;
}

float ProbabilityGGX(float ndh, float vdh, float Roughness)
{
	return NormalDistrib(ndh, Roughness) * ndh / (4.0*vdh);
}



float3 CalcEnvContrib(float3 normal, float3 tangent, float3 binormal, float roughness, float3 toViewDir, float3 diffColor, float3 specColor)
{
	float3 envContrib = {0, 0, 0};	
#ifdef ENV_TEXTURE
	vec3 Tp = normalize(tangent	- normal*dot(tangent, normal)); // local tangent
	vec3 Bp = normalize(cross(tangent, normal));
	float alpha = roughness * roughness;
	static float2 hammersley[16] = (float2[16])gHammersley;
	for(int i=0; i<ENV_SAMPLES; ++i)
	{
		vec2 Xi = hammersley[i];		
		vec3 Sd = ImportanceSampleLambert(Xi,Tp,Bp,normal);
		float pdfD = ProbabilityLambert(Sd, normal);
		float lodD = ComputeLOD(Sd, pdfD);	
		
		envContrib +=	SampleEnvironmentMap(Sd, lodD) * diffColor;
		vec3 Hn = ImportanceSampleGGX(Xi,Tp,Bp,normal,alpha);
		
		float3 Ln = -reflect(toViewDir, Hn);
		Ln.yz = Ln.zy;

		float ndl = dot(normal, Ln);

		// Horizon fading trick from http://marmosetco.tumblr.com/post/81245981087
		const float horizonFade = 1.3;
		float horiz = clamp( 1.0 + horizonFade * ndl, 0.0, 1.0 );
		horiz *= horiz;
		ndl = saturate(ndl);

		float vdh = saturate(dot(toViewDir, Hn));
		float ndh = saturate(dot(normal, Hn));
		float lodS = roughness < 0.01 ? 0.0 : ComputeLOD(Ln, ProbabilityGGX(ndh, vdh, roughness));
		//envContrib += SampleEnvironmentMap(Ln, lodS) * CookTorranceContrib(vdh, ndh, ndl, specColor, roughness) * horiz;
	}

	envContrib /= ENV_SAMPLES;
	return envContrib;
#else
	return envContrib;
#endif
}

float3 CalcPointLights(float3 worldPos, float3 normal)
{
	float3 result={0, 0, 0};
	int count = (int)gPointLightColor[0].w;
	count = min(3, count);
	for(int i=0; i<count; i++)
	{
		
		float lightReach = gPointLightPos[i].w;
		float3 toLight = gPointLightPos[i].xyz - worldPos;
		float dist = length(toLight);
		toLight = toLight / dist;
		float atten = clamp(1.0 - dist/lightReach, 0, 1);
		atten *= atten;
		float d = max(0, dot(toLight, normal));
		result += (atten * d) * gPointLightColor[i].xyz;
	}
	return result;
}
#define _CASCADE_COUNT_FLAG 3

//--------------------------------------------------------------------------------------
// Calculate amount to blend between two cascades and the band where blending will occure.
//--------------------------------------------------------------------------------------
void CalculateBlendAmountForInterval ( in int iCurrentCascadeIndex, 
                                       in out float fPixelDepth, 
                                       in out float fCurrentPixelsBlendBandLocation,
                                       out float fBlendBetweenCascadesAmount
                                       ) 
{

    // We need to calculate the band of the current shadow map where it will
	// fade into the next cascade. We can then early out of the expensive PCF 
	// for loop.
	static float cascadeFrustumsEyeSpaceDepthsFloat[8] = (float[8])gCascadeFrustumsEyeSpaceDepthsFloat;
    float fBlendInterval = cascadeFrustumsEyeSpaceDepthsFloat[iCurrentCascadeIndex];
    //if( iNextCascadeIndex > 1 ) 
    int fBlendIntervalbelowIndex = min(0, iCurrentCascadeIndex-1);
    fPixelDepth -= cascadeFrustumsEyeSpaceDepthsFloat[ fBlendIntervalbelowIndex ];
    fBlendInterval -= cascadeFrustumsEyeSpaceDepthsFloat[ fBlendIntervalbelowIndex ];
    
    // The current pixel's blend band location will be used to determine when 
	// we need to blend and by how much.
    fCurrentPixelsBlendBandLocation = fPixelDepth / fBlendInterval;
    fCurrentPixelsBlendBandLocation = 1.0f - fCurrentPixelsBlendBandLocation;
    // The fBlendBetweenCascadesAmount is our location in the blend band.
    fBlendBetweenCascadesAmount = fCurrentPixelsBlendBandLocation / gCascadeBlendArea;
}

void ComputeCoordinatesTransform( in int iCascadeIndex,                                      
                                   in out float4 vShadowTexCoord, 
                                   in out float4 vShadowTexCoordViewSpace ) 
{
    // Now that we know the correct map, we can transform the world space 
	// position of the current fragment                	
    vShadowTexCoord = vShadowTexCoordViewSpace * gCascadeScale[iCascadeIndex];
    vShadowTexCoord += gCascadeOffset[iCascadeIndex];
          
    vShadowTexCoord.x *= gShadowPartitionSize;  // precomputed 1.0f / cascadeLevels
    vShadowTexCoord.x += (gShadowPartitionSize * (float)iCascadeIndex ); 
} 

//--------------------------------------------------------------------------------------
// Use PCF to sample the depth map and return a percent lit value.
//--------------------------------------------------------------------------------------
void CalculatePCFPercentLit ( in float4 vShadowTexCoord, 
                              in float fRightTexelDepthDelta, 
                              in float fUpTexelDepthDelta,                               
                              out float fPercentLit
                              ) 
{
    fPercentLit = 0.0f;
    // This loop could be unrolled, and texture immediate offsets could be used 
	// if the kernel size were fixed. This would be performance improvment.
    for( int x = -2; x < 3; ++x ) 
    {
        for( int y = -2; y < 3; ++y ) 
        {
            float depthcompare = vShadowTexCoord.z;
            // A very simple solution to the depth bias problems of PCF is to 
			// use an offset.
            // Unfortunately, too much offset can lead to Peter-panning 
			// (shadows near the base of object disappear )
            // Too little offset can lead to shadow acne 
			// ( objects that should not be in shadow are partially self shadowed ).
            depthcompare -= 0.002f;
            //if ( USE_DERIVATIVES_FOR_DEPTH_OFFSET_FLAG ) 
            //{
                // Add in derivative computed depth scale based on the x and y pixel.
              //  depthcompare += fRightTexelDepthDelta * ( (float) x ) + fUpTexelDepthDelta * ( (float) y );
            //}
            // Compare the transformed pixel depth to the depth read from the map.
            fPercentLit += gShadowMap.SampleCmpLevelZero( gShadowSampler, 
                float2( 
                    vShadowTexCoord.x + ( ( (float) x ) * gShadowTexelSizeX ) , 
                    vShadowTexCoord.y + ( ( (float) y ) * gShadowTexelSize ) 
                    ), 
                depthcompare );
        }
    }
    fPercentLit /= (float)25.f;
}


//--------------------------------------------------------------------------------------
// This function calculates the screen space depth for shadow space texels
//--------------------------------------------------------------------------------------
void CalculateRightAndUpTexelDepthDeltas ( in float3 vShadowTexDDX,
                                           in float3 vShadowTexDDY,
                                           out float fUpTextDepthWeight,
                                           out float fRightTextDepthWeight
 ) {
        
    // We use the derivatives in X and Y to create a transformation matrix.  
	// Because these derivives give us the transformation from screen space 
	// to shadow space, we need the inverse matrix to take us from shadow space 
    // to screen space.  This new matrix will allow us to map shadow map texels 
	// to screen space.  This will allow us to find the screen space depth of 
	// a corresponding depth pixel.
	// This is not a perfect solution as it assumes the underlying geometry of 
	// the scene is a plane.  A more accureate way of finding the actual depth 
	// would be to do a deferred rendering approach and actually sample the depth.
    
    // Using an offset, or using variance shadow maps is a better approach to reducing these artifacts in most cases.
    
    float2x2 matScreentoShadow = float2x2( vShadowTexDDX.xy, vShadowTexDDY.xy );
    float fDeterminant = determinant ( matScreentoShadow );
    
    float fInvDeterminant = 1.0f / fDeterminant;
    
    float2x2 matShadowToScreen = float2x2 (
        matScreentoShadow._22 * fInvDeterminant, matScreentoShadow._12 * -fInvDeterminant, 
        matScreentoShadow._21 * -fInvDeterminant, matScreentoShadow._11 * fInvDeterminant );

    float2 vRightShadowTexelLocation = float2( gShadowTexelSize, 0.0f );
    float2 vUpShadowTexelLocation = float2( 0.0f, gShadowTexelSize );  
    
    // Transform the right pixel by the shadow space to screen space matrix.
    float2 vRightTexelDepthRatio = mul( vRightShadowTexelLocation,  matShadowToScreen );
    float2 vUpTexelDepthRatio = mul( vUpShadowTexelLocation,  matShadowToScreen );

    // We can now caculate how much depth changes when you move up or right in 
	// the shadow map.
    // We use the ratio of change in x and y times the dervivite in X and Y of 
	// the screen space depth to calculate this change.
    fUpTextDepthWeight = vUpTexelDepthRatio.x * vShadowTexDDX.z 
							+ vUpTexelDepthRatio.y * vShadowTexDDY.z;
    fRightTextDepthWeight = vRightTexelDepthRatio.x * vShadowTexDDX.z 
							+ vRightTexelDepthRatio.y * vShadowTexDDY.z;
        
}

static const float4 vCascadeColorsMultiplier[8] = 
{
    float4 ( 1.5f, 0.0f, 0.0f, 1.0f ),
    float4 ( 0.0f, 1.5f, 0.0f, 1.0f ),
    float4 ( 0.0f, 0.0f, 5.5f, 1.0f ),
    float4 ( 1.5f, 0.0f, 5.5f, 1.0f ),
    float4 ( 1.5f, 1.5f, 0.0f, 1.0f ),
    float4 ( 1.0f, 1.0f, 1.0f, 1.0f ),
    float4 ( 0.0f, 1.0f, 5.5f, 1.0f ),
    float4 ( 0.5f, 3.5f, 0.75f, 1.0f )
};

float GetShadow(float4 vShadowMapTextureCoordViewSpace, float fCurrentPixelDepth)
{	
	// swap y, z
	vShadowMapTextureCoordViewSpace = vShadowMapTextureCoordViewSpace.xzyw;
	int iCurrentCascadeIndex = 0;
	static float cascadeFrustumsEyeSpaceDepthsFloat[8] = (float[8])gCascadeFrustumsEyeSpaceDepthsFloat;
	if ( _CASCADE_COUNT_FLAG > 1 ) 
	{
		float4 vCurrentPixelDepth = fCurrentPixelDepth;
		float4 fComparison = ( vCurrentPixelDepth > 
			float4(cascadeFrustumsEyeSpaceDepthsFloat[0],
					cascadeFrustumsEyeSpaceDepthsFloat[1],
					cascadeFrustumsEyeSpaceDepthsFloat[2],
					cascadeFrustumsEyeSpaceDepthsFloat[3]));
					
		float4 fComparison2 = ( vCurrentPixelDepth > 
			float4(cascadeFrustumsEyeSpaceDepthsFloat[4],
					cascadeFrustumsEyeSpaceDepthsFloat[5],
					cascadeFrustumsEyeSpaceDepthsFloat[6],
					cascadeFrustumsEyeSpaceDepthsFloat[7]));
		float fIndex = dot( 
						float4( _CASCADE_COUNT_FLAG > 0,
								_CASCADE_COUNT_FLAG > 1, 
								_CASCADE_COUNT_FLAG > 2, 
								_CASCADE_COUNT_FLAG > 3)
						, fComparison )
					 + dot( 
						float4(
								_CASCADE_COUNT_FLAG > 4,
								_CASCADE_COUNT_FLAG > 5,
								_CASCADE_COUNT_FLAG > 6,
								_CASCADE_COUNT_FLAG > 7)
						, fComparison2 ) ;
								
		fIndex = min( fIndex, _CASCADE_COUNT_FLAG - 1 );
		iCurrentCascadeIndex = (int)fIndex;
	}
	
	// Repeat text coord calculations for the next cascade. 
    // The next cascade index is used for blurring between maps.
	int iNextCascadeIndex = min ( _CASCADE_COUNT_FLAG - 1, iCurrentCascadeIndex + 1 ); 
	float fBlendBetweenCascadesAmount = 1.0f;
    float fCurrentPixelsBlendBandLocation = 1.0f;
	if( _CASCADE_COUNT_FLAG > 1  ) 
	{
		CalculateBlendAmountForInterval ( iCurrentCascadeIndex, fCurrentPixelDepth, 
			fCurrentPixelsBlendBandLocation, fBlendBetweenCascadesAmount );
	}   
	
	// float3 vShadowMapTextureCoordDDX;
    // float3 vShadowMapTextureCoordDDY;
    // // The derivatives are used to find the slope of the current plane.
    // // The derivative calculation has to be inside of the loop in order to 
	// // prevent divergent flow control artifacts.
    // vShadowMapTextureCoordDDX = ddx( vShadowMapTextureCoordViewSpace );
    // vShadowMapTextureCoordDDY = ddy( vShadowMapTextureCoordViewSpace );    
    // vShadowMapTextureCoordDDX *= gCascadeScale[iCurrentCascadeIndex];
    // vShadowMapTextureCoordDDY *= gCascadeScale[iCurrentCascadeIndex];    
	
	float4 vShadowMapTextureCoord = 0.0f;	
	ComputeCoordinatesTransform( iCurrentCascadeIndex,                                   
                                 vShadowMapTextureCoord, 
                                 vShadowMapTextureCoordViewSpace );
	
	float4 vVisualizeCascadeColor = vCascadeColorsMultiplier[iCurrentCascadeIndex];
	
	float fUpTextDepthWeight=0;
	float fRightTextDepthWeight=0;
	//CalculateRightAndUpTexelDepthDeltas ( vShadowMapTextureCoordDDX, vShadowMapTextureCoordDDY,
      //                                        fUpTextDepthWeight, fRightTextDepthWeight );
	
	//int iBlurRowSize = m_iPCFBlurForLoopEnd - m_iPCFBlurForLoopStart;	
    //iBlurRowSize *= iBlurRowSize;
	//float fBlurRowSize = (float)iBlurRowSize;    
	float fPercentLit = 0.0f;
	CalculatePCFPercentLit ( vShadowMapTextureCoord, fRightTextDepthWeight, 
                            fUpTextDepthWeight, fPercentLit );
							
	if (_CASCADE_COUNT_FLAG > 1){
		const float cascadeBlendArea = 0.05f;
		if( fCurrentPixelsBlendBandLocation < cascadeBlendArea) 
        {  // the current pixel is within the blend band.
    
            // Repeat text coord calculations for the next cascade. 
            // The next cascade index is used for blurring between maps.    
			float4 vShadowMapTextureCoord_blend = 0.0f;			
            ComputeCoordinatesTransform( iNextCascadeIndex,
                                             vShadowMapTextureCoord_blend, 
										     vShadowMapTextureCoordViewSpace );        
			float fPercentLit_blend = 0.0f;
			float fUpTextDepthWeight_blend=0;
			float fRightTextDepthWeight_blend=0;
			CalculatePCFPercentLit( vShadowMapTextureCoord_blend, fRightTextDepthWeight_blend, 
									fUpTextDepthWeight_blend, fPercentLit_blend );
			fPercentLit = lerp( fPercentLit_blend, fPercentLit, fBlendBetweenCascadesAmount ); 
			// Blend the two calculated shadows by the blend amount.
               
        } 
	}
	return lerp(0.5f, 1.0f, fPercentLit);
}

float3 GetIrrad(float4 vNormal)
{
	//Ramamoorthi; Hanrahan, An Efficient Representation for Irradiance Environment Maps, 2001
	float c1 = 0.429043f, c2 = 0.511664f, c3 = 0.743125f,
		c4 = 0.886227f, c5 = 0.247708f;
		
	float4 ret = c1 * gIrradConstsnts[8] * (vNormal.x*vNormal.x - vNormal.y*vNormal.y) + c3 * gIrradConstsnts[6] * vNormal.z*vNormal.z + 
				c4 * gIrradConstsnts[0] - c5 * gIrradConstsnts[6] 
				+ 2.0 * c1 * (gIrradConstsnts[4]*(vNormal.x*vNormal.y) + gIrradConstsnts[7] * vNormal.x * vNormal.z + gIrradConstsnts[5] * vNormal.y * vNormal.z)
				+ 2.0 * c2 * (gIrradConstsnts[3] * vNormal.x + gIrradConstsnts[1] * vNormal.y + gIrradConstsnts[2] * vNormal.z);
				
	return ret.rgb;
				
	// Sloan, Stupid Spherical Harmonics(SH) Tricks, 2008
	// float3 x1, x2, x3;
	// // Linear + constant polynomial terms
	// x1.r = dot(gIrradConstsnts[0],vNormal);
	// x1.g = dot(gIrradConstsnts[1],vNormal);
	// x1.b = dot(gIrradConstsnts[2],vNormal);
	
	// // 4 of the quadratic polynomials
	// float4 vB = vNormal.xyzz * vNormal.yzzx;
	// x2.r = dot(gIrradConstsnts[3],vB);
	// x2.g = dot(gIrradConstsnts[4],vB);
	// x2.b = dot(gIrradConstsnts[5],vB);
	
	// // Final quadratic polynomial
	// float vC = vNormal.x*vNormal.x - vNormal.y*vNormal.y;
	// x3 = gIrradConstsnts[6].rgb * vC;
	
	// return x1+x2+x3;
}
