#pragma once

#include <obs-module.h>
#include <string>
#include <functional>

struct BrowserSource {
	obs_source_t   *source                  = nullptr;

	std::string    url;
	std::string    css;
	gs_texture_t   *texture                 = nullptr;
	gs_texture_t   *popup_texture           = nullptr;
	int            browserIdentifier        = 0;
	int            width                    = 0;
	int            height                   = 0;
	int            fps                      = 0;
	int            popup_width              = 0;
	int            popup_height             = 0;
	bool           shutdown                 = false;
	bool           isLocalFile              = false;
	bool           active                   = false;

	inline void DestroyTextures()
	{
		if (texture || popup_texture) {
			obs_enter_graphics();
			gs_texture_destroy(texture);
			gs_texture_destroy(popup_texture);
			texture = nullptr;
			popup_texture = nullptr;
			width = height = popup_width = popup_height = 0;
			obs_leave_graphics();
		}
	}

	/* ---------------------------- */

	void CreateBrowser();
	void DestroyBrowser();

	/* ---------------------------- */

	BrowserSource(obs_data_t *settings, obs_source_t *source);
	~BrowserSource();

	void Update(obs_data_t *settings = nullptr);
	void Render();
};

extern void QueueCEFTask(std::function<void()> task);
