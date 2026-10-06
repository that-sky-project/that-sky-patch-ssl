#include "config/config.hpp"
#include "util/fs.hpp"
#include "util/log.hpp"
#include "cJSON/cJSON.h"
#include <cstring>

namespace {

xyl::config::Settings gSettings;

void addMap(cJSON *parent, const char *key, const xyl::config::DomainMap &m) {
  cJSON *o = cJSON_AddObjectToObject(parent, key);
  cJSON_AddStringToObject(o, "from", m.from.c_str());
  cJSON_AddStringToObject(o, "to", m.to.c_str());
}

// Serialize settings to pretty JSON (used to auto-generate the default file).
std::string toJson(const xyl::config::Settings &s) {
  cJSON *root = cJSON_CreateObject();
  cJSON_AddStringToObject(root, "type", xyl::config::modeName(s.type));
  cJSON *cs = cJSON_AddObjectToObject(root, "custom_server");
  cJSON_AddBoolToObject(cs, "force_http", s.customServer.forceHttp);
  addMap(cs, "account", s.customServer.account);
  addMap(cs, "ws", s.customServer.ws);
  char *txt = cJSON_Print(root);
  std::string out = txt ? txt : "{}";
  cJSON_free(txt);
  cJSON_Delete(root);
  return out;
}

// Read a {from,to} object into `out`; missing fields are left untouched.
void readMap(cJSON *parent, const char *key, xyl::config::DomainMap &out) {
  cJSON *o = cJSON_GetObjectItemCaseSensitive(parent, key);
  if (!o) return;
  cJSON *f = cJSON_GetObjectItemCaseSensitive(o, "from");
  cJSON *t = cJSON_GetObjectItemCaseSensitive(o, "to");
  if (f && cJSON_IsString(f) && f->valuestring) out.from = f->valuestring;
  if (t && cJSON_IsString(t) && t->valuestring) out.to = t->valuestring;
}

} // namespace

namespace xyl::config {

Mode parseMode(const char *s) {
  if (s) {
    if (strcmp(s, "patch_ssl") == 0)     return Mode::PatchSsl;
    if (strcmp(s, "custom_server") == 0) return Mode::CustomServer;
  }
  return Mode::None;
}

const char *modeName(Mode m) {
  switch (m) {
    case Mode::PatchSsl:     return "patch_ssl";
    case Mode::CustomServer: return "custom_server";
    case Mode::None:         break;
  }
  return "none";
}

const Settings &get() { return gSettings; }

bool load(const char *absPath) {
  if (!absPath || !absPath[0]) return false;

  std::string text = xyl::util::readFile(absPath);
  if (text.empty()) {
    // First run (or unreadable): write defaults so the user has a file to edit.
    if (xyl::util::writeFile(absPath, toJson(gSettings))) XYLOG("config: wrote default %s", absPath);
    else                                                  XYLOG("config: could not write %s (using defaults)", absPath);
    return true; // gSettings already holds the defaults
  }

  cJSON *root = cJSON_Parse(text.c_str());
  if (!root) { XYLOG("config: parse error in %s (using defaults)", absPath); return true; }

  cJSON *v = cJSON_GetObjectItemCaseSensitive(root, "type");
  if (v && cJSON_IsString(v)) gSettings.type = parseMode(v->valuestring);

  if (cJSON *cs = cJSON_GetObjectItemCaseSensitive(root, "custom_server")) {
    if ((v = cJSON_GetObjectItemCaseSensitive(cs, "force_http")) && cJSON_IsBool(v))
      gSettings.customServer.forceHttp = cJSON_IsTrue(v);
    readMap(cs, "account", gSettings.customServer.account);
    readMap(cs, "ws", gSettings.customServer.ws);
  }
  cJSON_Delete(root);

  const auto &cs = gSettings.customServer;
  DomainMap ws = cs.wsResolved();
  XYLOG("config: loaded %s (type=%s, account=%s->%s, ws=%s->%s)", absPath, modeName(gSettings.type),
        cs.account.from.c_str(), cs.account.to.c_str(), ws.from.c_str(), ws.to.c_str());
  return true;
}

} // namespace xyl::config
