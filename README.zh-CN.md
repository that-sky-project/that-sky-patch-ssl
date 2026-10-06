# SSL Patch

> 面向《Sky: 光·遇》(PC) 的网络补丁 —— 跳过 TLS 证书校验，或把游戏的账号流量重定向到自建服务器。

[English](README.md) · **简体中文**

`sslpatch` 是一个独立 mod，依赖 [HTML-Sky](https://github.com/HTMonkeyG/HTML-Sky) 模组加载器运行。

---

## 依赖

| 依赖 | 说明 |
|------|------|
| [HTML-Sky](https://github.com/HTMonkeyG/HTML-Sky) | 模组加载器 / SDK。提供 `htmodloader` 头文件与导入库，已内置在 `vendor/htmodloader`。构建与运行都需要它。 |

---

## 模式

由 `sslpatch.json` 里的 `type` 选择其一：

| `type` | 说明 |
|--------|------|
| `none` | 什么都不做（默认）。 |
| `patch_ssl` | 跳过 TLS 证书链校验。 |
| `custom_server` | 把账号服务器（HTTP + WebSocket）重定向到自建服务器。 |

### `patch_ssl`

Hook `ssl_verify_cert_chain`（BoringSSL），让它恒返回「证书链已验证」—— 便于用代理 / MITM 抓包分析游戏流量。

### `custom_server`

把游戏的**账号**流量指向你自己的服务器：

- `force_http` —— 把协议降级为 `http`（对 WebSocket 即明文 `ws` 而非 `wss`）。
- `account` —— 账号 HTTP 的重定向。
- `ws` —— WebSocket 的重定向。**字段留空则继承 `account` 的对应字段**，所以整块 `ws` 留空即完全等同 `account`。

---

## 配置

首次运行会在 DLL 同目录生成 `sslpatch.json`：

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

- `from` 是游戏自身的域名（精确匹配）；`to` 是你的服务器，写 `host` 或 `host:port`。
- 想知道真实域名：把 `type` 设为 `custom_server`，看 mod 输出到游戏内控制台和调试器的 `http:` / `ws:` 日志，再填进 `from`。

示例 —— 把账号服务器指到本地：

```json
{
	"type":	"custom_server",
	"custom_server":	{
		"force_http":	true,
		"account":	{ "from": "<账号域名>", "to": "127.0.0.1:25565" },
		"ws":		{ "from": "<WebSocket域名>", "to": "127.0.0.1:25565" }
	}
}
```

---

## 目录结构

```
sslpatch/
├── manifest.json
├── Makefile
├── vendor/                  # htmodloader (SDK)、cJSON、imgui
└── src/
    ├── entry.cpp            # mod 入口（HTModOnInit）—— 按 `type` 分发
    ├── config/              # sslpatch.json 模型 + cJSON 读写
    ├── ssl/patch.cpp        # patch_ssl —— 证书绕过
    ├── net/                 # custom_server —— HTTP + WebSocket 钩子
    │   ├── uri.hpp          #   Net::Uri 布局 + 重定向 + 日志
    │   └── net.cpp
    ├── hook/hook.hpp        # hook::Hook —— 特征 → detour 的复用组件
    ├── sig/signatures.hpp   # HTSigScan 特征串
    └── util/                # log、fs、counter
```

---

## 构建

前置条件：MinGW-w64（`gcc` / `g++`）与 `make`。

```bash
make
```

产物：`dist/ssl-patch.dll`。`make clean` 可清除构建产物。

---

## 安装

1. 构建得到 `dist/ssl-patch.dll`；
2. 把 `ssl-patch.dll` 与 `manifest.json` 放进游戏的 mod 目录；
3. 编辑 `sslpatch.json`（首次运行生成）选择 `type`；
4. 启动游戏。

---
