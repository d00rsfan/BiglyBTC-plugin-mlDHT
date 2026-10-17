#include "identity.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <string_view>
int main(int argc, char** argv) {
  namespace policy = biglybtc::plugins::mldht;
  if (argc != 2 && argc != 3) { return 2; }
  const bool encode = argc == 3 && std::string_view(argv[1]) == "--encode";
  if (argc == 3 && !encode) { return 2; }
  const char* path = argv[encode ? 2 : 1];
  if (!std::string_view(path).starts_with("/tmp/biglybtc-")) { return 2; }
  std::ifstream stream(path, std::ios::binary);
  if (!stream.good()) { return 3; }
  const std::string bytes((std::istreambuf_iterator<char>(stream)), {});
  auto result = policy::readOriginalIdentity(bytes);
  if (!result) { std::cout << "REJECT " << static_cast<int>(result.error()) << '\n'; return 0; }
  if (result->originalBytes != bytes) { return 4; }
  if (encode) { std::cout << policy::encodeNewIdentity(result->key); return 0; }
  std::cout << "RESTORED " << policy::identityHex(result->key) << '\n';
}
