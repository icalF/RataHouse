#include "Form.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThread]
void Main(array<String^>^ args)
{
	/* activate GUI */
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(true);

	SmartGrid::MyForm form;
	Application::Run(%form);
}
