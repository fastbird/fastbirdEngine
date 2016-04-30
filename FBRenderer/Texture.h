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
#include "RendererEnums.h"
#include "RendererStructs.h"
namespace fb{
	FB_DECLARE_SMART_PTR(IPlatformTexture);
	FB_DECLARE_SMART_PTR(Texture);
	/** Texture.
	You can create a texture with the function Renderer::CreateTexture(...).
	When you create server textures with the same file name, the underying hardward
	texture resource will be shared among them.	
	*/
	class FB_DLL_RENDERER Texture
	{
		FB_DECLARE_PIMPL(Texture);
		friend class RenderResourceFactory;
		Texture();
		/** private operators.
		You can clone Textures efficiently with Texture:Clone(). GPU data
		will be shared as well as the underlying data like, texture type, size and path.
		If you modify the underlying data, for example setting the new texture file, 
		change binding slots and shader stages, only the instance you are handling
		will be affected.
		*/
		explicit Texture(const Texture&) = delete;
		Texture& operator=(const Texture& other) = delete;

	public:		
		static unsigned sNextTextureID;

		/** If you want this texture to be managed by the Renderer,
		do not call this function directly. Use Renderer::CreateTexture() instead.
		*/
		static TexturePtr Create();
		/** Usually, you don't need to clone on the texture you get from 
		Renderer::CreateTexture() since it is already cloned and you can make
		a change on binding shader stages or input slots as you with not polluting
		shared textures.
		Manually Cloned texture will not be managed by Renderer.
		*/
		static void ReloadTexture(const char* file);

		TexturePtr Clone() const;
		~Texture();

		size_t GetTextureID() const;
		const char* GetFilePath() const;		
		void SetFilePath(const char* path);
		void SetType(TEXTURE_TYPE type);
		TEXTURE_TYPE GetType() const;
		int GetWidth() const;
		int GetHeight() const;
		Vec2I GetSize();
		PIXEL_FORMAT GetFormat() const;
		void SetDebugName(const char* name);

		bool IsReady() const;
		void Bind(BINDING_SHADER shader, int slot);
		void Unbind();
		MapData Map(UINT subResource, MAP_TYPE type, MAP_FLAG flag);
		void Unmap(UINT subResource);
		void CopyToStaging(TexturePtr dst, UINT dstSubresource, UINT dstX, UINT dstY, UINT dstZ,
			UINT srcSubresource, Box3D* srcBox);
		void SaveToFile(const char* filename);
		void GenerateMips();
		void SetPlatformTexture(IPlatformTexturePtr platformTexture);
		IPlatformTexturePtr GetPlatformTexture() const;
	};
}