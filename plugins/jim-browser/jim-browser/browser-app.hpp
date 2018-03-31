#pragma once

#include <map>
#include "cef-headers.hpp"

class BrowserApp : public CefApp,
                   public CefRenderProcessHandler,
                   public CefV8Handler {

	void ExecuteJSFunction(CefRefPtr<CefBrowser> browser,
			const char *functionName,
			CefV8ValueList arguments);

	typedef std::map<int, CefRefPtr<CefV8Value>> CallbackMap;

	CallbackMap callbackMap;
	int callbackId;

public:
	virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override;
	virtual void OnRegisterCustomSchemes(
			CefRawPtr<CefSchemeRegistrar> registrar) override;
	virtual void OnBeforeCommandLineProcessing(
			const CefString &process_type,
			CefRefPtr<CefCommandLine> command_line) override;
	virtual void OnContextCreated(CefRefPtr<CefBrowser> browser,
			CefRefPtr<CefFrame> frame,
			CefRefPtr<CefV8Context> context) override;
	virtual bool OnProcessMessageReceived(
			CefRefPtr<CefBrowser> browser,
			CefProcessId source_process,
			CefRefPtr<CefProcessMessage> message) override;
	virtual bool Execute(const CefString &name,
			CefRefPtr<CefV8Value> object,
			const CefV8ValueList &arguments,
			CefRefPtr<CefV8Value> &retval,
			CefString &exception) override;

	IMPLEMENT_REFCOUNTING(BrowserApp);
};
