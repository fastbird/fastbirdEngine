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
#include "ParticleSystem.h"
#include "ParticleEmitter.h"
#include "ParticleRenderObject.h"
#include "ParticleOptions.h"
#include "FBRenderer/Renderer.h"
#include "FBRenderer/RenderTarget.h"
#include "FBRenderer/Camera.h"
using namespace fb;
namespace fb{
	void ClearParticleRenderObjects();
}
Timer* fb::gpTimer = Timer::GetMainTimer().get();
class ParticleSystem::Impl
{
public:
	ParticleSystem* mSelf;
	ParticleOptionsPtr mParticleOptions;
	typedef std::vector< ParticleEmitterPtr > Emitters;
	Emitters mActiveParticles;
	Emitters mPendingAdds;
	Emitters mPendingDeletes;
	CameraPtr mOverridingCamera;
	CameraPtr mOriginalCamera;

	// cache
	typedef std::map<unsigned, ParticleEmitterPtr > PARTICLE_EMITTERS_BY_ID;
	PARTICLE_EMITTERS_BY_ID mParticleEmitters;
	typedef std::map<std::string, ParticleEmitterPtr > PARTICLE_EMITTERS_BY_NAME;
	PARTICLE_EMITTERS_BY_NAME mParticleEmittersByName;

	ParticleEmitterPtr mEditingParticle;

	//---------------------------------------------------------------------------
	Impl(ParticleSystem* self)
		: mSelf(self)
		, mParticleOptions(ParticleOptions::Create())
		, mOverridingCamera(Camera::Create())
	{
	}
	~Impl(){
		ClearParticleRenderObjects();
	}

	void Update(float elapsedTime){
		ParticleRenderObject::ClearParticles();
		auto& renderer = Renderer::GetInstance();
		for (auto p : mPendingDeletes)
		{
			mActiveParticles.erase(
				std::remove(mActiveParticles.begin(), mActiveParticles.end(), p),
				mActiveParticles.end());
		}
		mPendingDeletes.clear();

		for (auto p : mPendingAdds)
		{
			AddActiveParticle(p);
		}
		mPendingAdds.clear();

		if (mEditingParticle)
		{
			if (!mEditingParticle->IsAlive())
			{
				mEditingParticle->Active(true);
			}
			if (0) // Move Edit Particle
			{
				float time = gpTimer->GetTime();
				Vec3 movedir = Vec3(cos(time), 0.f, 0.f);
				Vec3 oldPos = mEditingParticle->GetPosition();
				Vec3 pos = oldPos + 1.f * movedir;
				mEditingParticle->SetPosition(pos);
				Vec3 dir = pos - oldPos;				
				auto pcam = renderer.GetMainCamera();
				Vec3 campos = pcam->GetPosition();
				pcam->SetPosition(campos + dir);
				float len = dir.Normalize();
				if (len > 0.f)
				{
					mEditingParticle->SetDirection(dir);
				}
			}


		}
		auto cam = renderer.GetMainCamera();
		Vec3 camDir = cam->GetDirection();
		Vec3 camPos = cam->GetPosition();
		std::stable_sort(mActiveParticles.begin(), mActiveParticles.end(), [&camDir, &camPos](ParticleEmitterPtr a, ParticleEmitterPtr b)->bool
		{
			float da = (a->GetPosition() - camPos).Dot(camDir);
			float db = (b->GetPosition() - camPos).Dot(camDir);
			return da > db;
		});

		Emitters::iterator it = mActiveParticles.begin();
		for (; it != mActiveParticles.end();)
		{
			bool updated = (*it)->Update(elapsedTime);
			if (!updated)
				it = mActiveParticles.erase(it);
			else
				it++;
		}

		if (0)//r_numParticleEmitters
		{
			wchar_t buf[256];
			swprintf_s(buf, L"num of active particles : %u", mActiveParticles.size());
			renderer.QueueDrawText(Vec2I(100, 226), buf, Color::White);
		}

		ParticleRenderObject::EndUpdateParticles();
	}

