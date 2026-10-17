# BiglyBTC-plugin-mlDHT

Initial C++23 **host policy** component for original runtime ID `mldht`, from
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
