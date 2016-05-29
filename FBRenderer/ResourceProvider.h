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
#include "FBCommonHeaders/Types.h"
#include "ResourceTypes.h"

namespace fb{
	FB_DECLARE_SMART_PTR(IndexBuffer);
	FB_DECLARE_SMART_PTR(SamplerState);
	FB_DECLARE_SMART_PTR(DepthStencilState);
	FB_DECLARE_SMART_PTR(BlendState);
	FB_DECLARE_SMART_PTR(RasterizerState);
	FB_DECLARE_SMART_PTR(Material);
	FB_DECLARE_SMART_PTR(Shader);
	FB_DECLARE_SMART_PTR(Texture);
	FB_DECLARE_SMART_PTR(ResourceProvider);
	class ResourceProvider{
		FB_DECLARE_PIMPL_NON_COPYABLE(ResourceProvider);
		ResourceProvider();

	protected:
		~ResourceProvider();

	public:
		static ResourceProviderPtr Create();

		virtual TexturePtr GetTexture(int ResourceTypes_Textures);
		virtual TexturePtr GetTexture(int ResourceTypes_Textures, int index);
		virtual void SwapTexture(int ResourceTypes_Textures, int one, int two);
		virtual ShaderPtr GetShader(int ResourceTypes_Shaders);
		virtual void BindShader(int ResourceTypes_Shaders, bool unbindEmptySlot);
		virtual MaterialPtr GetMaterial(int ResourceTypes_Materials);
		virtual RasterizerStatePtr GetRasterizerState(int ResourceTypes_RasterizerStates);
		virtual BlendStatePtr GetBlendState(int ResourceTypes_BlendStates);
		virtual DepthStencilStatePtr GetDepthStencilState(int ResourceTypes_DepthStencilStates);
		virtual SamplerStatePtr GetSamplerState(int ResourceTypes_SamplerStates);
		virtual IndexBufferPtr GetIndexBuffer(ResourceTypes::IndexBuffer type);
		virtual void BindRasterizerState(int ResourceTypes_RasterizerStates);
		virtual void BindBlendState(int ResourceTypes_BlendStates);
		virtual void BindDepthStencilState(int ResourceTypes_DepthStencilStates, int stencilRef);

		virtual int GetNumToneMaps() const;
		virtual int GetNumLuminanceMaps() const;

		virtual void DeleteTexture(int ResourceTypes_Textures);
		virtual void DeleteShader(int ResourceTypes_Shaders);

	};
}

