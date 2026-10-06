#pragma once
#include <windows.h>

namespace xyl::net {

// Resolve and install both hooks. Call once at init (the "custom_server" mode).
bool init(HMODULE self);

// Total hook hits this session (HTTP + WebSocket).
unsigned long long hits();

} // namespace xyl::net
