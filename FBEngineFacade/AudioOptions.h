#pragma once
#include "FBCommonHeaders/Types.h"
#include "FBConsole/ICVarObserver.h"

namespace fb{
	FB_DECLARE_SMART_PTR_STRUCT(CVar);
	FB_DECLARE_SMART_PTR(AudioOptions);
	class AudioOptions : public ICVarObserver{
		AudioOptions();
		~AudioOptions();

	public:
		static AudioOptionsPtr Create();
		// ICVarObserver
		bool OnChangeCVar(CVarPtr pCVar);

		float a_MusicGain;
		float a_MasterGain;
		int a_Enabled;
	};
}