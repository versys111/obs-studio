#include "browser-load-handler.hpp"
#include "base64/base64.hpp"

void BrowserLoadHandler::OnLoadEnd(
		CefRefPtr<CefBrowser>,
		CefRefPtr<CefFrame> frame,
		int)
{
	if (frame->IsMain()) {
		std::string base64EncodedCSS = base64_encode(css);

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
