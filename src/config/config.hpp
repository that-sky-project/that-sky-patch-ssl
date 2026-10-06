// "type" selects the mode:
//   "none"          - do nothing.
//   "patch_ssl"     - bypass ssl_verify_cert_chain.
//   "custom_server" - redirect account traffic to a custom server (HTTP + WebSocket).
#pragma once
#include <string>

namespace xyl::config {

enum class Mode { None, PatchSsl, CustomServer };

// when a request's domain equals `from`, it is rewritten to `to` (which may be
// "host" or "host:port").
struct DomainMap {
  std::string from;
  std::string to;
};

// Settings for the "custom_server" mode.
struct CustomServer {
  bool      forceHttp = false; // downgrade the protocol to http (wss -> plain ws)
  DomainMap account;           // account-server (HTTP) redirect
  DomainMap ws;                // WebSocket redirect; an empty field inherits `account`

  // Effective WS redirect: an empty ws.from / ws.to falls back to the account's.
  // Leaving the whole `ws` block empty therefore mirrors `account`.
  DomainMap wsResolved() const {
    DomainMap r = ws;
    if (r.from.empty()) r.from = account.from;
    if (r.to.empty())   r.to   = account.to;
    return r;
  }
};

struct Settings {
  Mode         type = Mode::None;
  CustomServer customServer;
};

// Parse/format the "type" enum.
Mode        parseMode(const char *s);
const char *modeName(Mode m);

// Load settings from the JSON file at `absPath`. If the file is missing (or
// unparseable) a default config is written there and the built-in defaults are
// used. Call once at init, before ssl::init(). Returns false only on hard error.
bool load(const char *absPath);

// The active settings (built-in defaults until load() runs).
const Settings &get();

} // namespace xyl::config
