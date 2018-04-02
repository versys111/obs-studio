#include "jim-browser-source.hpp"
#include "browser-scheme.hpp"
#include "browser-client.hpp"
#include "browser-app.hpp"
#include "wide-string.hpp"
#include <util/threading.h>
#include <util/util.hpp>
#include <thread>
#include <mutex>

using namespace std;

static thread manager_thread;
static os_event_t *startupEvent;
extern mutex browser_list_mutex;
extern BrowserSource *first_browser;

/* ========================================================================= */

struct BrowserSource_Windows : public BrowserSource {
	CefRefPtr<CefBrowser> cefBrowser;

	inline BrowserSource_Windows(obs_data_t *settings,
			obs_source_t *source)
		: BrowserSource(settings, source)
	{
	}

	virtual ~BrowserSource_Windows()
	{
		DestroyBrowser();
	}

	virtual void CreateBrowser() override;
	virtual void DestroyBrowser() override;
	virtual void SendMouseClick(
			const struct obs_mouse_event *event,
			int32_t type,
			bool mouse_up,
			uint32_t click_count) override;
	virtual void SendMouseMove(
			const struct obs_mouse_event *event,
			bool mouse_leave) override;
	virtual void SendMouseWheel(
			const struct obs_mouse_event *event,
			int x_delta,
			int y_delta) override;
	virtual void SendFocus(bool focus) override;
	virtual void SendKeyClick(
			const struct obs_key_event *event,
			bool key_up) override;
	virtual void SetShowing(bool showing) override;
	virtual void SetActive(bool active) override;
	virtual void Refresh() override;

	inline void ExecuteOnBrowser(std::function<void()> func);
};

inline void BrowserSource_Windows::ExecuteOnBrowser(std::function<void()> func)
{
	if (!cefBrowser)
		return;

	os_event_t *finishedEvent;
	os_event_init(&finishedEvent, OS_EVENT_TYPE_AUTO);
	QueueCEFTask([&] () {
		func();
		os_event_signal(finishedEvent);
	});
	os_event_wait(finishedEvent);
	os_event_destroy(finishedEvent);
}

void BrowserSource_Windows::CreateBrowser()
{
	os_event_t *createdEvent;
	os_event_init(&createdEvent, OS_EVENT_TYPE_AUTO);

	os_event_wait(startupEvent);

	QueueCEFTask([&] ()
	{
		CefRefPtr<BrowserClient> browserClient = new BrowserClient(this);

		CefWindowInfo windowInfo;
#if CHROME_VERSION_BUILD < 3071
		windowInfo.transparent_painting_enabled = true;
#endif
		windowInfo.width = width;
		windowInfo.height = height;
		windowInfo.windowless_rendering_enabled = true;

		CefBrowserSettings cefBrowserSettings;
		cefBrowserSettings.windowless_frame_rate = fps;

		cefBrowser = CefBrowserHost::CreateBrowserSync(
						windowInfo, 
						browserClient,
						url, 
						cefBrowserSettings,
						nullptr);
		if (cefBrowser != nullptr)
			browserIdentifier = cefBrowser->GetIdentifier();
		os_event_signal(createdEvent);
	});

	os_event_wait(createdEvent);
	os_event_destroy(createdEvent);
}

void BrowserSource_Windows::DestroyBrowser()
{
	if (!cefBrowser)
		return;

	os_event_t *closeEvent;
	os_event_init(&closeEvent, OS_EVENT_TYPE_AUTO);
	QueueCEFTask([&] ()
	{
		/*
		 * This stops rendering
		 * http://magpcss.org/ceforum/viewtopic.php?f=6&t=12079
		 * https://bitbucket.org/chromiumembedded/cef/issues/1363/washidden-api-got-broken-on-branch-2062)
		 */
		cefBrowser->GetHost()->WasHidden(true);
		cefBrowser->GetHost()->CloseBrowser(true);
		os_event_signal(closeEvent);
	});
	os_event_wait(closeEvent);
	os_event_destroy(closeEvent);

	cefBrowser = nullptr;
	browserIdentifier = 0;
}

void BrowserSource_Windows::SendMouseClick(
		const struct obs_mouse_event *event,
		int32_t type,
		bool mouse_up,
		uint32_t click_count)
{
	ExecuteOnBrowser([&] ()
	{
		CefMouseEvent e;
		e.modifiers = event->modifiers;
		e.x = event->x;
		e.y = event->y;
		CefBrowserHost::MouseButtonType buttonType =
			(CefBrowserHost::MouseButtonType)type;
		cefBrowser->GetHost()->SendMouseClickEvent(e, buttonType,
			mouse_up, click_count);
	});
}

