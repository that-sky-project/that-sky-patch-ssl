#pragma once
#include <windows.h>
#include <cstdio>
#include "includes/htmod.h"

namespace xyl {

inline void logv(const char *fmt, va_list ap) {
  char buf[1024];
  vsnprintf(buf, sizeof(buf), fmt, ap);
  OutputDebugStringA("[XYLoader] ");
  OutputDebugStringA(buf);
  OutputDebugStringA("\n");
  HTTellText("§b[XY]§r %s", buf); 
}

inline void log(const char *fmt, ...) {
  va_list ap; va_start(ap, fmt); logv(fmt, ap); va_end(ap);
}

} // namespace xyl

#define XYLOG(...) ::xyl::log(__VA_ARGS__)
