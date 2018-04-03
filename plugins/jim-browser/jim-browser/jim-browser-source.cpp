#include "cef-headers.hpp"
#include "jim-browser-source.hpp"
#include <util/dstr.hpp>
#include <mutex>

using namespace std;

/* ========================================================================= */

class BrowserTask : public CefTask {
public:
	std::function<void()> task;

	inline BrowserTask(std::function<void()> task_) : task(task_) {}
	virtual void Execute() override {task();}

	IMPLEMENT_REFCOUNTING(BrowserTask);
};

void QueueCEFTask(std::function<void()> task)
{
	CefPostTask(TID_UI, CefRefPtr<BrowserTask>(new BrowserTask(task)));
}

/* ========================================================================= */

mutex browser_list_mutex;
BrowserSource *first_browser = nullptr;

BrowserSource::BrowserSource(obs_data_t *, obs_source_t *source_)
	: source(source_)
{
	/* defer update */
	obs_source_update(source, nullptr);

	lock_guard<mutex> lock(browser_list_mutex);
	p_prev_next = &first_browser;
	next = first_browser;
	if (first_browser) first_browser->p_prev_next = &next;
	first_browser = this;
}

BrowserSource::~BrowserSource()
{
	DestroyTextures();

	lock_guard<mutex> lock(browser_list_mutex);
	if (next) next->p_prev_next = p_prev_next;
	*p_prev_next = next;
}

void BrowserSource::Update(obs_data_t *settings)
{
	if (settings) {
		isLocalFile = obs_data_get_bool(settings, "is_local_file");
		width       = (int)obs_data_get_int(settings, "width");
		height      = (int)obs_data_get_int(settings, "height");
		fps         = (int)obs_data_get_int(settings, "fps");
		shutdown    = obs_data_get_bool(settings, "shutdown");
		restart     = obs_data_get_bool(settings, "restart_when_active");
		url         = obs_data_get_string(settings,
				isLocalFile ? "local_file" : "url");
	}

	DestroyBrowser();
	DestroyTextures();
	CreateBrowser();
}

inline void BrowserSource::Render()
{
	if (!texture)
		return;

#ifdef __APPLE__
#else
	gs_effect_t *effect = obs_get_base_effect(
			OBS_EFFECT_PREMULTIPLIED_ALPHA);
#endif

	while (gs_effect_loop(effect, "Draw"))
		obs_source_draw(texture, 0, 0, 0, 0, 0);
}

/* ========================================================================= */

static const char *default_css = "\
body { \
background-color: rgba(0, 0, 0, 0); \
margin: 0px auto; \
overflow: hidden; \
}";

static void browser_source_get_defaults(obs_data_t *settings)
{
	obs_data_set_default_string(settings, "url",
			"https://www.obsproject.com");
	obs_data_set_default_int(settings, "width", 800);
	obs_data_set_default_int(settings, "height", 600);
	obs_data_set_default_int(settings, "fps", 30);
	obs_data_set_default_bool(settings, "shutdown", false);
	obs_data_set_default_bool(settings, "restart_when_active", false);
	obs_data_set_default_string(settings, "css", default_css);
}

static bool is_local_file_modified(obs_properties_t *props,
		obs_property_t *, obs_data_t *settings)
{
	bool enabled = obs_data_get_bool(settings, "is_local_file");
	obs_property_t *url = obs_properties_get(props, "url");
	obs_property_t *local_file = obs_properties_get(props, "local_file");
	obs_property_set_visible(url, !enabled);
	obs_property_set_visible(local_file, enabled);

	return true;
}

