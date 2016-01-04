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
#include "FBCommonHeaders/CircularBuffer.h"
#include "FBSceneManager/SpatialObject.h"
namespace fb
{
	FB_DECLARE_SMART_PTR(IScene);
	FB_DECLARE_SMART_PTR(ParticleEmitter);
	class FB_DLL_PARTICLESYSTEM ParticleEmitter : public SpatialObject
	{
		FB_DECLARE_PIMPL_CLONEABLE(ParticleEmitter);
		ParticleEmitter(IScenePtr scene);
		~ParticleEmitter();
		static ParticleEmitterPtr Create(const ParticleEmitter& other);

	public:
		static ParticleEmitterPtr Create(IScenePtr scene);

		ParticleEmitterPtr Clone();
		/** Loads a particle emitter.
		if reload is true, Particle Template will be kept the sharing state.
		*/
		bool Load(const char* filepath, bool reload);
		bool UpdateEmitter(float elapsedTime, const Vec3& mainCamPosition);
		unsigned GetEmitterID() const;		
		void Active(bool a, bool pending = false);
		void Stop();
		void ProcessDeleteOnStop(bool stopping, bool force = false);
		void StopImmediate();
		void SetVisible(bool visible);
		bool GetVisible() const;
		bool IsAlive();
		bool IsActive() const;
		void SetEmitterDirection(const fb::Vec3& dir);
		void SetEmitterColor(const Color& c);
		void SetAlpha(float alpha);
		float GetAlpha() const ;
		bool IsInfinite() const;
		void SetBufferSize(unsigned size);
		//void Sort();
		void SetLength(float length);
		void SetRelativeVelocity(const Vec3& dir, float speed);
		void RemoveShaderDefine(const char* def);
		void AddShaderDefine(const char* def, const char* val);
		//void ApplyShaderDefine();	

		void SetScene(IScenePtr scene);
		IScenePtr GetScene();
		void SetTeamColor(const Color& color);
	};
}