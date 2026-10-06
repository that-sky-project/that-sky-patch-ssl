#include "net/net.hpp"
#include "net/uri.hpp"
#include "hook/hook.hpp"
#include "sig/signatures.hpp"
#include "util/counter.hpp"
#include "util/log.hpp"

namespace {

using xyl::net::NetUri;

// Resolved redirect per channel (ws inherits account for any empty field).
xyl::config::DomainMap gHttpMap, gWsMap;
xyl::util::Counter gHttpHits, gWsHits;

// Trampolines to the originals, captured at install time.
using RequestFn   = void *(*)(void *thisptr, void *ret, NetUri *uri, void *args);
using WsConnectFn = char  (*)(void *thisptr, NetUri *uri, const char *sub, void *args);
RequestFn   gOrigRequest   = nullptr;
WsConnectFn gOrigWsConnect = nullptr;

// HttpClient::Request(this /*rcx*/, ret /*rdx*/, Net::Uri* /*r8*/, args /*r9*/).
void *hookRequest(void *thisptr, void *ret, NetUri *uri, void *args) {
  xyl::net::applyRedirect(uri, gHttpMap);
  gHttpHits.hit();
  xyl::net::logUri("http:", uri);
  return gOrigRequest(thisptr, ret, uri, args);
}

// WebSocket::Connect(this /*rcx*/, Net::Uri* /*rdx*/, subprotocol /*r8*/, args /*r9*/).
char hookWsConnect(void *thisptr, NetUri *uri, const char *sub, void *args) {
  xyl::net::applyRedirect(uri, gWsMap);
  gWsHits.hit();
  xyl::net::logUri("ws:  ", uri);
  return gOrigWsConnect(thisptr, uri, sub, args);
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
                         "WebSocket::Connect");
  gOrigRequest   = httpHook.orig<RequestFn>();
  gOrigWsConnect = wsHook.orig<WsConnectFn>();

  const bool ok = httpHook.ok() && wsHook.ok();
  XYLOG("net: hooks %s", ok ? "enabled" : "INCOMPLETE");
  return ok;
}

} // namespace xyl::net
