#pragma once
#ifndef _fastbird_IShader_header_included_
#define _fastbird_IShader_header_included_

#include <CommonLib/SmartPtr.h>
#include <Engine/IMaterial.h>
#include <Engine/Renderer/RendererStructs.h>

namespace fastbird
{
	class CLASS_DECLSPEC_ENGINE IShader : public ReferenceCounter
	{
	public:
		static void ReloadShader(const char* name, const IMaterial::SHADER_DEFINES& shaderDefines);
		static IShader* FindShader(const char* name, const IMaterial::SHADER_DEFINES& shaderDefines);

		virtual ~IShader() {}
		virtual void Bind() = 0;
		virtual bool IsValid() const = 0;
		virtual void* GetVSByteCode(unsigned& size) const = 0;
		
		virtual const char* GetName() const = 0;
		virtual void SetShaderDefines(const IMaterial::SHADER_DEFINES& defines) = 0;
		virtual const IMaterial::SHADER_DEFINES& GetShaderDefines() const = 0;

		virtual void SetBindingShaders(int shaders) = 0;
		virtual int GetBindingShaders() const = 0;
	};
}

#endif //_fastbird_IShader_header_included_