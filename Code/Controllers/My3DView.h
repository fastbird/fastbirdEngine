#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::Diagnostics;

namespace Controllers {

	/// <summary>
	/// My3DView에 대한 요약입니다.
	/// </summary>
	public ref class My3DView : public System::Windows::Forms::UserControl
	{
	public:
		My3DView(void)
			: _swapChain(0)
		{
			InitializeComponent();
			//
			//TODO: 생성자 코드를 여기에 추가합니다.
			//
		}

	protected:
		/// <summary>
		/// 사용 중인 모든 리소스를 정리합니다.
		/// </summary>
		~My3DView()
		{
			if (components)
			{
				delete components;
			}
		}

	private:
		/// <summary>
		/// 필수 디자이너 변수입니다.
		/// </summary>
		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// 디자이너 지원에 필요한 메서드입니다.
		/// 이 메서드의 내용을 코드 편집기로 수정하지 마십시오.
		/// </summary>
		void InitializeComponent(void)
		{
			this->SuspendLayout();
			// 
			// My3DView
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(7, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->Name = L"My3DView";
			this->Load += gcnew System::EventHandler(this, &My3DView::My3DView_Load);
			this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &My3DView::My3DView_Paint);
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void My3DView_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) 
			 {
				 try
				 {
					 OnPaint();
				 }
				 catch (...)
				 {
				 }
			 }

	private:
		int _swapChain;
	private: 
		System::Void My3DView_Load(System::Object^  sender, System::EventArgs^  e) 
		{
			try
			{
				InitSwapChain();
			}
			catch(...)
			{

			}
			
		}

		void InitSwapChain();
		void OnPaint();
	};
}
