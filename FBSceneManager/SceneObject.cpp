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
#include "SceneObject.h"
#include "FBCommonHeaders/Helpers.h"
#include "Scene.h"
using namespace fb;
//---------------------------------------------------------------------------
SceneObject::SceneObject()
	: mObjFlag(0)
	, mGameType(-1)
	, mGameId(-1)
	, mGamePtr(0)
	, mDebug(false)
{
}

SceneObject::SceneObject(const SceneObject& other)
	: mName(other.mName)
	, mScenes(other.mScenes)
	, mObjFlag(other.mObjFlag)
	, mGameType(-1)
	, mGameId(-1)
	, mGamePtr(0)
{
}

SceneObject::~SceneObject()
{
}

SceneObjectType::Enum SceneObject::GetType() const{
	return SceneObjectType::Unknown;
}

void SceneObject::SetName(const char* name){
	if (name)
		mName = name;
}

const char* SceneObject::GetName() const{
	return mName.c_str();
}

void SceneObject::OnAttachedToScene(ScenePtr pScene)
{
	if (pScene == 0)
		return;

	if (!ValueExistsInVector(mScenes, pScene)) {
		mScenes.push_back(pScene);
	}
}

void SceneObject::OnDetachedFromScene(ScenePtr pScene)
{
	DeleteValuesInVector(mScenes, pScene);
}

bool SceneObject::IsAttached() const{
	for (auto it = mScenes.begin(); it != mScenes.end();/**/){
		IteratingWeakContainer(mScenes, it, scene);
	}
	return !mScenes.empty();
}

bool SceneObject::IsAttached(ScenePtr pScene) const {
	if (!pScene)
	{
		return !mScenes.empty();
	}
	return ValueExistsInVector(mScenes, pScene);
}

//-------------------------------------------------------------------
// Object Flags
//-------------------------------------------------------------------
void SceneObject::SetObjFlag(unsigned flag) {
	mObjFlag = flag;
}

unsigned SceneObject::GetObjFlag() const {
	return mObjFlag;
}

void SceneObject::ModifyObjFlag(unsigned flag, bool enable) {
	if (enable)
	{
		mObjFlag |= flag;
	}
	else
	{
		mObjFlag = mObjFlag  & ~flag;
	}
}

bool SceneObject::HasObjFlag(unsigned flag) {
	return (mObjFlag & flag) != 0;
}

void SceneObject::SetVisible(bool visible) {
	ModifyObjFlag(SceneObjectFlag::Hide, !visible);
}

bool SceneObject::GetVisible() const {
	return mObjFlag & SceneObjectFlag::Hide ? false : true;
}

//-------------------------------------------------------------------
// Debugging features
//-------------------------------------------------------------------
void SceneObject::SetGameType(int type){
	mGameType = type;
}

int SceneObject::GetGameType() const {
	return mGameType;
}

void SceneObject::SetGameId(unsigned id) {
	mGameId = id;
}

unsigned SceneObject::GetGameId() const {
	return mGameId;
}

void SceneObject::SetGamePtr(void* ptr) {
	mGamePtr = ptr;
}

void* SceneObject::GetGamePtr() const {
	return mGamePtr;
}

bool SceneObject::DetachFromScene(){
	return DetachFromScene(false);
}

bool SceneObject::DetachFromScene(bool includingRtt){
	std::vector<Scene*> scenes;
	for (auto it = mScenes.begin(); it != mScenes.end(); /**/)
	{
		IteratingWeakContainer(mScenes, it, scene);
		if (!scene->IsRttScene() || includingRtt)
		{
			scenes.push_back(scene.get());
		}
	}

	bool detached = false;
	for (auto scene : scenes){
		detached = scene->DetachObject(this) || detached;
	}

	return detached;
}

std::vector<ScenePtr> SceneObject::GetScenes()const {
	std::vector<ScenePtr> scenes;
	for (auto it = mScenes.begin(); it != mScenes.end(); /**/){
		IteratingWeakContainer(mScenes, it, scene);
		scenes.push_back(scene);
	}
	return scenes;
}