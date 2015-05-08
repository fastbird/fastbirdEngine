#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Diagnostics;

namespace Controllers {

	/// <summary>
	/// Engine�� ���� ����Դϴ�.
	/// </summary>
	public ref class EngineBridge :  public System::ComponentModel::Component
	{
	public:
		EngineBridge(void)
			: _initilaized(false)
		{
			InitializeComponent();
			//
			//TODO: ������ �ڵ带 ���⿡ �߰��մϴ�.
			//
			InitializeNativeEngine();			
		}
		EngineBridge(System::ComponentModel::IContainer ^container)
			: _initilaized(false)
		{
			/// <summary>
			/// Windows.Forms Ŭ���� �������� �����̳� ������ �ʿ��մϴ�.
			/// </summary>

			container->Add(this);
			InitializeComponent();
			InitializeNativeEngine();
		}

		int InitSwapChain(HANDLE hwnd, int width, int height);
		array<Byte, 2>^ GeneratePerlin(int width, int height, float persistence, float size);

	protected:
		/// <summary>
		/// ��� ���� ��� ���ҽ��� �����մϴ�.
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
		/// �ʼ� �����̳� �����Դϴ�.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// �����̳� ������ �ʿ��� �޼����Դϴ�.
		/// �� �޼����� ������ �ڵ� ������� �������� ���ʽÿ�.
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
