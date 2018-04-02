#pragma once

#include <functional>

extern void QueueCEFTask(std::function<void()> task);
