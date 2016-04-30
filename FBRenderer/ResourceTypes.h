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

#pragma once
namespace fb{
	namespace ResourceTypes{
		namespace Textures{
			// Only textures independent from the render target size.
			enum Enum{				
				Noise,
				GGXGenTarget,				
				GGX,
				ToneMap,
				LuminanceMap,
				Last = LuminanceMap,
			};
		}

		namespace Shaders{
			enum Enum{
				FullscreenQuadNearVS,
				FullscreenQuadFarVS,
				CopyPS,
				CopyPSMS,
				ShadowMapShader,
				DepthWriteVSPS,
				DepthOnlyVSPS,
				CloudDepthWriteVSPS,
				
				SampleLumInitialPS,
				SampleLumIterativePS,
				SampleLumFinalPS,
				CalcAdaptedLumPS,
				ToneMappingPS,
				SimpleToneMappingPS,
				BrightPassPS,
				BloomPS,
				Blur5x5PS,
				StarGlarePS,
				MergeTextures2PS,

				GodRayPS,
				OcclusionPrePassVSPS,
				OcclusionPrePassVSGSPS,

				GlowPS,
				SilouettePS,

				GGXGenPS,

				Last = GGXGenPS,



			};
		}

		namespace Materials{
			enum Enum{
				Missing,
				Quad,
				QuadTextured,
				BillboardQuad,

				Last = BillboardQuad,
			};
		}

		namespace RasterizerStates{
			enum Enum{
				Default,
				CullFrontFace,
				OneBiased,
				WireFrame,		
				ShadowMapRS,

				Last = WireFrame,
			};
		}

		namespace BlendStates{
			enum Enum{
				Default,
				DefaultKeepAlpha,
				Additive,
				AdditiveKeepAlpha,
				AlphaBlend,
				MaxBlend,				
				RedAlphaMask,
				GreenAlphaMask,
				GreenAlphaMaskMax,
				RedAlphaMaskAddMinus,
				GreenAlphaMaskAddAdd,
				RedAlphaMaskAddAdd,
				GreenAlphaMaskAddMinus,
				GreenMask,
				BlueMask,
				NoColorWrite,
			};
		}

		namespace DepthStencilStates{
			enum Enum{
				Default,
				NoDepthStencil,
				NoDepthWrite_LessEqual,
				LessEqual,
			};
		}

		namespace SamplerStates{
			enum Enum{
				Point,
				Linear,
				Anisotropic,
				Shadow,
				PointWrap,
				LinearWrap,
				BlackBorder,
				PointBlackBorder,
				LinearMirror,
				Last = LinearMirror,
				Num = Last + 1,
			};
		}
	}
}