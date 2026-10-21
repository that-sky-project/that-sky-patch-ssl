#pragma once
#include "includes/htmod.h"

namespace xyl::sig {

// ssl_verify_cert_chain(ssl /*rcx*/, store /*rdx*/) -> int (1 == chain trusted).
// Matched at its call site inside the TLS handshake:
//   mov rdx, r15 ; mov rcx, rbx ; call ssl_verify_cert_chain ; mov edi, eax ;
//   cmp dword [rbx+0x558], ...
static const HTAsmSig kSslVerifyCertChain = {
  "49 8B D7 48 8B CB E8 ?? ?? ?? ?? 8B F8 39 B3 58 05 00 00",
  HTSigScanType_E8,
  0x06,
};

// HttpClient::Request(this /*rcx*/, HttpRequestHandle* ret /*rdx, sret*/,
//                     Net::Uri* uri /*r8*/, HttpClientRequestArgs* args /*r9*/).
// Matched at the call site inside the request dispatcher:
//   lea r9, [rbp+...] ; call HttpClient::Request ; mov eax,[rbp+...] ; mov [rsi+368], eax
static const HTAsmSig kHttpClientRequest = {
  "4C 8D 8D ?? ?? ?? ?? E8 ?? ?? ?? ?? 8B 85 ?? ?? ?? ?? 89 86",
  HTSigScanType_E8,
  0x07,
};

// AccountServerWebSocket string Connect (only caller: account WS @ /account/ws).
// char Connect(this /*rcx*/, const char* url /*rdx*/, const char* host /*r8*/,
//              HttpClientRequestArgs* args /*r9*/) -> char.
// Parses the URL into a stack NetUri then forwards to the shared
// WebSocket::Connect. The shared Connect (sub_14010A620) is also used by the
// MQTT channel (path forced to "mqtt"), so hooking it would catch MQTT too.
static const HTAsmSig kWebSocketConnect = {
  "55 41 57 41 56 56 57 53 48 81 EC B8 01 00 00 48 8D AC 24 80 00 00 00 "
  "4C 89 CE 4C 89 C7 48 89 D3 49 89 CE C5 F8 57 C0",
  HTSigScanType_Direct,
  0,
};

} // namespace xyl::sig
