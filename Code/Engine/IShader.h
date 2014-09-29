#pragma once
#ifndef _fastbird_IShader_header_included_
#define _fastbird_IShader_header_included_
#include <set>
#include <CommonLib/SmartPtr.h>
#include <Engine/IMaterial.h>
#include <Engine/Renderer/RendererStructs.h>

namespace fastbird
{
	class CLASS_DECLSPEC_ENGINE IShader : public ReferenceCounter
	{
	public:
		static void ReloadShader(const char* name);
		static void ReloadShader(const char* name, const IMaterial::SHADER_DEFINES& shaderDefines);
		static IShader* FindShader(const char* name, const IMaterial::SHADER_DEFINES& shaderDefines);

		virtual ~IShader() {}
		virtual void Bind() = 0;
		virtual void BindVS() = 0;
		virtual void BindGS() = 0;
		virtual void BindPS() = 0;
		virtual void BindDS() = 0;
		virtual void BindHS() = 0;
		virtual bool IsValid() const = 0;
		virtual void SetCompileFailed(bool failed) = 0;
		virtual bool GetCompileFailed() const = 0;
		virtual void* GetVSByteCode(unsigned& size) const = 0;
		
		virtual const char* GetName() const = 0;
		virtual void SetShaderDefines(const IMaterial::SHADER_DEFINES& defines) = 0;
		virtual const IMaterial::SHADER_DEFINES& GetShaderDefines() const = 0;

		virtual void SetBindingShaders(int shaders) = 0;
		virtual int GetBindingShaders() const = 0;

		typedef std::set<std::string> INCLUDE_FILES;
		virtual void SetIncludeFiles(const INCLUDE_FILES& ifs) = 0;
		virtual const INCLUDE_FILES& GetIncludeFiles() const = 0;
		virtual bool CheckIncludes(const std::string& inc) = 0;

		// do not call directly. use FB_SET_DEVICE_DEBUG_NAME define.
		virtual void SetDebugName(const char*) = 0;

		// for game. in the engine use smartptr instead of this.
		virtual void Delete() = 0;
	};
}

#endif //_fastbird_IShader_header_included_