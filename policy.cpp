// SPDX-License-Identifier: GPL-2.0-or-later
// Port of MlDHTPlugin host policies at 3b12515c5541ed75cfb4f795b672d4e786df9f45.
#include "policy.hpp"
#include <algorithm>
#include <bit>
namespace biglybtc::plugins::mldht {
namespace {
bool valid(const Address& address) { return address.family == 4 || address.family == 6; }
Address normalize(Address address) {
  // InetAddress.getByAddress normalizes IPv4-mapped IPv6 to Inet4Address.
  if (address.family == 6 && std::all_of(address.bytes.begin(), address.bytes.begin() + 10,
      [](auto byte) { return byte == 0; }) && address.bytes[10] == 255 && address.bytes[11] == 255) {
    std::copy_n(address.bytes.begin() + 12, 4, address.bytes.begin());
    std::fill(address.bytes.begin() + 4, address.bytes.end(), 0);
    address.family = 4;
  }
  return address;
}
bool any(const Address& address) {
  return std::all_of(address.bytes.begin(), address.bytes.begin() + (address.family == 4 ? 4 : 16),
      [](auto byte) { return byte == 0; });
}
}
bool bindAllowed(Address candidate, std::span<const Address> configured) {
  if (!valid(candidate)) { return false; }
  candidate = normalize(candidate);
  const auto count = candidate.family == 4 ? 4 : 16;
  for (auto address : configured) {
    if (!valid(address)) { continue; }
    address = normalize(address);
    if (any(candidate)) {
      if (any(address)) { return true; }
    } else if (candidate.family == address.family
        && std::equal(candidate.bytes.begin(), candidate.bytes.begin() + count, address.bytes.begin())) {
      return true;
    }
  }
  return false;
}
std::int32_t effectiveDownloadState(std::int32_t state, bool metadata,
    bool hasCoreManager, std::optional<std::int64_t> enableAnnounce) {
  if (!metadata && (state == 4 || state == 5) && hasCoreManager && enableAnnounce
      && std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(*enableAnnounce)) == 0) {
    return 9; // ST_QUEUED: scrape rather than announce
  }
  return state;
}
}
