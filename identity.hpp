// SPDX-License-Identifier: MPL-2.0
#pragma once
#include <array>
#include <cstdint>
#include <cstddef>
#include <expected>
#include <span>
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
// Original Key.getDerivedKey(int): indexBits is the exact unsigned bit pattern
// of the Java int, including negative indices. Root bytes remain unchanged.
IdentityKey deriveOriginalIdentity(const IdentityKey& root, std::uint32_t indexBits) noexcept;
inline constexpr std::size_t MaximumActiveIdentities = 4096;
enum class IdentityDerivationError { TooManyActive };
struct DerivedIdentitySelection {
  IdentityKey key;
  std::uint32_t index;
};
// Pure Node.registerId selection only: caller owns insertion, retirement and
// concurrency. Chooses first unoccupied derived index; released holes are reused.
// Bound4096 is an explicit native limit, not an original maximum/socket claim.
std::expected<DerivedIdentitySelection, IdentityDerivationError> selectOriginalIdentity(
    const IdentityKey& root, std::span<const IdentityKey> active) noexcept;

}
