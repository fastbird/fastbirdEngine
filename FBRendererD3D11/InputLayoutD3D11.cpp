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
#include "InputLayoutD3D11.h"
#include "IUnknownDeleter.h"
#include "RendererD3D11.h"

using namespace fb;

//----------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(InputLayoutD3D11);
InputLayoutD3D11::InputLayoutD3D11()
	: mInputLayout(0)
{
}

void InputLayoutD3D11::Bind() {
	RendererD3D11::GetInstance().SetInputLayout(this);
}

void InputLayoutD3D11::SetDebugName(const char* name) {
	if (mInputLayout){
		mInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName, 0, 0);
		mInputLayout->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
	}
}

//----------------------------------------------------------------------------
void InputLayoutD3D11::SetHardwareInputLayout(ID3D11InputLayout* pLayout) {
	mInputLayout = ID3D11InputLayoutPtr(pLayout, IUnknownDeleter());
}

ID3D11InputLayout* InputLayoutD3D11::GetHardwareInputLayout() const{
	return mInputLayout.get();
}