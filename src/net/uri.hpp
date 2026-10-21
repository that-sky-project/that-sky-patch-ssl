#pragma once
#include "config/config.hpp"
#include "util/log.hpp"
#include <cstddef>
#include <cstring>
#include <string>

namespace xyl::net {

// Net::Uri 
struct NetUri {
  char protocol[10];  // +0
  char domain[128];   // +10
  char port[6];       // +138  (string; empty = default port)
  char path[256];     // +144
};
static_assert(sizeof(NetUri) == 400, "Net::Uri must be 400 bytes");
static_assert(offsetof(NetUri, domain) == 10, "Net::Uri domain offset must be 10");
static_assert(offsetof(NetUri, port) == 138, "Net::Uri port offset must be 138");
static_assert(offsetof(NetUri, path) == 144, "Net::Uri path offset must be 144");

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
  if (config::get().customServer.forceHttp) strcpy(uri->protocol, "http"); // fits protocol[10]
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

// URL-string redirect for the account WebSocket Connect overload.
// `url` looks like "https://host[:port]/account/ws". When the host exactly
// matches `map.from` it is rewritten to `map.to` ("host" or "host:port").
// force_http downgrades the scheme to http. The path is preserved.
// Writes into dst (always NUL-terminated when n > 0) and returns true when
// the URL was changed. `url` and `dst` may alias.
inline bool applyRedirectUrl(const char *url, char *dst, size_t n,
                             const config::DomainMap &map) {
  if (!url || !dst || n == 0) return false;
  const bool forceHttp = config::get().customServer.forceHttp;

  const char *schemeEnd = strstr(url, "://");
  if (!schemeEnd) {
    snprintf(dst, n, "%s", url);
    return false;
  }
  std::string scheme(url, schemeEnd);
  const char *hostBegin = schemeEnd + 3;

  // Path starts at the first '/' after "://" (never inside the scheme).
  const char *path = strchr(hostBegin, '/');
  std::string hostPort = path ? std::string(hostBegin, path) : std::string(hostBegin);
  if (!path) path = "";

  size_t colon = hostPort.rfind(':');
  std::string host = (colon != std::string::npos) ? hostPort.substr(0, colon)
                                                  : hostPort;
  std::string port = (colon != std::string::npos) ? hostPort.substr(colon + 1)
                                                  : std::string();

  bool changed = false;
  if (forceHttp && scheme != "http") {
    scheme = "http";
    changed = true;
  }

  if (!map.from.empty() && map.from == host) {
    size_t toColon = map.to.rfind(':');
    if (toColon != std::string::npos) {
      host = map.to.substr(0, toColon);
      port = map.to.substr(toColon + 1);
    } else {
      host = map.to;
      port.clear(); // no port => use the default (80/443)
    }
    changed = true;
    XYLOG("net: domain %s -> %s%s%s", map.from.c_str(), host.c_str(),
          port.empty() ? "" : ":", port.c_str());
  }

  snprintf(dst, n, "%s://%s%s%s%s", scheme.c_str(), host.c_str(),
           port.empty() ? "" : ":", port.c_str(), path);
  return changed;
}

} // namespace xyl::net
