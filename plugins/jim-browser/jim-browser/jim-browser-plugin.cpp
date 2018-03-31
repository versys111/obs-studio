#include <obs-module.h>
#include <util/util.hpp>
#include <thread>

#include "cef-headers.hpp"
#include "browser-scheme.hpp"
#include "browser-app.hpp"

OBS_DECLARE_MODULE()

using namespace std;

static void BrowserManagerThread(void)
{
	std::string path = obs_get_module_binary_path(obs_current_module());
	path = path.substr(0, path.find_last_of('/') + 1);
	path += "//cef-jimstrap";
#ifdef _WIN32
	path += ".exe";
#endif

	CefMainArgs args;
	CefSettings settings;
	settings.log_severity = LOGSEVERITY_VERBOSE;
	settings.windowless_rendering_enabled = true;
	settings.no_sandbox = true;

	BPtr<char> conf_path = obs_module_config_path("");
	CefString(&settings.cache_path) = conf_path;
	CefString(&settings.browser_subprocess_path) = path;

	CefRefPtr<BrowserApp> app(new BrowserApp());
	CefExecuteProcess(args, app, nullptr);
	CefInitialize(args, settings, app, nullptr);

	CefRegisterSchemeHandlerFactory("http", "absolute",
			new BrowserSchemeHandlerFactory());

	CefRunMessageLoop();
	CefShutdown();
}

static thread manager_thread;

bool obs_module_load(void)
{
	manager_thread = thread(BrowserManagerThread);
	return true;
}

void obs_module_unload(void)
{
	if (manager_thread.joinable())
		manager_thread.join();
}
