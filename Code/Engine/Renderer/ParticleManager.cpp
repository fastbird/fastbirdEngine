#include <Engine/StdAfx.h>
#include <Engine/Renderer/ParticleManager.h>
#include <Engine/Renderer/ParticleEmitter.h>
#include <Engine/RenderObjects/ParticleRenderObject.h>
#include <Engine/ICamera.h>
#include <CommonLib/tinydir.h>
namespace fastbird
{

ParticleManager* ParticleManager::sParticleManager=0;

//---------------------------------------------------------------------------
void ParticleManager::InitializeParticleManager()
{
	sParticleManager = FB_NEW(ParticleManager);
}

//---------------------------------------------------------------------------
ParticleManager& ParticleManager::GetParticleManager()
{
	assert(sParticleManager);
	return *sParticleManager;
}

//---------------------------------------------------------------------------
void ParticleManager::FinalizeParticleManager()
{
	FB_SAFE_DEL(sParticleManager);
}

//---------------------------------------------------------------------------
std::string ParticleManager::FindParticleNameWithID(const char* particlefolder, unsigned id)
{
	tinydir_dir dir;
	if (tinydir_open(&dir, particlefolder) == -1)
	{
		Log("Failed to open %s", particlefolder);
		return std::string();
	}

	while (dir.has_next)
	{
		tinydir_file file;
		if (tinydir_readfile(&dir, &file) == -1)
			continue;
		StringVector sv = Split(file.name, "_");
		if (sv.size() >= 2)
		{
			unsigned fileid = StringConverter::parseUnsignedInt(sv[0]);
			if (fileid == id)
				return std::string(particlefolder) + "/" + file.name;
		}
		tinydir_next(&dir);
	}
	Log("Cannot find particles which has id %d", id);
	return std::string();
}

//---------------------------------------------------------------------------
ParticleManager::ParticleManager()
:mEditCamIdx(-1)
{
}

//---------------------------------------------------------------------------
ParticleManager::~ParticleManager()
{
	FB_FOREACH(it, mActiveParticles)
	{
		(*it)->Active(false);
	}

	mActiveParticles.clear();
	mPendingDeletes.clear();

	mParticleEmitters.clear();
	mParticleEmittersByName.clear();
	ParticleRenderObject::FinalizeRenderObjects();
}

//---------------------------------------------------------------------------
void ParticleManager::Update(float elapsedTime)
{
	ParticleRenderObject::ClearParticles();

	for each(IParticleEmitter* p in mPendingDeletes)
	{
		mActiveParticles.erase(
			std::remove(mActiveParticles.begin(), mActiveParticles.end(), p), 
			mActiveParticles.end());
	}
	mPendingDeletes.clear();

	if (mEditingParticle)
	{
		if (!mEditingParticle->IsAlive())
		{
			mEditingParticle->Active(true);
		}
		if (gFBEnv->pConsole->GetEngineCommand()->MoveEditParticle)
		{
			float time = gFBEnv->pTimer->GetTime();
			Vec3 movedir = Vec3(cos(time), 0.f, 0.f);
			Vec3 pos = mEditingPos + 20.f * movedir;
			Vec3 oldPos = mEditingParticle->GetPos();
			mEditingParticle->SetPos(pos);
			Vec3 dir = pos - oldPos;
			ICamera* pcam = gFBEnv->pEngine->GetCamera(mEditCamIdx);
			Vec3 campos = pcam->GetPos();
			pcam->SetPos(campos + dir);
			float len = dir.Normalize();
			if (len > 0.f)
			{
				mEditingParticle->SetDir(dir);
			}
		}
		

	}

	Vec3 camDir = gFBEnv->pRenderer->GetCamera()->GetForward();
	Vec3 camPos = gFBEnv->pRenderer->GetCamera()->GetPos();
	std::sort(mActiveParticles.begin(), mActiveParticles.end(), [&camDir, &camPos](IParticleEmitter* a, IParticleEmitter* b)->bool
	{
		float da = (a->GetPos() - camPos).Dot(camDir);
		float db = (b->GetPos() - camPos).Dot(camDir);
		return da > db;
	});

	Emitters::iterator it = mActiveParticles.begin();
	for (; it!=mActiveParticles.end(); )
	{
		bool updated = (*it)->Update(elapsedTime);
		if (!updated)
			it = mActiveParticles.erase(it);
		else
			it++;
	}
}

void ParticleManager::AddActiveParticle(IParticleEmitter* pEmitter)
{
	LOCK_CRITICAL_SECTION lock(mActivePCS);
	mActiveParticles.push_back(pEmitter);	
}

void ParticleManager::RemoveDeactiveParticle(IParticleEmitter* pEmitter)
{
	LOCK_CRITICAL_SECTION lock(mPendingDCS);
	mPendingDeletes.push_back(pEmitter);
}

//---------------------------------------------------------------------------
IParticleEmitter* ParticleManager::GetParticleEmitter(const char* file)
{
	LOCK_CRITICAL_SECTION lock(mGetPNameCS);
	PARTICLE_EMITTERS_BY_NAME::iterator found =mParticleEmittersByName.find(file);
	if (found != mParticleEmittersByName.end())
	{
		return (IParticleEmitter*)found->second->Clone();
	}

	SmartPtr<IParticleEmitter> p = IParticleEmitter::CreateParticleEmitter();
	bool succ = p->Load(file);
	if (succ)
	{
		mParticleEmitters[p->GetEmitterID()] = p;
		mParticleEmittersByName[file] = p;
		return (IParticleEmitter*)p->Clone();
	}
		
	return 0;
}

IParticleEmitter* ParticleManager::GetParticleEmitter(unsigned id)
{
	LOCK_CRITICAL_SECTION lock(mGetPCS);
	PARTICLE_EMITTERS_BY_ID::iterator found = mParticleEmitters.find(id);
	if (found != mParticleEmitters.end())
		return (IParticleEmitter*)found->second->Clone();

	std::string filepath = ParticleManager::FindParticleNameWithID("data/particles", id);
	IParticleEmitter* pemitter = GetParticleEmitter(filepath.c_str());
	return pemitter;
}

void ParticleManager::RenderProfile()
{
	wchar_t msg[255];
	int x = 700;
	int y = 20;
	int yStep = 18;
	IFont* pFont = gFBEnv->pRenderer->GetFont();
	if (pFont)
		yStep = (int)pFont->GetHeight();
	IRenderer* r = gFBEnv->pRenderer;
	swprintf_s(msg, 255, L"Emitter Types= %u", mParticleEmitters.size());
	r->DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
	y += yStep;

	swprintf_s(msg, 255, L"Total Emitter Instances= %u", mActiveParticles.size());
	r->DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
	y += yStep;

	swprintf_s(msg, 255, L"Total Emitter Renderers = %u", ParticleRenderObject::GetNumRenderObject());
	r->DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
	y += yStep;

	swprintf_s(msg, 255, L"Number of Draw Calls = %u", ParticleRenderObject::GetNumDrawCalls());
	r->DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
	y += yStep;

	swprintf_s(msg, 255, L"Number of Draw Vertices = %u", ParticleRenderObject::GetNumPrimitives());
	r->DrawText(Vec2I(x, y), msg, Vec3(1, 1, 1));
	y += yStep;


}

//---------------------------------------------------------------------------
void ParticleManager::ReloadParticle(const char* file)
{
	for (auto& p : mParticleEmittersByName)
	{
		if (stricmp(p.first.c_str(), file)==0)
		{
			std::string lower = file;
			ToLowerCase(lower);
			p.second->Load(lower.c_str());
			ParticleEmitter* editing = (ParticleEmitter*)mEditingParticle.get();
			if (editing && editing->mAdam == p.second)
			{
				mEditingParticle->StopImmediate();
				mEditingParticle = (IParticleEmitter*)p.second->Clone();
				ICamera* pCam = gFBEnv->pEngine->GetCamera(mEditCamIdx);
				if (pCam)
					pCam->SetTarget(mEditingParticle);
				mEditingParticle->Active(true);
			}
		}
	}
}

void ParticleManager::EditThisParticle(const char* file)
{
	if (mEditingParticle)
		mEditingParticle->StopImmediate();

	bool numeric = IsNumeric(file);
	std::string fullpath;
	if (numeric)
	{
		fullpath = FindParticleNameWithID("data/particles", StringConverter::parseInt(file));
	}
	else
	{
		fullpath = file;
		fullpath += ".particle";
	}
	
	
	IParticleEmitter* pnew = 0;
	if (strcmp(file, "0"))
		pnew = GetParticleEmitter(fullpath.c_str());
	 
	if (pnew)
	{
		mEditingParticle = pnew;
		Log("Editing : %s", fullpath.c_str());
		ICamera* oldCam = gFBEnv->pRenderer->GetCamera();
		mEditingParticle->SetPos(oldCam->GetPos() + oldCam->GetDir() * 4.f);
		mEditingParticle->Active(true);
		if (mEditCamIdx == -1)
			mEditCamIdx = gFBEnv->pEngine->CreateCameraAndRegister("ParticleEditCam");
		gFBEnv->pEngine->SetActiveCamera(mEditCamIdx);
		ICamera* pCam = gFBEnv->pEngine->GetCamera(mEditCamIdx);
		mEditingPos = oldCam->GetPos();
		pCam->SetPos(oldCam->GetPos());
		pCam->SetDir(oldCam->GetDir());
		pCam->SetTarget(mEditingParticle);
		pCam->SetEnalbeInput(true);
	}
	else if (strcmp(file,"0")==0)
	{
		gFBEnv->pEngine->SetActiveCamera(0);
		Log("Exit particle editor");
	}
	else
	{
		Log("Cannot find the particle %s", fullpath.c_str());
	}
}

}
