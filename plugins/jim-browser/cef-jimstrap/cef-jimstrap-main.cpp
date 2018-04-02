#include "cef-headers.hpp"
#include "jim-browser/browser-app.hpp"

#ifdef _WIN32
int CALLBACK WinMain(HINSTANCE, HINSTANCE,
	LPSTR, int)
{
	CefMainArgs mainArgs(nullptr);
#else
int main(int argc, char *argv[])
{
	CefMainArgs mainArgs(argc, argv);
#endif
	CefRefPtr<BrowserApp> mainApp(new BrowserApp());
	return CefExecuteProcess(mainArgs, mainApp.get(), NULL);                   
}
