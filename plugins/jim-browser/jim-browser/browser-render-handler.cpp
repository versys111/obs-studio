#include "browser-render-handler.hpp"

bool BrowserRenderHandler::GetViewRect(
		CefRefPtr<CefBrowser>,
		CefRect &rect)
{
	rect.Set(0, 0, bs->width, bs->height);
	return true;
}

void BrowserRenderHandler::OnPaint(
		CefRefPtr<CefBrowser>,
		PaintElementType type,
		const RectList &,
		const void *buffer,
		int width,
		int height)
{
	if (type == PET_VIEW) {
		if (bs->width != width || bs->height != height) {
			obs_enter_graphics();
			bs->DestroyTextures();
			if (width && height) {
				bs->texture = gs_texture_create(
						width, height, GS_RGBA, 1,
						(const uint8_t **)&buffer,
						GS_DYNAMIC);
				bs->width = width;
				bs->height = height;
			}
			obs_leave_graphics();

		} else if (bs->texture) {
			obs_enter_graphics();
			gs_texture_set_image(bs->texture,
					(const uint8_t *)buffer,
					width * 4, false);
			obs_leave_graphics();
		}

	} else if (type == PET_POPUP) {
		if (bs->popup_width != width || bs->popup_height != height) {
			obs_enter_graphics();
			gs_texture_destroy(bs->popup_texture);
			bs->popup_texture = nullptr;

			if (width && height) {
				bs->popup_texture = gs_texture_create(
						width, height, GS_RGBA, 1,
						(const uint8_t **)&buffer,
						GS_DYNAMIC);
				bs->popup_width = width;
				bs->popup_height = height;
			}
			obs_leave_graphics();

		} else if (bs->popup_texture) {
			obs_enter_graphics();
			gs_texture_set_image(bs->popup_texture,
					(const uint8_t *)buffer,
					width * 4, false);
			obs_leave_graphics();
		}
	}
}
