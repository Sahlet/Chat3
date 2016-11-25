#include <Log.h>
#include "MyForm.h"

using namespace chat_server_tester;

[STAThread]

void main(array<String^>^ args) {
	LOG_FILES::out = stdout;
	LOG_FILES::warn = stdout;
	LOG_FILES::err = stderr;

	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(true);
	MyForm form;
	Application::Run(%form);
}