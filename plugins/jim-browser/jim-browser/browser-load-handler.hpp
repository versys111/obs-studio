#pragma once

#include "cef-headers.hpp"
#include <string>

class BrowserLoadHandler : public CefLoadHandler {
	std::string css;
public:
	inline BrowserLoadHandler(const std::string &css_) : css(css_) {}

	virtual void OnLoadEnd(
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			int httpStatusCode) override;

	IMPLEMENT_REFCOUNTING(BrowserLoadHandler);
};
