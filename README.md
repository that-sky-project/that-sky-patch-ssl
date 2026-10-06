# SSL Patch

> Network patches for *Sky: Children of the Light* (PC) — skip TLS certificate verification, or redirect the game's account traffic to your own server.

**English** · [简体中文](README.zh-CN.md)

`sslpatch` is a standalone mod that runs on the [HTML-Sky](https://github.com/HTMonkeyG/HTML-Sky) mod loader.

---

## Dependency

| Dependency | Notes |
|------------|-------|
| [HTML-Sky](https://github.com/HTMonkeyG/HTML-Sky) | Mod loader / SDK. Provides the `htmodloader` headers and import library; vendored under `vendor/htmodloader`. Required for both building and running. |

---

## Modes

`type` in `sslpatch.json` selects one of:

| `type` | Description |
|--------|-------------|
| `none` | Do nothing (default). |
| `patch_ssl` | Bypass TLS certificate-chain verification. |
| `custom_server` | Redirect the account server (HTTP + WebSocket) to a custom one. |

### `patch_ssl`

Hooks `ssl_verify_cert_chain` (BoringSSL) so it always reports the chain as verified — useful for inspecting the game's traffic with a proxy / MITM setup.

### `custom_server`

Points the game's **account** traffic at your own server:

- `force_http` — downgrade the protocol to `http` (for WebSocket this means plain `ws` instead of `wss`).
- `account` — the HTTP account-server redirect.
- `ws` — the WebSocket redirect. **An empty field inherits the matching `account` field**, so leaving the whole `ws` block empty mirrors `account`.

---

## Configuration

`sslpatch.json` is generated next to the DLL on first run:

```json
{
	"type":	"none",
	"custom_server":	{
		"force_http":	false,
		"account":	{ "from": "", "to": "" },
		"ws":		{ "from": "", "to": "" }
	}
}
```

- `from` is the game's own domain (exact match); `to` is your server, written as `host` or `host:port`.
- To discover the real domains, set `type` to `custom_server` and read the `http:` / `ws:` lines the mod logs to the in-game console and the debugger, then paste them into `from`.

Example — move the account server to a local one:

```json
{
	"type":	"custom_server",
	"custom_server":	{
		"force_http":	true,
		"account":	{ "from": "<account host>", "to": "127.0.0.1:25565" },
		"ws":		{ "from": "<websocket host>", "to": "127.0.0.1:25565" }
	}
}
```

---

## Structure

```
sslpatch/
├── manifest.json
├── Makefile
├── vendor/                  # htmodloader (SDK), cJSON, imgui
└── src/
    ├── entry.cpp            # mod entry (HTModOnInit) — dispatch by `type`
    ├── config/              # sslpatch.json model + cJSON I/O
    ├── ssl/patch.cpp        # patch_ssl — certificate bypass
    ├── net/                 # custom_server — HTTP + WebSocket hooks
    │   ├── uri.hpp          #   Net::Uri layout + redirect + logging
    │   └── net.cpp
    ├── hook/hook.hpp        # hook::Hook — signature → detour helper
    ├── sig/signatures.hpp   # HTSigScan patterns
    └── util/                # log, fs, counter
```

---

## Build

Requires MinGW-w64 (`gcc` / `g++`) and `make`.

```bash
make
```

Output: `dist/ssl-patch.dll`. `make clean` removes the build output.

---

## Install

1. Build `dist/ssl-patch.dll`;
2. Copy `ssl-patch.dll` and `manifest.json` into the game's mods folder;
3. Edit `sslpatch.json` (generated on first run) to pick a `type`;
4. Launch the game.

---
