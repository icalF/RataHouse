#include "MyForm.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThread]
void Main(cli::array<String^>^ args)
{
	/* activate GUI */
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(true);

	RataHouse::MyForm form;
	Application::Run(%form);
}

