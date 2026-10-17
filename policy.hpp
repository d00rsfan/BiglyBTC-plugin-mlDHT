// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
namespace biglybtc::plugins::mldht {
inline constexpr std::string_view RuntimeId = "mldht";
// Host-value boundary from MlDHTPlugin's bind predicate. No sockets or DNS.
struct Address {
  std::uint32_t family; // 4/6; IPv4 first four bytes, IPv6 all sixteen
  std::array<std::uint8_t, 16> bytes{};
};
bool bindAllowed(Address candidate, std::span<const Address> configured);
// Original per-download plugin option overrides only active non-metadata states.
std::int32_t effectiveDownloadState(std::int32_t originalState, bool metadata,
    bool hasCoreManager, std::optional<std::int64_t> enableAnnounce);
}