void BrowserSource_Windows::SendMouseMove(
		const struct obs_mouse_event *event,
		bool mouse_leave)
{
	ExecuteOnBrowser([&] ()
	{
		CefMouseEvent e;
		e.modifiers = event->modifiers;
		e.x = event->x;
		e.y = event->y;
		cefBrowser->GetHost()->SendMouseMoveEvent(e, mouse_leave);
	});
}

void BrowserSource_Windows::SendMouseWheel(
		const struct obs_mouse_event *event,
		int x_delta,
		int y_delta)
{
	ExecuteOnBrowser([&] ()
	{
		CefMouseEvent e;
		e.modifiers = event->modifiers;
		e.x = event->x;
		e.y = event->y;
		cefBrowser->GetHost()->SendMouseWheelEvent(e, x_delta, y_delta);
	});
}

void BrowserSource_Windows::SendFocus(bool focus)
{
	ExecuteOnBrowser([&] ()
	{
		cefBrowser->GetHost()->SendFocusEvent(focus);
	});
}

void BrowserSource_Windows::SendKeyClick(
		const struct obs_key_event *event,
		bool key_up)
{
	ExecuteOnBrowser([&] ()
	{
		CefKeyEvent e;
		e.windows_key_code = event->native_vkey;
		e.native_key_code = 0;

		e.type = key_up ? KEYEVENT_KEYUP : KEYEVENT_RAWKEYDOWN;

		if (event->text) {
			wstring wide = to_wide(event->text);
			if (wide.size())
				e.character = wide[0];
		}

		//e.native_key_code = event->native_vkey;
		e.modifiers = event->modifiers;

		cefBrowser->GetHost()->SendKeyEvent(e);
		if (event->text && !key_up) {
			e.type = KEYEVENT_CHAR;
			e.windows_key_code = e.character;
			e.character = 0;
			cefBrowser->GetHost()->SendKeyEvent(e);
		}
	});
}

void BrowserSource_Windows::SetShowing(bool showing)
{
	if (!showing)
		cefBrowser->GetHost()->WasHidden(true);

	if (shutdown) {
		if (showing)
			Update();
	} else {
		ExecuteOnBrowser([&] ()
		{
			CefRefPtr<CefProcessMessage> msg =
				CefProcessMessage::Create("Visibility");
			CefRefPtr<CefListValue> args = msg->GetArgumentList();
			args->SetBool(0, showing);
			cefBrowser->SendProcessMessage(PID_RENDERER, msg);
		});
	}

	if (showing) {
		cefBrowser->GetHost()->WasHidden(false);
		cefBrowser->GetHost()->Invalidate(PET_VIEW);
	}
}

void BrowserSource_Windows::SetActive(bool active)
{
	ExecuteOnBrowser([&] ()
	{
		CefRefPtr<CefProcessMessage> msg =
			CefProcessMessage::Create("Active");
		CefRefPtr<CefListValue> args = msg->GetArgumentList();
		args->SetBool(0, active);
		cefBrowser->SendProcessMessage(PID_RENDERER, msg);
	});
}

void BrowserSource_Windows::Refresh()
{
	ExecuteOnBrowser([&] ()
	{
		cefBrowser->ReloadIgnoreCache();
	});
}

/* ========================================================================= */

BrowserSource *CreateBrowserSource(obs_data_t *settings, obs_source_t *source)
{
	return new BrowserSource_Windows(settings, source);
}

static void BrowserManagerThread(void)
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

void InitBrowserManager()
{
	os_event_init(&startupEvent, OS_EVENT_TYPE_MANUAL);
	manager_thread = thread(BrowserManagerThread);
}

void FreeBrowserManager()
{
	QueueCEFTask([] () {
		CefQuitMessageLoop();
	});

	if (manager_thread.joinable())
		manager_thread.join();

	os_event_destroy(startupEvent);
}

static void ExecuteOnAllBrowsers(function<void(BrowserSource_Windows*)> func)
{
	lock_guard<mutex> lock(browser_list_mutex);

	BrowserSource *bs = first_browser;
	while (bs) {
		BrowserSource_Windows *bsw =
			reinterpret_cast<BrowserSource_Windows *>(bs);
		bsw->ExecuteOnBrowser([&] () {func(bsw);});
		bs = bs->next;
	}
}

void DispatchJSEvent(const char *eventName, const char *jsonString)
{
	ExecuteOnAllBrowsers([&] (BrowserSource_Windows *bsw)
	{
		CefRefPtr<CefProcessMessage> msg =
			CefProcessMessage::Create("DispatchJSEvent");
		CefRefPtr<CefListValue> args = msg->GetArgumentList();

		args->SetString(0, eventName);
		args->SetString(1, jsonString);
		bsw->cefBrowser->SendProcessMessage(PID_RENDERER, msg);
	});
}
