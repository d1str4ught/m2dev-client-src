#pragma once

// Control-plane packet structs used by all CNetworkStream subclasses.
// Extracted from UserInterface/Packet.h so the base class (EterLib)
// can handle key exchange, ping/pong, and phase transitions without
// depending on UserInterface.

#include <cstdint>

// Control-plane header constants (subset of CG::/GC:: namespaces).
// The full CG/GC namespaces live in Packet.h which includes this file,
// so all existing code sees these via Packet.h as before.
namespace CG {
constexpr uint16_t PONG = 0x0006;
constexpr uint16_t KEY_RESPONSE = 0x000A;
} // namespace CG
namespace GC {
constexpr uint16_t PING = 0x0007;
constexpr uint16_t PHASE = 0x0008;
constexpr uint16_t KEY_CHALLENGE = 0x000B;
constexpr uint16_t KEY_COMPLETE = 0x000C;
} // namespace GC

#pragma pack(push, 1)

struct TPacketGCPhase {
  uint16_t header;
  uint16_t length;
  uint8_t phase;
};

struct TPacketGCPing {
  uint16_t header;
  uint16_t length;
  uint32_t server_time;
};

struct TPacketCGPong {
  uint16_t header;
  uint16_t length;
};

struct TPacketGCKeyChallenge {
  uint16_t header;
  uint16_t length;
  uint8_t server_pk[32];
  uint8_t challenge[32];
  uint32_t server_time;
};

struct TPacketCGKeyResponse {
  uint16_t header;
  uint16_t length;
  uint8_t client_pk[32];
  uint8_t challenge_response[32];
};

struct TPacketGCKeyComplete {
  uint16_t header;
  uint16_t length;
  uint8_t encrypted_token[32 + 16];
  uint8_t nonce[24];
};

#pragma pack(pop)
