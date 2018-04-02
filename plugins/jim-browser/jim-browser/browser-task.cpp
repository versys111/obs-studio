#include "browser-task.hpp"
#include "cef-headers.hpp"

class BrowserTask : public CefTask {
public:
	std::function<void()> task;

	inline BrowserTask(std::function<void()> task_) : task(task_) {}
	virtual void Execute() override {task();}

	IMPLEMENT_REFCOUNTING(BrowserTask);
};

void QueueCEFTask(std::function<void()> task)
{
	CefPostTask(TID_UI, CefRefPtr<BrowserTask>(new BrowserTask(task)));
}
