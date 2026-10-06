#include <windows.h>
#include <string>
#include "includes/htmod.h"
#include "config/config.hpp"
#include "ssl/patch.hpp"
#include "net/net.hpp"
#include "util/fs.hpp"
#include "util/log.hpp"

static HMODULE gSelf = nullptr;

extern "C" __declspec(dllexport) HTStatus HTMLAPI HTModOnInit(void *) {
  const std::string cfg = xyl::util::moduleDir(gSelf) + "\\sslpatch.json";
  xyl::config::load(cfg.c_str()); 

  bool ok = true;
  switch (xyl::config::get().type) {
    case xyl::config::Mode::None:
      XYLOG("type=none; no hooks installed");
      break;
    case xyl::config::Mode::PatchSsl:
      ok = xyl::ssl::init(gSelf);
      break;
    case xyl::config::Mode::CustomServer:
      ok = xyl::net::init(gSelf);
      break;
  }
  if (!ok) { XYLOG("hook init failed"); return HT_FAIL; }
  return HT_SUCCESS;
}

extern "C" __declspec(dllexport) HTStatus HTMLAPI HTModOnEnable(void *) {
  // XYLOG("enabled (ssl hits=%llu, net hits=%llu)", xyl::ssl::hits(), xyl::net::hits());
  return HT_SUCCESS;
}

extern "C" __declspec(dllexport) void HTMLAPI HTModRenderGui(float, void *) {}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH) gSelf = hModule;
  return TRUE;
}
