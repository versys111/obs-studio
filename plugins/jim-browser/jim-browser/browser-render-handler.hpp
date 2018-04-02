#pragma once

#include "cef-headers.hpp"
#include "jim-browser-source.hpp"

class BrowserRenderHandler : public CefRenderHandler {
	BrowserSource *bs;
	CefRect popupRect;
	CefRect originalPopupRect;

public:
	inline BrowserRenderHandler(BrowserSource *bs_) : bs(bs_) {}

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

	IMPLEMENT_REFCOUNTING(BrowserRenderHandler);
};
