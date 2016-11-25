#pragma once

#include<chat_server\consts.h>
#include<My\WinSockBaseSerialization.h>
#include <msclr/marshal_cppstd.h>

namespace chat_server_tester {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::IO;

	public ref class MyForm : public System::Windows::Forms::Form {
	public:
		MyForm(void)
		{
			InitializeComponent();
			openFileDialog1->InitialDirectory = Environment::CurrentDirectory;
			openFileDialog1->Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*";
			openFileDialog1->FilterIndex = 1;
			openFileDialog1->RestoreDirectory = true;
			openFileDialog1->Multiselect = true;

			saveFileDialog1->Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*";
			saveFileDialog1->FilterIndex = 1;
			saveFileDialog1->RestoreDirectory = true;
		}

	private:

		void load_Click(Object^ sender, EventArgs^ e) {
			Stream^ myStream = nullptr;

			if (openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
			{
				try {
					richTextBox1->Text = "";
					for (int i = 0; i < openFileDialog1->FileNames->Length; i++) {
						richTextBox1->Text += File::ReadAllText(openFileDialog1->FileNames[i]->ToString());
					}
				}
				catch (Exception^ ex) {
					MessageBox::Show("Error: Could not read file from disk. Original error: " + ex->Message);
				}
			}
		}

		void save_Click(Object^ sender, EventArgs^ e) {
			Stream^ myStream = nullptr;

			if (saveFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK)
			{
				File::WriteAllText(openFileDialog1->FileName, richTextBox1->Text);
			}
		}

		void send_Click(Object^ sender, EventArgs^ e) {
			array<String^>^ separator = { L"_read_;" };
			array<String^>^ arr = richTextBox1->Text->Split(separator, StringSplitOptions::RemoveEmptyEntries);
			My::WinSocketAddress address("127.0.0.1", 30000);
			My::WinSocket sock;
			sock.connect(address);
			try {
				for (int i = 0; i < arr->Length; i++) {
					array<String^>^ separator2 = { L"_send_;" };
					array<String^>^ arr2 = arr[i]->Split(separator2, StringSplitOptions::RemoveEmptyEntries);
					
					for (int j = 0; j < arr2->Length; j++) {
						sock << msclr::interop::marshal_as< std::string >(arr2[j]->ToString());
					}
					try {
						std::string response;
						do {
							sock >> response;
							if (!response.size()) break;
							richTextBox2->Text += L"\n\r____________\n\r";
							richTextBox2->Text += msclr::interop::marshal_as< String^ >(response);
						} while (response != END_OF_QUERY_PROCESSING);

					} catch (const My::WinSocketException& ex) {
						sock = My::WinSocket();
						sock.connect(address);
					}
				}
			} catch (const My::WinSocketException& ex) {
				System::Windows::Forms::MessageBox::Show("error in send_Click");
			}
		}

#pragma region Windows Form Designer generated code
	public:
		~MyForm() {
			if (components)
			{
				delete components;
			}
		}
	private:
		System::ComponentModel::Container ^components = nullptr;
		RichTextBox^ richTextBox1;
		RichTextBox^ richTextBox2;
		Label^ label1;
		Label^ label2;
		Button^ button1;
		Button^ button2;
		Button^ button3;
		OpenFileDialog^ openFileDialog1;
		SaveFileDialog^ saveFileDialog1;
		void InitializeComponent(void)
		{
			richTextBox1 = gcnew RichTextBox();
			richTextBox2 = gcnew RichTextBox();
			label1 = gcnew Label();
			label2 = gcnew Label();
			button1 = gcnew Button();
			button2 = gcnew Button();
			button3 = gcnew Button();
			openFileDialog1 = gcnew OpenFileDialog();
			saveFileDialog1 = gcnew SaveFileDialog();
			SuspendLayout();
			// 
			// richTextBox1
			// 
			richTextBox1->Location = System::Drawing::Point(13, 32);
			richTextBox1->Name = "richTextBox1";
			richTextBox1->Size = System::Drawing::Size(290, 277);
			richTextBox1->TabIndex = 0;
			richTextBox1->Text = "";
			// 
			// richTextBox2
			// 
			richTextBox2->Location = System::Drawing::Point(390, 32);
			richTextBox2->Name = "richTextBox2";
			//richTextBox2->ReadOnly = true;
			richTextBox2->Size = System::Drawing::Size(290, 277);
			richTextBox2->TabIndex = 1;
			richTextBox2->Text = "";
			// 
			// label1
			// 
			label1->AutoSize = true;
			label1->Location = System::Drawing::Point(12, 16);
			label1->Name = "label1";
			label1->Size = System::Drawing::Size(47, 13);
			label1->TabIndex = 2;
			label1->Text = "Request";
			// 
			// label2
			// 
			label2->AutoSize = true;
			label2->Location = System::Drawing::Point(387, 16);
			label2->Name = "label2";
			label2->Size = System::Drawing::Size(55, 13);
			label2->TabIndex = 3;
			label2->Text = "Response";
			// 
			// button1
			// 
			button1->Location = System::Drawing::Point(309, 140);
			button1->Name = "button1";
			button1->Size = System::Drawing::Size(75, 23);
			button1->TabIndex = 4;
			button1->Text = "send";
			button1->UseVisualStyleBackColor = true;
			button1->Click += gcnew System::EventHandler(this, &MyForm::send_Click);
			// 
			// button2
			// 
			button2->Location = System::Drawing::Point(111, 324);
			button2->Name = "button2";
			button2->Size = System::Drawing::Size(75, 23);
			button2->TabIndex = 5;
			button2->Text = "load";
			button2->UseVisualStyleBackColor = true;
			button2->Click += gcnew System::EventHandler(this, &MyForm::load_Click);
			// 
			// button3
			// 
			button3->Location = System::Drawing::Point(517, 324);
			button3->Name = "button3";
			button3->Size = System::Drawing::Size(75, 23);
			button3->TabIndex = 6;
			button3->Text = "save";
			button3->UseVisualStyleBackColor = true;
			button3->Click += gcnew System::EventHandler(this, &MyForm::save_Click);
			// 
			// openFileDialog1
			// 
			openFileDialog1->FileName = "openFileDialog1";
			// 
			// Form1
			// 
			AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			ClientSize = System::Drawing::Size(691, 361);
			Controls->Add(button3);
			Controls->Add(button2);
			Controls->Add(button1);
			Controls->Add(label2);
			Controls->Add(label1);
			Controls->Add(richTextBox2);
			Controls->Add(richTextBox1);
			Name = "Form1";
			Text = "chat_server_tester";
			ResumeLayout(false);
			PerformLayout();
		}
#pragma endregion
	};
}
