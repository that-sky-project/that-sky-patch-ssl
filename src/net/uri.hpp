#pragma once
#include "config/config.hpp"
#include "util/log.hpp"
#include <cstring>
#include <string>

namespace xyl::net {

// Net::Uri 
struct NetUri {
  char protocol[8];   // +0
  char domain[128];   // +8
  char port[6];       // +136  (string; empty = default port)
  char path[256];     // +142
};
static_assert(sizeof(NetUri) == 398, "Net::Uri must be 398 bytes");

inline void setField(char *dst, size_t n, const std::string &s) {
  strncpy(dst, s.c_str(), n - 1);
  dst[n - 1] = '\0';
}

// Apply one channel's redirect to a live Net::Uri: force http (global flag) and
// rewrite the domain if it exactly matches `map.from` (mod_hookHttp.c style).
// Non-matching requests (CDN, analytics, shop, …) are left untouched.
// `map.to` may be "host" or "host:port"; the port goes into its own field.
inline void applyRedirect(NetUri *uri, const config::DomainMap &map) {
  if (!uri) return;
  if (config::get().customServer.forceHttp) strcpy(uri->protocol, "http"); // fits protocol[8]
  if (map.from.empty() || map.from != uri->domain) return;

  size_t colon = map.to.rfind(':');
  if (colon != std::string::npos) {
    setField(uri->domain, sizeof(uri->domain), map.to.substr(0, colon));
    setField(uri->port, sizeof(uri->port), map.to.substr(colon + 1));
  } else {
    setField(uri->domain, sizeof(uri->domain), map.to);
    uri->port[0] = '\0'; // no port => use the default (80/443)
  }
  XYLOG("net: domain %s -> %s:%s", map.from.c_str(), uri->domain, uri->port);
}

inline void logUri(const char *channel, const NetUri *uri) {
  if (!uri) return;
  XYLOG("%s %s://%s%s%s%s", channel, uri->protocol, uri->domain,
        uri->port[0] ? ":" : "", uri->port, uri->path);
}

} // namespace xyl::net
