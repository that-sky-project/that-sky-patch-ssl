#include "ssl/patch.hpp"
#include "hook/hook.hpp"
#include "sig/signatures.hpp"
#include "util/counter.hpp"

namespace {

xyl::util::Counter gHits;

int hookVerify(void *, void *) {
  gHits.hit();
  return 1;
}

} // namespace

namespace xyl::ssl {

unsigned long long hits() { return gHits.value(); }

bool init(HMODULE self) {
  xyl::hook::Hook hook(self, sig::kSslVerifyCertChain, (void *)&hookVerify,
                       "ssl_verify_cert_chain");
  return hook.ok();
}

} // namespace xyl::ssl
