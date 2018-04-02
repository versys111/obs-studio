#include "jim-browser-source.hpp"
#include "browser-render-handler.hpp"
#include "browser-load-handler.hpp"
#include "browser-scheme.hpp"
#include "browser-client.hpp"
#include "browser-task.hpp"
#include "browser-app.hpp"
#include <util/threading.h>
#include <util/util.hpp>

using namespace std;

extern os_event_t                      *startupEvent;
extern map<int, CefRefPtr<CefBrowser>> browserMap;

void BrowserManagerThread(void)
{
	string path = obs_get_module_binary_path(obs_current_module());
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
	os_event_signal(startupEvent);
	CefRunMessageLoop();
	CefShutdown();
}

void BrowserSource::CreateBrowser()
{
	os_event_t *createdEvent;
	os_event_init(&createdEvent, OS_EVENT_TYPE_AUTO);

	os_event_wait(startupEvent);

	QueueCEFTask([&] ()
	{
		CefRefPtr<BrowserRenderHandler> renderHandler =
			new BrowserRenderHandler(this);
		CefRefPtr<BrowserLoadHandler> loadHandler =
			new BrowserLoadHandler(css);
		CefRefPtr<BrowserClient> browserClient =
			new BrowserClient(renderHandler, loadHandler);

		CefWindowInfo windowInfo;
#if CHROME_VERSION_BUILD < 3071
		windowInfo.transparent_painting_enabled = true;
#endif
		windowInfo.width = width;
		windowInfo.height = height;
		windowInfo.windowless_rendering_enabled = true;

		CefBrowserSettings cefBrowserSettings;
		cefBrowserSettings.windowless_frame_rate = fps;

		CefRefPtr<CefBrowser> cefBrowser =
				CefBrowserHost::CreateBrowserSync(
						windowInfo, 
						browserClient,
						url, 
						cefBrowserSettings,
						nullptr);
		if (cefBrowser != nullptr) {
			browserIdentifier = cefBrowser->GetIdentifier();
			browserMap[browserIdentifier] = cefBrowser;
		}
		os_event_signal(createdEvent);
	});

	os_event_wait(createdEvent);
	os_event_destroy(createdEvent);
}

void BrowserSource::DestroyBrowser()
{
}
