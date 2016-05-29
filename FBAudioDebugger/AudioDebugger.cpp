#include "stdafx.h"
#include "AudioDebugger.h"
#include "FBAudioPlayer/AudioSource.h"
#include "FBAudioPlayer/AudioManager.h"
#include "FBAudioPlayer/AudioBuffer.h"
#include "FBRenderer/Renderer.h"
#include "FBRenderer/Camera.h"
#include "FBRenderer/Texture.h"
#include "FBRenderer/ResourceProvider.h"
#include "FBRenderer/Material.h"
using namespace fb;
class AudioDebugger::Impl{
public:

	void Render(){
		auto& am = AudioManager::GetInstance();
		auto& renderer = Renderer::GetInstance();
		auto cam = renderer.GetCamera();
		auto mat = cam->GetMatrix(ICamera::ViewProj);
		const auto& rtSize = renderer.GetMainRenderTargetSize();

		std::vector<AudioManager::AudioDebugData> list;
		am.GetAudioList(list);
		auto texture = renderer.CreateTexture("EssentialEngineData/textures/audio.png", 
			TextureCreationOption{false, false});
		unsigned numWaiting = 0;
		unsigned numPlaying = 0;
		unsigned numDropping = 0;
		for (auto& it : list){
			auto dist = it.mDistPerRef;
			//22500 = 150* 150
			auto sizeMulti = (1.0f - SmoothStep(0.f, 22500.f, std::min(22500.f, dist)));
			int imageSize = Round(30 * sizeMulti);
			float fontSize = 16.f * sizeMulti;
			Vec3 pos = it.mPosition;
			Color color;
			auto status = it.mStatus;
			if (status == AudioSourceStatus::Playing){
				color = Color::Yellow;
			}
			else if (status == AudioSourceStatus::Waiting){
				color = Color::Silver;
			}
			else if (status == AudioSourceStatus::Dropping){
				color = Color::Blue;
			}
			else{
				Logger::Log(FB_ERROR_LOG_ARG, "Invalid audio status.");
				color = Color::White;
			}
			color.a() = it.mGain;
			auto toPos = pos - cam->GetPosition();
			if (toPos.Dot(cam->GetDirection()) < 0)
				continue;
			auto screenPos = mat * Vec4(pos, 1.0f);
			screenPos.x /= screenPos.w;
			screenPos.y /= screenPos.w;
			auto x = Round((screenPos.x * .5f + .5f) * rtSize.x);
			auto y = Round((1.0f - (screenPos.y * .5f + .5f)) * rtSize.y);
			
			auto mat = renderer.GetResourceProvider()->GetMaterial(ResourceTypes::Materials::QuadTextured);			
			Vec2I posI(x, y);
			renderer.DrawQuadWithTexture(posI, Vec2I(imageSize, imageSize), color, texture);
			char buf[255];
			sprintf_s(buf, "File: %s", it.mFilePath.c_str());
			renderer.QueueDrawText(Vec2I(x + 40, y), AnsiToWide(buf), color, fontSize);
			y += 24;
			sprintf_s(buf, "Gain: %.2f", it.mGain);
			renderer.QueueDrawText(Vec2I(x + 40, y), AnsiToWide(buf), color, fontSize);

			if (it.mStatus == AudioSourceStatus::Playing){
				++numPlaying;
			}
			else if (it.mStatus == AudioSourceStatus::Dropping){
				++numDropping;
			}
			else if (it.mStatus == AudioSourceStatus::Waiting){
				++numWaiting;
			}
		}

		char buf[255];
		int x = 30;
		int y = 200;
		sprintf_s(buf, "Num generated: %d", am.GetNumGenerated());
		renderer.QueueDrawText(Vec2I(x, y), AnsiToWide(buf), Color::White, 16.f);
		y += 24;
		sprintf_s(buf, "Num sources: %d", list.size());
		renderer.QueueDrawText(Vec2I(x, y), AnsiToWide(buf), Color::White, 16.f);		
		y += 24;
		sprintf_s(buf, "Num playing: %d", numPlaying);
		renderer.QueueDrawText(Vec2I(x, y), AnsiToWide(buf), Color::White, 16.f);
		y += 24;
		sprintf_s(buf, "Num waiting: %d", numWaiting);
		renderer.QueueDrawText(Vec2I(x, y), AnsiToWide(buf), Color::White, 16.f);
		y += 24;
		sprintf_s(buf, "Num dropping: %d", numDropping);
		renderer.QueueDrawText(Vec2I(x, y), AnsiToWide(buf), Color::White, 16.f);
	}
};

//---------------------------------------------------------------------------
FB_IMPLEMENT_STATIC_CREATE(AudioDebugger);
AudioDebugger::AudioDebugger()
	: mImpl(new Impl)
{

}

AudioDebugger::~AudioDebugger(){

}

void AudioDebugger::Render(){
	mImpl->Render();
}