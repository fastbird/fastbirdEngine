#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Diagnostics;

namespace Controllers {

	/// <summary>
	/// Engine에 대한 요약입니다.
	/// </summary>
	public ref class EngineBridge :  public System::ComponentModel::Component
	{
	public:
		EngineBridge(void)
			: _initilaized(false)
		{
			InitializeComponent();
			//
			//TODO: 생성자 코드를 여기에 추가합니다.
			//
			InitializeNativeEngine();			
		}
		EngineBridge(System::ComponentModel::IContainer ^container)
			: _initilaized(false)
		{
			/// <summary>
			/// Windows.Forms 클래스 컴퍼지션 디자이너 지원에 필요합니다.
			/// </summary>

			container->Add(this);
			InitializeComponent();
			InitializeNativeEngine();
		}

		int InitSwapChain(HANDLE hwnd, int width, int height);
		array<Byte, 2>^ GeneratePerlin(int width, int height, float persistence, float size);

	protected:
		/// <summary>
		/// 사용 중인 모든 리소스를 정리합니다.
		/// </summary>
		~EngineBridge()
		{
			if (components)
			{
				delete components;
			}
			FinalizeNativeEngine();
		}

		//---------------------------------------------------------------------------
		void InitializeNativeEngine();

		//---------------------------------------------------------------------------
		void FinalizeNativeEngine();

	private:
		/// <summary>
		/// 필수 디자이너 변수입니다.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// 디자이너 지원에 필요한 메서드입니다.
		/// 이 메서드의 내용을 코드 편집기로 수정하지 마십시오.
		/// </summary>
		void InitializeComponent(void)
		{
			components = gcnew System::ComponentModel::Container();
		}
#pragma endregion

		bool _initilaized;
		fastbird::IEngine* _engine;
	};
}

extern fastbird::GlobalEnv* gFBEnv;
