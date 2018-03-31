#include "wide-string.hpp"
#include <util/platform.h>

using namespace std;

wstring to_wide(const char *utf8)
{
	if (!utf8 || !*utf8)
		return wstring();

	size_t isize = strlen(utf8);
	size_t osize = os_utf8_to_wcs(utf8, isize, nullptr, 0);

	if (!osize)
		return wstring();

	wstring wide;
	wide.resize(osize);
	os_utf8_to_wcs(utf8, isize, &wide[0], osize);
	return wide;
}

wstring to_wide(const std::string &utf8)
{
	if (utf8.empty())
		return wstring();

	size_t osize = os_utf8_to_wcs(utf8.c_str(), utf8.size(), nullptr, 0);

	if (!osize)
		return wstring();

	wstring wide;
	wide.resize(osize);
	os_utf8_to_wcs(utf8.c_str(), utf8.size(), &wide[0], osize);
	return wide;
}
