#include "net/net.hpp"
#include "net/uri.hpp"
#include "hook/hook.hpp"
#include "sig/signatures.hpp"
#include "util/counter.hpp"
#include "util/log.hpp"
#include <cstdio>
#include <cstring>
#include <string>

namespace {

// Resolved redirect per channel (ws inherits account for any empty field).
xyl::config::DomainMap gHttpMap, gWsMap;
xyl::util::Counter gHttpHits, gWsHits;

// Trampolines to the originals, captured at install time.
using RequestFn   = void *(*)(void *thisptr, void *ret, xyl::net::NetUri *uri, void *args);
using WsConnectFn = char (*)(void *thisptr, const char *url, const char *host, void *args);
RequestFn   gOrigRequest   = nullptr;
WsConnectFn gOrigWsConnect = nullptr;

// HttpClient::Request(this /*rcx*/, ret /*rdx*/, Net::Uri* /*r8*/, args /*r9*/).
void *hookRequest(void *thisptr, void *ret, xyl::net::NetUri *uri, void *args) {
  xyl::net::applyRedirect(uri, gHttpMap);
  gHttpHits.hit();
  xyl::net::logUri("http:", uri);
  return gOrigRequest(thisptr, ret, uri, args);
}

// AccountServerWebSocket string Connect.
// char Connect(this /*rcx*/, const char* url /*rdx*/, const char* host /*r8*/,
//              args /*r9*/).
// Account WS only (sole caller builds https://host/account/ws); MQTT uses the
// shared NetUri Connect and is intentionally left alone.
char hookWsConnect(void *thisptr, const char *url, const char *host, void *args) {
  char rewritten[256];
  std::snprintf(rewritten, sizeof(rewritten), "%s", url ? url : "");
  const bool changed = xyl::net::applyRedirectUrl(url, rewritten, sizeof(rewritten), gWsMap);

  // Keep the separate host argument consistent with the rewritten URL host
  // (hostname only, no port) — Connect copies it onto the session object.
  char hostBuf[128];
  std::snprintf(hostBuf, sizeof(hostBuf), "%s", host ? host : "");
  if (changed) {
    const char *scheme = std::strstr(rewritten, "://");
    if (scheme) {
      scheme += 3;
      const char *end = std::strchr(scheme, '/');
      std::string hp = end ? std::string(scheme, end) : std::string(scheme);
      size_t colon = hp.rfind(':');
      std::snprintf(hostBuf, sizeof(hostBuf), "%.*s",
                    (int)(colon == std::string::npos ? hp.size() : colon), hp.c_str());
    }
  }

  gWsHits.hit();
  XYLOG("ws:   %s", rewritten);
  return gOrigWsConnect(thisptr, rewritten, hostBuf, args);
}

} // namespace

namespace xyl::net {

unsigned long long hits() { return gHttpHits.value() + gWsHits.value(); }

bool init(HMODULE self) {
  const auto &cs = xyl::config::get().customServer;
  gHttpMap = cs.account;
  gWsMap   = cs.wsResolved();
  XYLOG("net: redirects (force_http=%d) account=%s->%s ws=%s->%s", cs.forceHttp,
        gHttpMap.from.c_str(), gHttpMap.to.c_str(), gWsMap.from.c_str(), gWsMap.to.c_str());

  xyl::hook::Hook httpHook(self, sig::kHttpClientRequest, (void *)&hookRequest,
                           "HttpClient::Request");
  xyl::hook::Hook wsHook(self, sig::kWebSocketConnect, (void *)&hookWsConnect,
                         "AccountServerWebSocket::Connect");
  gOrigRequest   = httpHook.orig<RequestFn>();
  gOrigWsConnect = wsHook.orig<WsConnectFn>();

  const bool ok = httpHook.ok() && wsHook.ok();
  XYLOG("net: hooks %s", ok ? "enabled" : "INCOMPLETE");
  return ok;
}

} // namespace xyl::net
