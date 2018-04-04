#pragma once

#include "cef-headers.hpp"

struct BrowserSource;

class BrowserClient : public CefClient,
                      public CefLifeSpanHandler,
                      public CefContextMenuHandler,
                      public CefRenderHandler,
                      public CefLoadHandler {

public:
	BrowserSource *bs;
	CefRect popupRect;
	CefRect originalPopupRect;

	inline BrowserClient(BrowserSource *bs_) : bs(bs_) {}

	/* CefClient */
	virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override;
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;
	virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler()
		override;
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override;

	virtual bool OnProcessMessageReceived(
			CefRefPtr<CefBrowser> browser,
			CefProcessId source_process,
			CefRefPtr<CefProcessMessage> message) override;

	/* CefLifeSpanHandler */
	virtual bool OnBeforePopup(
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			const CefString &target_url,
			const CefString &target_frame_name,
			WindowOpenDisposition target_disposition,
			bool user_gesture,
			const CefPopupFeatures &popupFeatures,
			CefWindowInfo &windowInfo,
			CefRefPtr<CefClient> &client,
			CefBrowserSettings &settings,
			bool *no_javascript_access) override;

	/* CefContextMenuHandler */
	virtual void OnBeforeContextMenu(
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			CefRefPtr<CefContextMenuParams> params,
			CefRefPtr<CefMenuModel> model);

	/* CefRenderHandler */
	virtual bool GetViewRect(
			CefRefPtr<CefBrowser> browser,
			CefRect &rect) override;
	virtual void OnPaint(
			CefRefPtr<CefBrowser> browser,
			PaintElementType type,
			const RectList &dirtyRects,
			const void *buffer,
			int width,
			int height) override;

	/* CefLoadHandler */
	virtual void OnLoadEnd(
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			int httpStatusCode) override;

	IMPLEMENT_REFCOUNTING(BrowserClient);
};
