#include <obs-module.h>
#include <obs-frontend-api.h>
#include "json11/json11.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("jim-browser", "en-US")

using namespace json11;

extern void InitBrowserManager();
extern void FreeBrowserManager();
extern void RegisterBrowserSource();
extern void DispatchJSEvent(const char *eventName, const char *jsonString);

static void handle_obs_frontend_event(enum obs_frontend_event event, void *)
{
	switch (event) {
	case OBS_FRONTEND_EVENT_STREAMING_STARTING:
		DispatchJSEvent("obsStreamingStarting", nullptr);
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STARTED:
		DispatchJSEvent("obsStreamingStarted", nullptr);
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STOPPING:
		DispatchJSEvent("obsStreamingStopping", nullptr);
		break;
	case OBS_FRONTEND_EVENT_STREAMING_STOPPED:
		DispatchJSEvent("obsStreamingStopped", nullptr);
		break;
	case OBS_FRONTEND_EVENT_RECORDING_STARTING:
		DispatchJSEvent("obsRecordingStarting", nullptr);
		break;
	case OBS_FRONTEND_EVENT_RECORDING_STARTED:
		DispatchJSEvent("obsRecordingStarted", nullptr);
		break;
	case OBS_FRONTEND_EVENT_RECORDING_STOPPING:
		DispatchJSEvent("obsRecordingStopping", nullptr);
		break;
	case OBS_FRONTEND_EVENT_RECORDING_STOPPED:
		DispatchJSEvent("obsRecordingStopped", nullptr);
		break;
	case OBS_FRONTEND_EVENT_SCENE_CHANGED:
		{
			obs_source_t *source = obs_frontend_get_current_scene();

			Json json = Json::object {
				{"name", obs_source_get_name(source)},
				{"width", (int)obs_source_get_width(source)},
				{"height", (int)obs_source_get_height(source)}
			};

			obs_source_release(source);

			DispatchJSEvent("obsSceneChanged", json.dump().c_str());
			break;
		}
	case OBS_FRONTEND_EVENT_EXIT:
		DispatchJSEvent("obsExit", nullptr);
		break;
	default:;
	}
}

bool obs_module_load(void)
{
	InitBrowserManager();
	RegisterBrowserSource();
	obs_frontend_add_event_callback(handle_obs_frontend_event, nullptr);
	return true;
}

void obs_module_unload(void)
{
	FreeBrowserManager();
}
