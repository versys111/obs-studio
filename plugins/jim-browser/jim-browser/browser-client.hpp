#pragma once

#include "cef-headers.hpp"

struct BrowserSource;
class BrowserRenderHandler;
class BrowserLoadHandler;

class BrowserClient : public CefClient,
                      public CefLifeSpanHandler,
                      public CefContextMenuHandler {

	CefRefPtr<CefRenderHandler> renderHandler;
	CefRefPtr<CefLoadHandler> loadHandler;

public:
	inline BrowserClient(CefRenderHandler *renderHandler_,
	                     CefLoadHandler *loadHandler_)
		: renderHandler (renderHandler_),
		  loadHandler   (loadHandler_)
	{
	}

	virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override;
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;
	virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler()
		override;
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() override;

	virtual bool OnProcessMessageReceived(
			CefRefPtr<CefBrowser> browser,
			CefProcessId source_process,
			CefRefPtr<CefProcessMessage> message) override;
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
	virtual void OnBeforeContextMenu(
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			CefRefPtr<CefContextMenuParams> params,
			CefRefPtr<CefMenuModel> model);

	IMPLEMENT_REFCOUNTING(BrowserClient);
};
