#include "browser-client.hpp"

CefRefPtr<CefRenderHandler> BrowserClient::GetRenderHandler()
{
	return renderHandler;
}

CefRefPtr<CefLoadHandler> BrowserClient::GetLoadHandler()
{
	return loadHandler;
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
	// block popups
	return true;
}

void BrowserClient::OnBeforeContextMenu(
		CefRefPtr<CefBrowser>,
		CefRefPtr<CefFrame>,
		CefRefPtr<CefContextMenuParams>,
		CefRefPtr<CefMenuModel> model)
{
	// remove all context menu contributions
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
