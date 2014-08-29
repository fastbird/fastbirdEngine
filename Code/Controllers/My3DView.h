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
	/// My3DView�� ���� ����Դϴ�.
	/// </summary>
	public ref class My3DView : public System::Windows::Forms::UserControl
	{
	public:
		My3DView(void)
			: _swapChain(0)
		{
			InitializeComponent();
			//
			//TODO: ������ �ڵ带 ���⿡ �߰��մϴ�.
			//
		}

	protected:
		/// <summary>
		/// ��� ���� ��� ���ҽ��� �����մϴ�.
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
		/// �ʼ� �����̳� �����Դϴ�.
		/// </summary>
		System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// �����̳� ������ �ʿ��� �޼����Դϴ�.
		/// �� �޼����� ������ �ڵ� ������� �������� ���ʽÿ�.
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
