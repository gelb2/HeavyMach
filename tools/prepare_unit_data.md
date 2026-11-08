# Runtime unit data exporter

`python3 tools/prepare_unit_data.py` reads the preserved original `Resources/RawData` and `prepare_unit_data_authoring.json`, then writes `Resources/RuntimeData`. `--output PATH` selects another output directory. No original input is changed.

The JSON is a captured authoring baseline from product tree `191728e310158a55df1164ee452b19b48fa97e57`. It is not created by the application. `prepare_unit_data_baseline.cpp` records the one-time capture procedure and deliberately requires the old audited checkout's Core headers and library:

```
c++ -std=c++20 -I OLD_CHECKOUT/Sources/Core tools/prepare_unit_data_baseline.cpp OLD_BUILD/libHeavyMachCore.a -o capture-authoring
./capture-authoring OLD_CHECKOUT/Resources/RawData > captured.json
```

The old factory implementation is not retained in production Core. Reproduction of the capture requires the named historical checkout, not the new Core. Routine package regeneration only uses Python and the committed authoring JSON.

The writer copies original MCH child nodes and map groups/children without reserializing their payloads. It appends new original-format nodes and changes only the enclosing child counts. The original stale size words are preserved because the original reader traverses payload length and child count. Before writing, it verifies every retained node's bytes against the original. All 46 files are emitted; unchanged maps remain byte-identical. The manifest records original input and derived output SHA-256 separately.

Added runtime records: 24 weapons, 20 machines, 78 parts, 304 fixed map placements. The six extra parts relative to the previous 72-part expansion are actual exhaust nodes in the common aircraft part trees. The body replaces the invisible anchor, preserving its sockets. Fourteen formations remain explicit extension events rather than fake static map enemies.

`UnitExtensions.hmx` is a distinct binary contract, not an MCH extension disguised as the same file format. All integers are little-endian, floats are IEEE float32, strings use a uint16 byte length with no terminator. The header is the eight ASCII bytes `HMEX0001`. Each section starts with a uint32 record count:

1. Part presentation: part name; logical size (2 floats); animation byte (0 original frame state, 1 world-clock loop); period float; effect byte (0 none, 1 pulsed exhaust); column count byte; frame count uint32; frames of tile-name string and offset (2 floats); nozzle (2 floats); aft-direction byte; seven floats for base length, vertical velocity reference/amplitude, pulse frequency/amplitude, thickness and alpha.
2. Machine behavior: machine name; uint32 flags (bounds=1, gauge=2, engine bounce=4, private RNG=8, velocity pitch=16, mounted weapon origin=32, rotated bounds=64); four bounds floats; gauge Y; muzzle XY; horizontal/vertical pitch degrees, horizontal/vertical speed reference and easing (five floats).
3. Weapon behavior: weapon name; AI time scale float; burst group delay float; burst group size byte.
4. Stage formation: map name; trigger boss name (empty means final approach); entry/release speed floats; uint32 unit count; units of machine name and offset XY.

The parser rejects bad versions, truncation, oversized counts, non-finite floats, unknown/duplicate referenced IDs, invalid periods/speeds/effect values, missing boss triggers and trailing bytes. Original MCH/MAP parsers are unchanged. `GameData::loadDirectory` reads the same fixed MCH filenames and MAP files, then applies this explicit extension. Original-data tools may leave it optional; product callers pass `requireExtensions=true`, which rejects a missing contract before a session can run. Android applies the same bytes through `GameData::applyExtensions` after its asset-buffer parsing.

The common `PartsInfo` presentation chooses real frame files. The common part traversal renders aircraft bodies/exhaust and ground parts; it no longer dispatches by aircraft or GAA names. `GameData::presentationImages` enumerates unique actual image keys for platform preloading. Original parts/machines do not receive extension properties. No new custom crop or resize occurs in Core.

These contracts preserve additional behavior that the original 1.13 file format cannot express. They are documented restoration-engine extensions, not claims that the original game supported these fields.