	void RenderProfile(){
		wchar_t msg[255];
		int x = 700;
		int y = 20;
		int yStep = 20;
		auto& r = Renderer::GetInstance();
		swprintf_s(msg, 255, L"Emitter Types= %u", mParticleEmitters.size());
		r.QueueDrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
		y += yStep;

		swprintf_s(msg, 255, L"Total Emitter Instances= %u", mActiveParticles.size());
		r.QueueDrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
		y += yStep;

		swprintf_s(msg, 255, L"Total Emitter Renderers = %u", ParticleRenderObject::GetNumRenderObject());
		r.QueueDrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
		y += yStep;

		swprintf_s(msg, 255, L"Number of Draw Calls = %u", ParticleRenderObject::GetNumDrawCalls());
		r.QueueDrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
		y += yStep;

		swprintf_s(msg, 255, L"Number of Draw Vertices = %u", ParticleRenderObject::GetNumPrimitives());
		r.QueueDrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
		y += yStep;
	}

	ParticleEmitterPtr GetParticleEmitter(const char* file){
		PARTICLE_EMITTERS_BY_NAME::iterator found = mParticleEmittersByName.find(file);
		if (found != mParticleEmittersByName.end())
		{
			return found->second->Clone();
		}

		ParticleEmitterPtr p = ParticleEmitter::Create();
		bool succ = p->Load(file, false);
		if (succ)
		{
			mParticleEmitters[p->GetEmitterID()] = p;
			mParticleEmittersByName[file] = p;
			return p->Clone();
		}

		return 0;
	}

	ParticleEmitterPtr GetParticleEmitter(unsigned id){		
		PARTICLE_EMITTERS_BY_ID::iterator found = mParticleEmitters.find(id);
		if (found != mParticleEmitters.end())
			return found->second->Clone();

		std::string filepath = FindParticleNameWithID("data/particles", id);
		return GetParticleEmitter(filepath.c_str());		
	}

	void ReloadParticle(const char* file){
		for (auto& p : mParticleEmittersByName)
		{
			if (_stricmp(p.first.c_str(), file) == 0)
			{
				std::string lower = file;
				ToLowerCase(lower);
				bool reload = true;
				p.second->Load(lower.c_str(), reload);				
			}
		}
	}

	void EditThisParticle(const char* file){
		if (mEditingParticle)
			mEditingParticle->StopImmediate();

		bool numeric = IsNumeric(file);
		std::string fullpath;
		if (numeric)
		{
			fullpath = FindParticleNameWithID("data/particles", StringConverter::ParseInt(file));
		}
		else
		{
			fullpath = file;
			fullpath += ".particle";
		}


		ParticleEmitterPtr pnew = 0;
		if (strcmp(file, "0"))
			pnew = GetParticleEmitter(fullpath.c_str());

		auto& renderer = Renderer::GetInstance();
		if (pnew)
		{

			mEditingParticle = pnew;
			Logger::Log(FB_DEFAULT_LOG_ARG, FormatString("Editing : %s", fullpath.c_str()).c_str());			
			auto rt = renderer.GetMainRenderTarget();
			assert(rt);
			if (!mOriginalCamera){
				mOriginalCamera = rt->ReplaceCamera(mOverridingCamera);
				*mOverridingCamera = *mOriginalCamera;
			}
			mEditingParticle->SetPosition(mOverridingCamera->GetPosition() + mOverridingCamera->GetDirection() * 4.f);
			mEditingParticle->Active(true);
			mOverridingCamera->SetTarget(mEditingParticle);
			mOverridingCamera->SetEnalbeInput(true);
		}
		else if (strcmp(file, "0") == 0)
		{
			mEditingParticle->StopImmediate();
			mEditingParticle = 0;
			auto rt = renderer.GetMainRenderTarget();
			assert(rt);
			if (mOriginalCamera){
				rt->ReplaceCamera(mOriginalCamera);
				mOriginalCamera = 0;
			}
			Logger::Log(FB_ERROR_LOG_ARG, "Exit particle editor");			
		}
		else
		{
			Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot find the particle %s", fullpath.c_str()).c_str());			
		}
	}

	void ScaleEditingParticle(float scale){
		if (mEditingParticle)
		{
			mEditingParticle->SetScale(Vec3(scale));
		}
	}

