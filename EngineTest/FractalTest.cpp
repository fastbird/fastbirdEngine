#include "stdafx.h"
#include "FractalTest.h"
#include "FBRenderer/Renderer.h"
#include "FBRenderer/Shader.h"
#include "EssentialEngineData/shaders/CommonDefines.h"
#include "EssentialEngineData/shaders/Constants.h"
#include "FBSerializationLib/Serialization.h"
#include "FBRenderer/GeometryBuffer.h"
#include "FBRenderer/Material.h"
#include "FBRenderer/Camera.h"
using namespace fb;

class FractalTest::Impl {
public:
	ShaderPtr mFractalPS;
	GeometryBuffer mSphere;
	MaterialPtr mMaterial;
	Impl() {
		//GenerateGradients("Gradiants_256_extended.dat");		
		//GeneratePermutation("Permutation_256_extented.dat");
		
		mSphere = GeometryBuffer::CreateSphereMesh(10.f, 10, 20);
		mMaterial = Renderer::GetInstance().CreateMaterial("EssentialEngineData/materials/FractalSphere.material");
		auto cam = Renderer::GetInstance().GetCamera();
		//cam->SetTargetPos(Vec3(0, 0, 0));
	}
	void BeforeUIRendering(HWindowId hwndId, HWindow hwnd) {
		if (mMaterial) {
			auto& renderer = Renderer::GetInstance();
			auto cam = renderer.GetCamera();			
			OBJECT_CONSTANTS objConstants = {
				Mat44::IDENTITY,
				cam->GetMatrix(ICamera::MatrixType::View),
				cam->GetMatrix(ICamera::MatrixType::ViewProj),
			};
			renderer.UpdateObjectConstantsBuffer(&objConstants);

			mSphere.Bind();
			mMaterial->Bind();
			
			renderer.BindSystemTexture(SystemTextures::Permutation);
			renderer.BindSystemTexture(SystemTextures::ValueNoise);
			renderer.DrawIndexed(mSphere.mIndexBuffer->GetNumIndices(), 0, 0);						
		}		
	}

	void GeneratePermutation(const char* filepath) {
		// Generate Permutation
		ByteArray p(NUM_PERM + NUM_PERM + 2);
		std::iota(p.begin(), p.begin() + NUM_PERM, 0);
		for (int i = 0; i < NUM_PERM; i += 2) {
			auto j = Random(0, NUM_PERM - 1);
			std::swap(p[i], p[j]);
		}

		for (int i = 0; i < NUM_PERM + 2; ++i) {
			p[NUM_PERM + i] = p[i];
		}
		std::ofstream file(filepath, std::ios_base::binary);
		write(file, p);
	}

	void GenerateGradients(const char* filepath) {
		// Generate Gradients
		std::vector<Vec4> gradiants(NUM_PERM + NUM_PERM + 2);
		std::srand(1);
		float s;
		Vec3 v;
		for (int i = 0; i < NUM_PERM; ++i) {
			do {
				for (int j = 0; j < 3; ++j) {
					v[j] = Random(-1.f, 1.f);
				}
				s = v.Dot(v);
			} while (s > 1.0f);
			s = sqrt(s);
			gradiants[i] = Vec4(v / s);
		}

		for (int i = 0; i < NUM_PERM + 2; ++i) {			
			gradiants[NUM_PERM + i] = gradiants[i];
		}		
		std::ofstream file(filepath, std::ios_base::binary);
		write(file, gradiants);
	}
};

FB_IMPLEMENT_STATIC_CREATE(FractalTest);

FractalTest::FractalTest() 
	: mImpl(new Impl)
{

}
FractalTest::~FractalTest() = default;


void FractalTest::BeforeUIRendering(HWindowId hwndId, HWindow hwnd) {
	mImpl->BeforeUIRendering(hwndId, hwnd);
	
}	