# BiglyBTC-plugin-mlDHT

Initial C++23 **host policy and identity import** components for original runtime ID `mldht`, from
MlDHTPlugin at `3b12515c5541ed75cfb4f795b672d4e786df9f45` (GPL-2.0-or-later
file headers). This is not yet a loadable network plugin or Mainline DHT engine.

Preserves the original any-local binding behavior across IPv4/IPv6, exact address
binding, Java IPv4-mapped address normalization, and per-torrent announce override
including Java signed32 truncation. Caller-supplied immutable host values keep
policy independent from sockets, DNS and application initialization.

The original method bodies run in a fenced test harness with controlled host
services;4860 observations are frozen under migration/fixtures/mldht-policies in
the migration workspace, or `fixtures/` in the independent source export.

```sh
cmake -S . -B /tmp/biglybtc-mldht-build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/biglybtc-mldht-build -j2
ctest --test-dir /tmp/biglybtc-mldht-build --output-on-failure
```

The native
DHT engine/wire/cache/identity format, full scheduling/tracker/config/UI and
privacy/bind policy integration still need implementation and interoperability.

The read-only `baseID.config` importer additionally follows Node/Key at audited
engine fork `f9290cbd63805229aef418639b781696c5fa1573`. It retains original bytes,
Java ASCII trim/line selection, UTF-8 decoding rules and uppercase writer encoding.
Malformed identities that Java regenerates are reported without overwriting the
original. This component has no identity generation or disk writer. The original
wrapper/POM engine pairing remains unqualified; no operational DHT is enabled.

Wrapper policy files are GPL-2.0-or-later (LICENSE). Identity files are MPL-2.0
(LICENSES/MPL-2.0.txt), preserving the original engine source grant. Identity
fixtures live under `migration/fixtures/mldht-identity` in the workspace or
`identity-fixtures/` in the independent export.
