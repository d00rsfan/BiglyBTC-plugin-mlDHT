// SPDX-License-Identifier: MPL-2.0
// Port of Node.initKey/persistKey and Key formatting at
// f9290cbd63805229aef418639b781696c5fa1573 (Mozilla Public License 2.0).
#include "identity.hpp"
#include <utility>
namespace biglybtc::plugins::mldht {
namespace {
bool utf8(std::string_view text) {
  for (std::size_t i = 0; i < text.size();) {
    const auto lead = static_cast<unsigned char>(text[i++]);
    if (lead < 128) { continue; }
    const unsigned length = lead >= 0xc2 && lead <= 0xdf ? 1
        : lead >= 0xe0 && lead <= 0xef ? 2 : lead >= 0xf0 && lead <= 0xf4 ? 3 : 0;
    if (!length || text.size() - i < length) { return false; }
    const auto second = static_cast<unsigned char>(text[i]);
    if ((lead == 0xe0 && second < 0xa0) || (lead == 0xed && second >= 0xa0)
        || (lead == 0xf0 && second < 0x90) || (lead == 0xf4 && second >= 0x90)) { return false; }
    for (unsigned j = 0; j < length; ++j) {
      const auto byte = static_cast<unsigned char>(text[i++]);
      if (byte < 0x80 || byte > 0xbf) { return false; }
    }
  }
  return true;
}
int nibble(char value) {
  if (value >= '0' && value <= '9') { return value - '0'; }
  if (value >= 'a' && value <= 'f') { return value - 'a' + 10; }
  if (value >= 'A' && value <= 'F') { return value - 'A' + 10; }
  return -1;
}
}
std::expected<OriginalIdentity, IdentityError> readOriginalIdentity(std::string bytes) {
  if (bytes.size() > 1024 * 1024) { return std::unexpected(IdentityError::TooLarge); }
  // Files.readAllLines decodes the entire UTF-8 file before choosing a line.
  if (!utf8(bytes)) { return std::unexpected(IdentityError::InvalidUtf8); }
  std::string_view remaining(bytes);
  while (!remaining.empty()) {
    const auto end = remaining.find_first_of("\r\n");
    auto line = remaining.substr(0, end);
    while (!line.empty() && static_cast<unsigned char>(line.front()) <= 32) { line.remove_prefix(1); }
    while (!line.empty() && static_cast<unsigned char>(line.back()) <= 32) { line.remove_suffix(1); }
    std::size_t run = 0;
    bool candidate = false;
    for (const char c : line) {
      run = nibble(c) >= 0 ? run + 1 : 0;
      if (run >= 40) { candidate = true; break; }
    }
    if (candidate) {
      // Pattern.asPredicate uses find(), then Key(String) requires length40.
      // An earlier overlong candidate prevents considering later valid lines.
      if (line.size() != 40) { return std::unexpected(IdentityError::InvalidCandidate); }
      IdentityKey key{};
      for (std::size_t i = 0; i < key.size(); ++i) {
        key[i] = static_cast<std::uint8_t>((nibble(line[i * 2]) << 4) | nibble(line[i * 2 + 1]));
      }
      return OriginalIdentity{key, std::move(bytes)};
    }
    if (end == std::string_view::npos) { break; }
    const auto newline = remaining[end];
    remaining.remove_prefix(end + 1);
    if (newline == '\r' && remaining.starts_with('\n')) { remaining.remove_prefix(1); }
  }
  return std::unexpected(IdentityError::MissingKey);
}
std::string identityHex(const IdentityKey& key) {
  constexpr std::string_view digits = "0123456789ABCDEF";
  std::string result;
  result.reserve(40);
  for (auto byte : key) { result += digits[byte >> 4]; result += digits[byte & 15]; }
  return result;
}
std::string encodeNewIdentity(const IdentityKey& key) { return identityHex(key) + '\n'; }
}
