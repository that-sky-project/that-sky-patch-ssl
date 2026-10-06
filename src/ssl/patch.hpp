#pragma once
#include <windows.h>

namespace xyl::ssl {

// Resolve ssl_verify_cert_chain and install the hook. No-op (returns true) when
// disabled in config. Call once at init, after config::load().
bool init(HMODULE self);

// Number of times the verification hook has fired this session.
unsigned long long hits();

} // namespace xyl::ssl
