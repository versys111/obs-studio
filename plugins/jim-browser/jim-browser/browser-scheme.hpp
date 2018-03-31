#pragma once

#include "cef-headers.hpp"
#include <string>
#include <fstream>

class BrowserSchemeHandlerFactory : public CefSchemeHandlerFactory {
public:
	virtual CefRefPtr<CefResourceHandler> Create(
			CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame>,
			const CefString &,
			CefRefPtr<CefRequest> request) override;

	IMPLEMENT_REFCOUNTING(BrowserSchemeHandlerFactory);
};

class BrowserSchemeHandler : public CefResourceHandler {
	std::string fileName;
	std::ifstream inputStream;
	bool isComplete = false;
	int64 length = 0;
	int64 remaining = 0;

public:
	virtual bool ProcessRequest(
			CefRefPtr<CefRequest> request,
			CefRefPtr<CefCallback> callback) override;
	virtual void GetResponseHeaders(
			CefRefPtr<CefResponse> response,
			int64 &response_length,
			CefString &redirectUrl) override;
	virtual bool ReadResponse(
			void *data_out,
			int bytes_to_read,
			int &bytes_read,
			CefRefPtr<CefCallback> callback) override;
	virtual void Cancel() override;

	IMPLEMENT_REFCOUNTING(BrowserSchemeHandler);
};
