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

namespace fb
{
	//--------------------------------------------------------------------------------
	class IFileChangeObserver
	{
	public:
		enum{
			FileChange_Engine,			
			FileChange_Game,
		};
		/// Called when any modificated file found.
		/// Then you can call FileMonitor::Check() to make FileMonitor to noify through IFileChangeObserver::OnFileChanged() function.
		/// If you call FileMonitor::Check() every frame, you don't need to call this function.
		virtual void OnChangeDetected()=0;
		/// extension contains "." ex).xml
		/// returns 'processed'
		virtual bool OnFileChanged(const char* watchDir, const char* filepath, const char* loweredExtension) = 0;
		
	};
}