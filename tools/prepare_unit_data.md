# Runtime unit data exporter

`python3 tools/prepare_unit_data.py` reads the preserved original `Resources/RawData` and `prepare_unit_data_authoring.json`, then writes `Resources/RuntimeData`. `--output PATH` selects another output directory. No original input is changed.

The initial JSON records were captured from product tree `191728e310158a55df1164ee452b19b48fa97e57` for the resource-path hotfix. Later units are authored directly as JSON records; `authored_extensions` distinguishes their sources and choices. The application does not create these definitions. `prepare_unit_data_baseline.cpp` records the one-time historical capture procedure and deliberately requires the old audited checkout's Core headers and library:

```
c++ -std=c++20 -I OLD_CHECKOUT/Sources/Core tools/prepare_unit_data_baseline.cpp OLD_BUILD/libHeavyMachCore.a -o capture-authoring
./capture-authoring OLD_CHECKOUT/Resources/RawData > captured.json
```

The old factory implementation is not retained in production Core. Reproduction of the capture requires the named historical checkout, not the new Core. Routine package regeneration only uses Python and the committed authoring JSON.

The writer copies original MCH child nodes and map groups/children without reserializing their payloads. It appends new original-format nodes and changes only the enclosing child counts. The original stale size words are preserved because the original reader traverses payload length and child count. Before writing, it verifies every retained node's bytes against the original. All 46 files are emitted; unchanged maps remain byte-identical. The manifest records original input and derived output SHA-256 separately.

Added runtime records: 27 weapons, 22 machines, 86 parts, 304 fixed map placements. The aircraft exhausts are actual nodes in the common part trees. The body replaces the invisible anchor, preserving its sockets. Fourteen formations remain explicit extension events rather than fake static map enemies. Ka52 and Lux_Eurofighter add two machines, eight parts and three weapons; their introduction does not add or replace campaign placements or formations.

Ka52 uses the original `partsType=3` / `tileCnt=2` rotor animation for two vertically separated coaxial rotor nodes. These are native MCH part records, with no custom rotor clock or aircraft-specific renderer. Two frames show animation; they do not establish visibly measured opposite rotation directions. Its cannon and missile launcher are visible `partsType=2` children and are read by the same weapon parser and combat path as original units. Their stats inherit the recorded original weapon values. The missile's `rotWpn=1` is a new authored choice for a visible rotating launcher, not a claim that the invisible original E_Wpn_ASM had this value.

`aircraft_weapon_mounts` optionally selects `part_sockets` for an aircraft with a visible swivelling launcher. This leaves the existing mounted-origin flag off so the original socket and weapon-rotation code computes its muzzle. The default `fixed_muzzle` preserves the six earlier aircraft contracts. Lux_Eurofighter uses `part_sockets`; its burst values inherit Lux_F22_Missile while its visible launcher uses `rotWpn=1`. No new runtime name dispatch or C++ definition factory is involved.

`machine_gauge_offsets` authors the existing HMEX custom-gauge property without changing MCH or adding renderer dispatch. Ka52 uses -18 so its HP bar stays below its moving body and weapons. Its body anchor is Heli006's `(-26,-18)` multiplied by the body-width ratio `112/126`, giving `(-23.111111,-16)`; the common original bounds calculation remains in use. This gauge offset is an explicit restoration-engine extension, not an original MCH field. `part_record_order` names every added part once and keeps all earlier runtime part records in their existing byte order before the eight new parts. Invalid or incomplete orders are rejected.

`HeavyMachUnitAssemblyTests` runs actual common AI, animation, weapon muzzle geometry, damage, destruction, pause and save-event checks. `HeavyMachUnitAssemblyValidationTests` repeats those conditions with the validation build and verifies the read-only bullet/damage observations against real weapon impacts. They require the real packaged image files; dummy dimensions are not accepted for the seven new images. These core checks do not replace the separately recorded web render and play inspection.

`UnitExtensions.hmx` is a distinct binary contract, not an MCH extension disguised as the same file format. All integers are little-endian, floats are IEEE float32, strings use a uint16 byte length with no terminator. The header is the eight ASCII bytes `HMEX0001`. Each section starts with a uint32 record count:

1. Part presentation: part name; logical size (2 floats); animation byte (0 original frame state, 1 world-clock loop); period float; effect byte (0 none, 1 pulsed exhaust); column count byte; frame count uint32; frames of tile-name string and offset (2 floats); nozzle (2 floats); aft-direction byte; seven floats for base length, vertical velocity reference/amplitude, pulse frequency/amplitude, thickness and alpha.
2. Machine behavior: machine name; uint32 flags (bounds=1, gauge=2, engine bounce=4, private RNG=8, velocity pitch=16, mounted weapon origin=32, rotated bounds=64); four bounds floats; gauge Y; muzzle XY; horizontal/vertical pitch degrees, horizontal/vertical speed reference and easing (five floats).
3. Weapon behavior: weapon name; AI time scale float; burst group delay float; burst group size byte.
4. Stage formation: map name; trigger boss name (empty means final approach); entry/release speed floats; uint32 unit count; units of machine name and offset XY.

The parser rejects bad versions, truncation, oversized counts, non-finite floats, unknown/duplicate referenced IDs, invalid periods/speeds/effect values, missing boss triggers and trailing bytes. Original MCH/MAP parsers are unchanged. `GameData::loadDirectory` reads the same fixed MCH filenames and MAP files, then applies this explicit extension. Original-data tools may leave it optional; product callers pass `requireExtensions=true`, which rejects a missing contract before a session can run. Android applies the same bytes through `GameData::applyExtensions` after its asset-buffer parsing.

The common `PartsInfo` presentation chooses real frame files. The common part traversal renders aircraft bodies/exhaust and ground parts; it no longer dispatches by aircraft or GAA names. `GameData::presentationImages` enumerates unique actual image keys for platform preloading. Original parts/machines do not receive extension properties. No new custom crop or resize occurs in Core.

These contracts preserve additional behavior that the original 1.13 file format cannot express. They are documented restoration-engine extensions, not claims that the original game supported these fields.

### 신규 2종 이미지 재현

`python3 tools/prepare_unit_images.py --authoring Art/KamovEurofighter/20261109/bake.json --evidence build/kamov-images --verify`로 최종 7개 PNG와 manifest 전체 행을 검증한다. 출력하려면 `--verify`를 제외한다. 입력 SHA-256과 균등 축소 조건을 먼저 검사한다. 기존 채택 PNG는 보존한다. authored 이미지가 있는 출력에 baseline 전체 재생성을 실행하면 중단한다. 원본 IPA·원작 PNG는 수정하지 않는다.
