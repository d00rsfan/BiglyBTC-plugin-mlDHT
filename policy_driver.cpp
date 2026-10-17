#include "policy.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
namespace policy = biglybtc::plugins::mldht;
policy::Address address(const std::string& hex) {
  if (hex.size() != 8 && hex.size() != 32) { throw std::runtime_error("Invalid address bytes"); }
  policy::Address result{hex.size() == 8 ? 4U : 6U, {}};
  for (std::size_t i = 0; i < hex.size() / 2; ++i) {
    result.bytes[i] = static_cast<std::uint8_t>(std::stoul(hex.substr(i * 2, 2), nullptr, 16));
  }
  return result;
}
int main(int argc, char** argv) {
  if (argc != 2 || !std::string_view(argv[1]).starts_with("/tmp/biglybtc-")) { return 2; }
  std::ifstream stream(argv[1]);
  if (!stream.good()) { return 3; }
  std::string line;
  while (std::getline(stream, line)) {
    std::istringstream input(line);
    char kind = 0; input >> kind;
    if (kind == 'B') {
      std::string candidate, hex; input >> candidate;
      std::vector<policy::Address> binds;
      while (input >> hex) { binds.push_back(address(hex)); }
      std::cout << policy::bindAllowed(address(candidate), binds) << '\n';
    } else if (kind == 'S') {
      int state = 0, metadata = 0, core = 0; std::string option;
      input >> state >> metadata >> core >> option;
      std::optional<std::int64_t> enabled;
      if (option != "-") { enabled = std::stoll(option); }
      std::cout << policy::effectiveDownloadState(state, metadata, core, enabled) << '\n';
    } else { return 4; }
  }
}
