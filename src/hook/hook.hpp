#pragma once
#include "includes/htmod.h"
#include "util/log.hpp"

namespace xyl::hook {

class Hook {
 public:
  Hook(HMODULE owner, const HTAsmSig &sig, void *detour, const char *name) {
    target_ = HTSigScan(&sig);
    if (!target_) { XYLOG("hook: could not resolve %s", name); return; }
    if (HTAsmHookCreateRaw(owner, target_, detour, &origin_) != HT_SUCCESS) {
      XYLOG("hook: create failed (%s)", name); target_ = nullptr; return;
    }
    if (HTAsmHookEnable(owner, target_) != HT_SUCCESS) {
      XYLOG("hook: enable failed (%s)", name); target_ = nullptr; return;
    }
    ok_ = true;
    XYLOG("hook: %s @ %p enabled", name, target_);
  }

  Hook(const Hook &) = delete;
  Hook &operator=(const Hook &) = delete;

  bool  ok() const { return ok_; }
  void *target() const { return target_; } // resolved address (the hook site)

  template <class Fn> Fn orig() const { return reinterpret_cast<Fn>(origin_); }

 private:
  void *target_ = nullptr;
  void *origin_ = nullptr;
  bool  ok_ = false;
};

} // namespace xyl::hook
