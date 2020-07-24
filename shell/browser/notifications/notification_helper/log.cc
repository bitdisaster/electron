#include <stdarg.h>
#include <codecvt>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "log.h"

using convert_t = std::codecvt_utf8<wchar_t>;
std::wstring_convert<convert_t, wchar_t> strconverter;

std::string to_string(std::wstring wstr) {
  return strconverter.to_bytes(wstr);
}

std::wstring to_wstring(std::string str) {
  return strconverter.from_bytes(str);
}

// std::string format(const std::string format, ...) {
//  va_list args;
//  va_start(args, format);
//  size_t len = std::vsnprintf(NULL, 0, format.c_str(), args);
//  va_end(args);
//  std::vector<char> vec(len + 1);
//  va_start(args, format);
//  std::vsnprintf(&vec[0], len + 1, format.c_str(), args);
//  va_end(args);
//  return &vec[0];
//}

void xlog(const std::string format, ...) {
  va_list args;
  va_start(args, format);
  size_t len = std::vsnprintf(NULL, 0, format.c_str(), args);
  va_end(args);
  std::vector<char> vec(len + 1);
  va_start(args, format);
  std::vsnprintf(&vec[0], len + 1, format.c_str(), args);
  va_end(args);
  // return &vec[0];
  // auto ss = format(fmt, ...);

  // int size =
  //    ((int)fmt.size()) * 2 + 50;  // Use a rubric appropriate for your code
  //  std::string str;
  //  va_list ap;
  //  while (1) {     // Maximum two passes on a POSIX system...
  //      str.resize(size);
  //      va_start(ap, fmt);
  //      int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
  //      va_end(ap);
  //      if (n > -1 && n < size) {  // Everything worked
  //          str.resize(n);
  //          break;
  //      }
  //      if (n > -1)  // Needed size returned
  //          size = n + 1;   // For null char
  //      else
  //          size *= 2;      // Guess at a larger size (OS specific)
  //  }

  // std::string s = str(format("%2% %2% %1%\n") % "world" % "hello");

  std::ofstream fs("C:\\electron\\src\\out\\Testing\\eee.txt",
                   std::ios_base::app);
  fs << &vec[0] << "\n";
  fs.close();
}