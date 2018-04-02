#include "jim-browser-source.hpp"
#include "browser-task.hpp"
#include <util/dstr.hpp>

/* ========================================================================= */

BrowserSource::BrowserSource(obs_data_t *settings, obs_source_t *source_)
	: source(source_)
{
	Update(settings);
}

BrowserSource::~BrowserSource()
{
	DestroyTextures();
}

void BrowserSource::Update(obs_data_t *settings)
{
	if (settings) {
		isLocalFile = obs_data_get_bool(settings, "is_local_file");
		url         = obs_data_get_string(settings,
				isLocalFile ? "local_file" : "url");
		width       = (int)obs_data_get_int(settings, "width");
		height      = (int)obs_data_get_int(settings, "height");
		fps         = (int)obs_data_get_int(settings, "fps");
		shutdown    = obs_data_get_bool(settings, "shutdown");
	}

	if (active) {
		// TODO: Stop Browser
		DestroyTextures();
	}

	// TODO: Start Browser
}

void BrowserSource::Render()
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
body {\
	background-color: rgba(0, 0, 0, 0);\
	margin: 0px auto;\
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
	return props;
}

void RegisterBrowserSource(void)
{
	struct obs_source_info info = {};
	info.id                     = "browser_source--jim";
	info.type                   = OBS_SOURCE_TYPE_INPUT;
	info.output_flags           = OBS_SOURCE_VIDEO |
	                              OBS_SOURCE_CUSTOM_DRAW |
	                              OBS_SOURCE_DO_NOT_DUPLICATE;
	info.get_properties         = browser_source_get_properties;
	info.get_defaults           = browser_source_get_defaults;

	info.get_name = [] (void *)
	{
		return obs_module_text("BrowserSource");
	};
	info.create = [] (obs_data_t *settings, obs_source_t *source) -> void *
	{
		return new BrowserSource(settings, source);
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

	obs_register_source(&info);
}
