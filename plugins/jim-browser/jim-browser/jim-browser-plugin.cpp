#include <obs-module.h>
#include <util/threading.h>
#include <thread>
#include <map>

#include "cef-headers.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("jim-browser", "en-US")

using namespace std;

extern void BrowserManagerThread(void);

static thread                   manager_thread;
os_event_t                      *startupEvent;
map<int, CefRefPtr<CefBrowser>> browserMap;

bool obs_module_load(void)
{
	os_event_init(&startupEvent, OS_EVENT_TYPE_MANUAL);
	manager_thread = thread(BrowserManagerThread);
	return true;
}

void obs_module_unload(void)
{
	if (manager_thread.joinable())
		manager_thread.join();

	os_event_destroy(startupEvent);
}
