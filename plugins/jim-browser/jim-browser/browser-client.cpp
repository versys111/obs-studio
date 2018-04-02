#include "browser-client.hpp"
#include "jim-browser-source.hpp"
#include "base64/base64.hpp"

CefRefPtr<CefRenderHandler> BrowserClient::GetRenderHandler()
{
	return this;
}

CefRefPtr<CefLoadHandler> BrowserClient::GetLoadHandler()
{
	return this;
}

CefRefPtr<CefLifeSpanHandler> BrowserClient::GetLifeSpanHandler()
{
	return this;
}

CefRefPtr<CefContextMenuHandler> BrowserClient::GetContextMenuHandler()
{
	return this;
}

bool BrowserClient::OnBeforePopup(
		CefRefPtr<CefBrowser>,
		CefRefPtr<CefFrame>,
		const CefString &,
		const CefString &,
		WindowOpenDisposition,
		bool,
		const CefPopupFeatures &,
		CefWindowInfo &,
		CefRefPtr<CefClient> &,
		CefBrowserSettings&,
		bool *)
{
	/* block popups */
	return true;
}

void BrowserClient::OnBeforeContextMenu(
		CefRefPtr<CefBrowser>,
		CefRefPtr<CefFrame>,
		CefRefPtr<CefContextMenuParams>,
		CefRefPtr<CefMenuModel> model)
{
	/* remove all context menu contributions */
	model->Clear();
}

bool BrowserClient::OnProcessMessageReceived(
	CefRefPtr<CefBrowser> browser,
	CefProcessId,
	CefRefPtr<CefProcessMessage> message)
{
	// TODO
	return false;
}

bool BrowserClient::GetViewRect(
		CefRefPtr<CefBrowser>,
		CefRect &rect)
{
	rect.Set(0, 0, bs->width, bs->height);
	return true;
}

void BrowserClient::OnPaint(
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

void BrowserClient::OnLoadEnd(
		CefRefPtr<CefBrowser>,
		CefRefPtr<CefFrame> frame,
		int)
{
	if (frame->IsMain()) {
		std::string base64EncodedCSS = base64_encode(bs->css);

		std::string href;
		href += "data:text/css;charset=utf-8;base64,";
		href += base64EncodedCSS;

		std::string script;
		script += "var link = document.createElement('link');";
		script += "link.setAttribute('rel', 'stylesheet');";
		script += "link.setAttribute('type', 'text/css');";
		script += "link.setAttribute('href', '" + href + "');";
		script += "document.getElementsByTagName('head')[0].appendChild(link);";

		frame->ExecuteJavaScript(script, href, 0);
	}
}
