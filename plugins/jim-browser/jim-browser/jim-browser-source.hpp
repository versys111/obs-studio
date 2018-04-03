#pragma once

#include "cef-headers.hpp"

#include <obs-module.h>
#include <string>
#include <functional>

struct BrowserSource {
	BrowserSource         **p_prev_next            = nullptr;
	BrowserSource         *next                    = nullptr;

	obs_source_t          *source                  = nullptr;

	int                   browserIdentifier        = 0;

	std::string           url;
	std::string           css;
	gs_texture_t          *texture                 = nullptr;
	gs_texture_t          *popup_texture           = nullptr;
	int                   width                    = 0;
	int                   height                   = 0;
	int                   fps                      = 0;
	int                   popup_width              = 0;
	int                   popup_height             = 0;
	bool                  restart                  = false;
	bool                  shutdown                 = false;
	bool                  isLocalFile              = false;

	inline void DestroyTextures()
	{
		if (texture || popup_texture) {
			obs_enter_graphics();
			gs_texture_destroy(texture);
			gs_texture_destroy(popup_texture);
			texture = nullptr;
			popup_texture = nullptr;
			popup_width = popup_height = 0;
			obs_leave_graphics();
		}
	}

	/* ---------------------------- */

	virtual void CreateBrowser()=0;
	virtual void DestroyBrowser()=0;

	/* ---------------------------- */

	BrowserSource(obs_data_t *settings, obs_source_t *source);
	virtual ~BrowserSource();

	void Update(obs_data_t *settings = nullptr);
	inline void Render();
	virtual void SendMouseClick(
			const struct obs_mouse_event *event,
			int32_t type,
			bool mouse_up,
			uint32_t click_count)=0;
	virtual void SendMouseMove(
			const struct obs_mouse_event *event,
			bool mouse_leave)=0;
	virtual void SendMouseWheel(
			const struct obs_mouse_event *event,
			int x_delta,
			int y_delta)=0;
	virtual void SendFocus(bool focus)=0;
	virtual void SendKeyClick(
			const struct obs_key_event *event,
			bool key_up)=0;
	virtual void SetShowing(bool showing)=0;
	virtual void SetActive(bool active)=0;
	virtual void Refresh()=0;
};

extern void QueueCEFTask(std::function<void()> task);
extern BrowserSource *CreateBrowserSource(obs_data_t *settings,
		obs_source_t *source);
