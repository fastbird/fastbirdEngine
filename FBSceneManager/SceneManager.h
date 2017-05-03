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
namespace fb{	
	FB_DECLARE_SMART_PTR(SceneManagerOptions);
	FB_DECLARE_SMART_PTR(Camera);
	FB_DECLARE_SMART_PTR(IScene);
	FB_DECLARE_SMART_PTR(Scene);
	FB_DECLARE_SMART_PTR(SceneManager);
	class FB_DLL_SCENEMANAGER SceneManager{
		FB_DECLARE_PIMPL_NON_COPYABLE(SceneManager);
		SceneManager();				
		~SceneManager();

	public:
		
		/** You have the ownership of returned scene. */
		static SceneManagerPtr Create();
		/** Returns the SceneManager as a reference.
		This function does not check the validity whether the SceneManager is created or not.
		It will cause a crash if you call this function without calling CreateSceneManager();
		*/
		static SceneManager& GetInstance();
		static void DeleteSceneManager();

		/** You have the ownership of returned scene. 
		The first created scene is the main scene. */
		ScenePtr CreateScene(const char* name);
		/// Get the main scene.
		ScenePtr GetMainScene() const;
		
		void Update(TIME_PRECISION dt);

		void CopyDirectionalLight(IScenePtr destScene, int destLightSlot, IScenePtr srcScene, int srcLightSlot);

		SceneManagerOptionsPtr GetOptions() const;
	};
}