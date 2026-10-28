// SPDX-License-Identifier: MPL-2.0
#include "identity.hpp"
#include <algorithm>
#include <charconv>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
namespace id = biglybtc::plugins::mldht;
namespace {
void require(bool value, const char* message) { if (!value) { throw std::runtime_error(message); } }
id::IdentityKey key(std::string_view text) {
  auto value = id::readOriginalIdentity(std::string(text));
  require(value.has_value(), "key"); return value->key;
}
std::uint32_t index(std::string_view text) {
  require(text.size() == 8, "index width"); std::uint32_t result = 0;
  const auto [end, ec] = std::from_chars(text.data(), text.data() + text.size(), result, 16);
  require(ec == std::errc{} && end == text.data() + text.size(), "index"); return result;
}
std::vector<std::string_view> split(std::string_view text, char separator) {
  std::vector<std::string_view> out;
  do { const auto end = text.find(separator); out.push_back(text.substr(0, end));
    if (end == text.npos) { break; } text.remove_prefix(end + 1);
  } while (true); return out;
}
std::string hex(std::string_view bytes) {
  constexpr char digits[] = "0123456789abcdef"; std::string result;
  for (unsigned char byte : bytes) { result += digits[byte >> 4]; result += digits[byte & 15]; } return result;
}
void replay(const char* path) {
  std::ifstream file(path); require(file.good(), "tape"); std::string line; std::size_t count = 0;
  while (std::getline(file, line)) {
    require(++count <= 4096 && line.size() <= 16384, "tape bounds"); const auto f = split(line, '\t');
    require(f.size() == 4 && !f[1].empty() && f[1].size() <= 64, "columns"); const auto root = key(f[2]);
    if (f[0] == "D") {
      const auto derived = id::deriveOriginalIdentity(root, index(f[3]));
      std::cout << "D\t" << f[1] << '\t' << id::identityHex(derived) << '\t' << hex(id::encodeNewIdentity(root)) << '\n';
    } else {
      require(f[0] == "S", "opcode"); std::vector<id::IdentityKey> active; std::cout << "S\t" << f[1];
      for (const auto op : split(f[3], ',')) {
        if (op == "A") {
          const auto selected = id::selectOriginalIdentity(root, active); require(selected.has_value(), "selection");
          active.push_back(selected->key); std::cout << "\tA:" << selected->index << ':' << id::identityHex(selected->key);
        } else {
          require(op.starts_with('R'), "release opcode"); const auto released = index(op.substr(1));
          const auto value = id::deriveOriginalIdentity(root, released); const auto it = std::find(active.begin(), active.end(), value);
          const bool found = it != active.end(); if (found) { active.erase(it); }
          std::cout << "\tR:" << released << ':' << (found ? 1 : 0);
        }
      }
      std::cout << '\n';
    }
  }
  require(file.eof(), "tape read");
}
void units() {
  const auto root = key("00112233445566778899AABBCCDDEEFF01234567"); const auto before = root;
  require(id::deriveOriginalIdentity(root, 0) == root, "zero index/root");
  auto expected = root; expected[0] ^= 0x80; require(id::deriveOriginalIdentity(root, 1) == expected, "index1 first bit");
  expected = root; expected[3] ^= 1; require(id::deriveOriginalIdentity(root, 0x80000000U) == expected, "signed Java minimum");
  expected = root; for (unsigned i = 0; i < 4; ++i) { expected[i] ^= 0xff; }
  require(id::deriveOriginalIdentity(root, 0xffffffffU) == expected && root == before, "negative1/full reverse/immutable root");
  for (unsigned bit = 0; bit < 32; ++bit) {
    expected = root; expected[bit / 8] ^= static_cast<std::uint8_t>(0x80U >> (bit % 8));
    require(id::deriveOriginalIdentity(root, std::uint32_t{1} << bit) == expected, "every index basis bit");
  }
  auto imported = id::readOriginalIdentity(id::encodeNewIdentity(root));
  require(imported && imported->key == root && id::encodeNewIdentity(root) == "00112233445566778899AABBCCDDEEFF01234567\n", "existing exact Linux writer reused");
  std::vector<id::IdentityKey> active;
  for (std::uint32_t i = 0; i < id::MaximumActiveIdentities; ++i) { active.push_back(id::deriveOriginalIdentity(root, i)); }
  auto selected = id::selectOriginalIdentity(root, active);
  require(selected && selected->index == id::MaximumActiveIdentities, "full finite prefix first free");
  active.erase(active.begin() + 123); selected = id::selectOriginalIdentity(root, active);
  require(selected && selected->index == 123, "released interior slot reused");
  active.erase(active.begin()); selected = id::selectOriginalIdentity(root, active);
  require(selected && selected->index == 0, "released root index reused");
  auto foreign = root; foreign[19] ^= 1; active.assign(2, foreign);
  selected = id::selectOriginalIdentity(root, active); require(selected && selected->index == 0, "foreign tail cannot occupy derived slot");
  active.assign(2, id::deriveOriginalIdentity(root, 0xffffffffU));
  selected = id::selectOriginalIdentity(root, active); require(selected && selected->index == 0, "distant/signed index and duplicate membership");
  active.assign(id::MaximumActiveIdentities + 1, root);
  require(!id::selectOriginalIdentity(root, active), "explicit finite active bound");
}
}
int main(int argc, char** argv) {
  try {
    if (argc == 3 && std::string_view(argv[1]) == "--replay") { replay(argv[2]); return 0; }
    require(argc == 1, "arguments"); units(); std::cout << "PASS original root derivation/selection/serialization boundaries\n"; return 0;
  } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
