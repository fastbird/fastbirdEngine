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

#include "stdafx.h"
#include "NullPlatformRenderer.h"
using namespace fb;

FB_IMPLEMENT_STATIC_CREATE(NullPlatformRenderer);

NullPlatformRenderer::NullPlatformRenderer(){

}
NullPlatformRenderer::~NullPlatformRenderer(){

}

void NullPlatformRenderer::PrepareQuit() {
}

Vec2ITuple NullPlatformRenderer::FindClosestSize(HWindowId id, const Vec2ITuple& input) {
	return Vec2ITuple(0, 0);
}

bool NullPlatformRenderer::GetResolutionList(unsigned& outNum, Vec2ITuple* list) {
	return false;
}

bool NullPlatformRenderer::InitCanvas(const CanvasInitInfo& info, 
	IPlatformTexturePtr& outColorTexture, IPlatformTexturePtr& outDepthTexture) {
	return false;
}

void NullPlatformRenderer::DeinitCanvas(HWindowId id, HWindow window) {
	
}

bool NullPlatformRenderer::ChangeResolution(HWindowId id, HWindow window, const Vec2ITuple& resol,IPlatformTexturePtr& outColorTexture, IPlatformTexturePtr& outDepthTexture) {
	return false;
}

bool NullPlatformRenderer::ChangeFullscreenMode(HWindowId id, HWindow window, int mode) {
	return false;
}

unsigned NullPlatformRenderer::GetMultiSampleCount() const {
	return 1;
}

void NullPlatformRenderer::SetShaderCacheOption(bool useShaderCache, bool generateCache) {
	
}

IPlatformTexturePtr NullPlatformRenderer::CreateTexture(const char* path, const TextureCreationOption& option) {
	return 0;
}

IPlatformTexturePtr NullPlatformRenderer::CreateTexture(void* data, int width, int height,PIXEL_FORMAT format, 
	int mipLevels, BUFFER_USAGE usage, int  buffer_cpu_access,int texture_type) {
	return 0;
}

IPlatformVertexBufferPtr NullPlatformRenderer::CreateVertexBuffer(void* data, unsigned stride,unsigned numVertices, BUFFER_USAGE usage, BUFFER_CPU_ACCESS_FLAG accessFlag) {
	return 0;
}

IPlatformIndexBufferPtr NullPlatformRenderer::CreateIndexBuffer(void* data, unsigned int numIndices,INDEXBUFFER_FORMAT format) {
	return 0;
}

IPlatformShaderPtr NullPlatformRenderer::CreateVertexShader(const char* path,
	const SHADER_DEFINES& defines, bool ignoreCache) {
	return 0;
}

IPlatformShaderPtr NullPlatformRenderer::CreateGeometryShader(const char* path,
	const SHADER_DEFINES& defines, bool ignoreCache) {
	return 0;
}

IPlatformShaderPtr NullPlatformRenderer::CreatePixelShader(const char* path,
	const SHADER_DEFINES& defines, bool ignoreCache) {
	return 0;
}

IPlatformShaderPtr NullPlatformRenderer::CreateComputeShader(const char* path,
	const SHADER_DEFINES& defines, bool ignoreCache) {
	return 0;
}

IPlatformShaderPtr NullPlatformRenderer::CompileComputeShader(const char* code, const char* entry,
	const SHADER_DEFINES& defines) {
	return 0;
}

IPlatformInputLayoutPtr NullPlatformRenderer::CreateInputLayout(const INPUT_ELEMENT_DESCS& descs,void* shaderByteCode, unsigned size) {
	return 0;
}

IPlatformBlendStatePtr NullPlatformRenderer::CreateBlendState(const BLEND_DESC& desc) {
	return 0;
}

IPlatformDepthStencilStatePtr NullPlatformRenderer::CreateDepthStencilState(const DEPTH_STENCIL_DESC& desc) {
	return 0;
}

IPlatformRasterizerStatePtr NullPlatformRenderer::CreateRasterizerState(const RASTERIZER_DESC& desc) {
	return 0;
}

IPlatformSamplerStatePtr NullPlatformRenderer::CreateSamplerState(const SAMPLER_DESC& desc) {
	return 0;
}

unsigned NullPlatformRenderer::GetNumLoadingTexture() const {
	return 0;
}

void NullPlatformRenderer::SetRenderTarget(IPlatformTexturePtr pRenderTargets[], size_t rtViewIndex[], int num,IPlatformTexturePtr pDepthStencil, size_t dsViewIndex) {
	
}

void NullPlatformRenderer::SetDepthTarget(IPlatformTexturePtr pDepthStencil, size_t dsViewIndex) {

}

void NullPlatformRenderer::SetViewports(const Viewport viewports[], int num) {
	
}

void NullPlatformRenderer::SetScissorRects(const Rect rects[], int num) {
	
}

void NullPlatformRenderer::SetVertexBuffers(unsigned int startSlot, unsigned int numBuffers,IPlatformVertexBuffer const * pVertexBuffers[], unsigned int const strides[], unsigned int offsets[]) {
	
}

void NullPlatformRenderer::SetPrimitiveTopology(PRIMITIVE_TOPOLOGY pt) {
	
}

void NullPlatformRenderer::SetTextures(IPlatformTexturePtr pTextures[], int num, SHADER_TYPE shaderType, int startSlot) {
	
}

void NullPlatformRenderer::UpdateShaderConstants(ShaderConstants::Enum type, const void* data, int size) {
	
}

void* NullPlatformRenderer::MapShaderConstantsBuffer() const{
	return 0;
}

void NullPlatformRenderer::UnmapShaderConstantsBuffer() const{

}

void* NullPlatformRenderer::MapBigBuffer() const{
	return 0;
}

void NullPlatformRenderer::UnmapBigBuffer() const{

}

void NullPlatformRenderer::UnbindInputLayout() {
	
}

void NullPlatformRenderer::UnbindShader(SHADER_TYPE shader) {
	
}

void NullPlatformRenderer::UnbindTexture(SHADER_TYPE shader, int slot) {
	
}

void NullPlatformRenderer::CopyToStaging(IPlatformTexture* dst, UINT dstSubresource, UINT dstx, UINT dsty, UINT dstz,IPlatformTexture* src, UINT srcSubresource, Box3D* pBox) {
	
}

void NullPlatformRenderer::Draw(unsigned int vertexCount, unsigned int startVertexLocation) {
	
}

void NullPlatformRenderer::DrawIndexed(unsigned indexCount, unsigned startIndexLocation, unsigned startVertexLocation) {
	
}

void NullPlatformRenderer::Clear(Real r, Real g, Real b, Real a, Real z, unsigned char stencil) {
	
}

void NullPlatformRenderer::Clear(Real r, Real g, Real b, Real a) {
	
}

void NullPlatformRenderer::ClearDepthStencil(Real z, UINT8 stencil) {

}

void NullPlatformRenderer::ClearState() {
	
}

void NullPlatformRenderer::Present() {
	
}

void NullPlatformRenderer::BeginEvent(const char* name) {
	
}

void NullPlatformRenderer::EndEvent() {
	
}

void NullPlatformRenderer::TakeScreenshot(const char* filename) {
	
}