static obs_properties_t *browser_source_get_properties(void *data)
{
	obs_properties_t *props = obs_properties_create();
	BrowserSource *bs = static_cast<BrowserSource *>(data);
	DStr path;

	obs_properties_set_flags(props, OBS_PROPERTIES_DEFER_UPDATE);
	obs_property_t *prop = obs_properties_add_bool(props, "is_local_file",
			obs_module_text("LocalFile"));

	if (bs && !bs->url.empty()) {
		const char *slash;

		dstr_copy(path, bs->url.c_str());
		dstr_replace(path, "\\", "/");
		slash = strrchr(path->array, '/');
		if (slash)
			dstr_resize(path, slash - path->array + 1);
	}

	obs_property_set_modified_callback(prop, is_local_file_modified);
	obs_properties_add_path(props, "local_file",
			obs_module_text("Local file"), OBS_PATH_FILE, "*.*",
			path->array);
	obs_properties_add_text(props, "url",
			obs_module_text("URL"), OBS_TEXT_DEFAULT);

	obs_properties_add_int(props, "width",
			obs_module_text("Width"), 1, 4096, 1);
	obs_properties_add_int(props, "height",
			obs_module_text("Height"), 1, 4096, 1);
	obs_properties_add_int(props, "fps",
			obs_module_text("FPS"), 1, 60, 1);
	obs_properties_add_text(props, "css",
			obs_module_text("CSS"), OBS_TEXT_MULTILINE);
	obs_properties_add_bool(props, "shutdown",
			obs_module_text("ShutdownSourceNotVisible"));
	obs_properties_add_bool(props, "restart_when_active",
			obs_module_text("RefreshBrowserActive"));

	obs_properties_add_button(props, "refreshnocache",
			obs_module_text("RefreshNoCache"),
			[] (obs_properties_t *, obs_property_t *, void *data)
			{
				static_cast<BrowserSource *>(data)->Refresh();
				return false;
			});
	return props;
}

void RegisterBrowserSource()
{
	struct obs_source_info info = {};
	info.id                     = "browser_source--jim";
	info.type                   = OBS_SOURCE_TYPE_INPUT;
	info.output_flags           = OBS_SOURCE_VIDEO |
	                              OBS_SOURCE_CUSTOM_DRAW |
	                              OBS_SOURCE_INTERACTION |
	                              OBS_SOURCE_DO_NOT_DUPLICATE;
	info.get_properties         = browser_source_get_properties;
	info.get_defaults           = browser_source_get_defaults;

	info.get_name = [] (void *)
	{
		return obs_module_text("BrowserSource");
	};
	info.create = [] (obs_data_t *settings, obs_source_t *source) -> void *
	{
		return CreateBrowserSource(settings, source);
	};
	info.destroy = [] (void *data)
	{
		delete static_cast<BrowserSource *>(data);
	};
	info.update = [] (void *data, obs_data_t *settings)
	{
		static_cast<BrowserSource *>(data)->Update(settings);
	};
	info.get_width = [] (void *data)
	{
		return (uint32_t)static_cast<BrowserSource *>(data)->width;
	};
	info.get_height = [] (void *data)
	{
		return (uint32_t)static_cast<BrowserSource *>(data)->height;
	};
	info.video_render = [] (void *data, gs_effect_t *)
	{
		static_cast<BrowserSource *>(data)->Render();
	};
	info.mouse_click = [] (
			void *data,
			const struct obs_mouse_event *event,
			int32_t type,
			bool mouse_up,
			uint32_t click_count)
	{
		static_cast<BrowserSource *>(data)->SendMouseClick(
				event, type, mouse_up, click_count);
	};
	info.mouse_move = [] (
			void *data,
			const struct obs_mouse_event *event,
			bool mouse_leave)
	{
		static_cast<BrowserSource *>(data)->SendMouseMove(
				event, mouse_leave);
	};
	info.mouse_wheel = [] (
			void *data,
			const struct obs_mouse_event *event,
			int x_delta,
			int y_delta)
	{
		static_cast<BrowserSource *>(data)->SendMouseWheel(
				event, x_delta, y_delta);
	};
	info.focus = [] (void *data, bool focus)
	{
		static_cast<BrowserSource *>(data)->SendFocus(focus);
	};
	info.key_click = [] (
			void *data,
			const struct obs_key_event *event,
			bool key_up)
	{
		static_cast<BrowserSource *>(data)->SendKeyClick(event, key_up);
	};
	info.show = [] (void *data)
	{
		static_cast<BrowserSource *>(data)->SetShowing(true);
	};
	info.hide = [] (void *data)
	{
		static_cast<BrowserSource *>(data)->SetShowing(false);
	};
	info.activate = [] (void *data)
	{
		BrowserSource *bs = static_cast<BrowserSource *>(data);
		if (bs->restart)
			bs->Refresh();
		bs->SetActive(true);
	};
	info.deactivate = [] (void *data)
	{
		static_cast<BrowserSource *>(data)->SetActive(false);
	};

	obs_register_source(&info);
}
