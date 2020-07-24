#include <codecvt>
#include <string>

#ifndef HELPER_LOGGER
#define HELPER_LOGGER

void xlog(const std::string format, ...);

std::string to_string(std::wstring wstr);
std::wstring to_wstring(std::string str);
#endif  // HELPER_LOGGER
