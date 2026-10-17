// SPDX-License-Identifier: MPL-2.0
#pragma once
#include <array>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
namespace biglybtc::plugins::mldht {
using IdentityKey = std::array<std::uint8_t, 20>;
enum class IdentityError { TooLarge, InvalidUtf8, MissingKey, InvalidCandidate };
struct OriginalIdentity {
  IdentityKey key;
  std::string originalBytes;
};
// Read-only import. Java's fallback can replace malformed identity files; this
// reader reports that condition without generating a new key or changing bytes.
std::expected<OriginalIdentity, IdentityError> readOriginalIdentity(std::string bytes);
std::string identityHex(const IdentityKey& key);
// Exact Linux Node.persistKey representation for a caller-approved new key.
std::string encodeNewIdentity(const IdentityKey& key);
}
