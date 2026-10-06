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

// WebSocket::Connect(this /*rcx*/, Net::Uri* uri /*rdx*/, const char* subprotocol /*r8*/,
//                    HttpClientRequestArgs* args /*r9*/) -> char.
static const HTAsmSig kWebSocketConnect = {
  "55 41 57 41 56 41 54 56 57 53 48 81 EC 30 01 00 00 48 8D AC 24 80 00 00 00 "
  "4C 89 CF 4D 89 C6 48 89 CE 80 B9 18 51 00 00 01",
  HTSigScanType_Direct,
  0,
};

} // namespace xyl::sig