	unsigned GetNumActiveParticles() const{
		return mActiveParticles.size();
	}

	void StopParticles(){
		Emitters::iterator it = mActiveParticles.begin();
		for (; it != mActiveParticles.end(); it++)
		{
			(*it)->StopImmediate();
		}
	}

	
	std::string FindParticleNameWithID(const char* particleFolder, unsigned id){
		if (!ValidCStringLength(particleFolder)){
			Logger::Log(FB_ERROR_LOG_ARG, "Invalid arg");
			return std::string();
		}

		auto iterator = FileSystem::GetDirectoryIterator(particleFolder, false);
		while (iterator->HasNext()){
			std::string filename = FileSystem::GetFileName(iterator->GetNextFilePath());
			auto sv = Split(filename, "_");
			if (sv.size() >= 2)
			{
				unsigned fileid = StringConverter::ParseUnsignedInt(sv[0]);
				if (fileid == id){					
					return std::string(particleFolder) + "/" + filename;
				}
			}
		}
		Logger::Log(FB_ERROR_LOG_ARG, FormatString("Cannot find particles which has id %d", id).c_str());
		return std::string();
	}

	void AddActiveParticle(ParticleEmitterPtr pEmitter){
		assert(!ValueExistsInVector(mActiveParticles, pEmitter));
		mActiveParticles.push_back(pEmitter);
		DeleteValuesInVector(mPendingDeletes, pEmitter);
	}

	void RemoveDeactiveParticle(ParticleEmitterPtr pEmitter){
		mPendingDeletes.push_back(pEmitter);
	}

	void AddActiveParticlePending(ParticleEmitterPtr pEmitter){
		assert(!ValueExistsInVector(mPendingAdds, pEmitter));
		mPendingAdds.push_back(pEmitter);
		DeleteValuesInVector(mPendingDeletes, pEmitter);
	}	
};

//---------------------------------------------------------------------------
static ParticleSystemWeakPtr sParticleSystem;
ParticleSystemPtr ParticleSystem::Create(){
	if (sParticleSystem.expired()){
		auto p = ParticleSystemPtr(new ParticleSystem, [](ParticleSystem* obj){ delete obj; });
		sParticleSystem = p;
		return p;
	}
	return sParticleSystem.lock();
}
ParticleSystem& ParticleSystem::GetInstance(){
	if (sParticleSystem.expired()){
		Logger::Log(FB_ERROR_LOG_ARG, "Particle System is deleted! The program will crash...");
	}
	return *sParticleSystem.lock();
}

ParticleSystem::ParticleSystem()
	:mImpl(new Impl(this))
{

}

ParticleSystem::~ParticleSystem(){

}

void ParticleSystem::Update(float elapsedTime) {
	mImpl->Update(elapsedTime);
}

void ParticleSystem::RenderProfile() {
	mImpl->RenderProfile();
}

ParticleEmitterPtr ParticleSystem::GetParticleEmitter(const char* file) {
	return mImpl->GetParticleEmitter(file);
}

ParticleEmitterPtr ParticleSystem::GetParticleEmitter(unsigned id) {
	return mImpl->GetParticleEmitter(id);
}

void ParticleSystem::ReloadParticle(const char* file) {
	mImpl->ReloadParticle(file);
}

void ParticleSystem::EditThisParticle(const char* path) {
	mImpl->EditThisParticle(path);
}

void ParticleSystem::ScaleEditingParticle(float scale) {
	mImpl->ScaleEditingParticle(scale);
}

unsigned ParticleSystem::GetNumActiveParticles() const {
	return mImpl->GetNumActiveParticles();
}

void ParticleSystem::StopParticles() {
	mImpl->StopParticles();
}

void ParticleSystem::AddActiveParticle(ParticleEmitterPtr pEmitter) {
	mImpl->AddActiveParticle(pEmitter);
}

void ParticleSystem::RemoveDeactiveParticle(ParticleEmitterPtr pEmitter) {
	mImpl->RemoveDeactiveParticle(pEmitter);
}

void ParticleSystem::AddActiveParticlePending(ParticleEmitterPtr pEmitter) {
	mImpl->AddActiveParticlePending(pEmitter);
}

