# New-unit before/after observations

`UnitContractTrace.cpp` is compiled unchanged against the preserved pre-hotfix tree and the candidate. `tools/capture_unit_contract.py` builds an isolated wrapper; it does not edit either checkout's build configuration. The baseline build alone defines `HM_UNIT_CONTRACT_BASELINE` to call the former runtime factories. The candidate loads `Resources/RuntimeData` and requires `UnitExtensions.hmx`.

The observer records actual Core execution: shared and per-machine RNG, enemy motion and weapon AI, live bullets, ordered draw quads, events and final save bytes. Private access is used for controlled fixture setup and read-only observations; there is no copied AI, collision or firing algorithm. It does not insert bullets inside targets. All 44 combat fixtures must exhibit movement, enemy fire, player fire and target damage.

Fixtures cover the 8 new visual unit families and original Tank001/Heli001/E0505, seeds 1 and 987654, widths 480 and 900, ground-unit ramp contact, pause/resume, jump, tilt and live manual fire. Player HP is explicitly set to 100000 in the isolated test save to keep the observation running. This is not a normal-difficulty completion test. Each fixture runs 901 ticks.

Campaign observations call the real `WorldState::processRegen` across all 35 available stages at widths 480 and 900, then repeat regeneration to reject duplicate spawns. The policy is all three VTOL types in each of the seven missions, at least one VTOL per stage, 152 total. It does not require all three types in every individual stage.

Example:

```
python3 tools/capture_unit_contract.py capture --checkout /path/to/preserved-tree --output build/unit-contract/before --baseline
python3 tools/capture_unit_contract.py capture --checkout . --output build/unit-contract/after
python3 tools/capture_unit_contract.py compare --baseline build/unit-contract/before --candidate build/unit-contract/after --output build/unit-contract/comparison.json
```

Existing capture output is never overwritten. Failed or partial captures, different observer sources, and source/data changes during capture cannot pass the comparison. When a separately approved VTOL placement change is implemented, `--allow-vtol-map-change` only exempts VTOL map rows; original and other new-unit placements still compare exactly, and the mission/stage policy is still checked. No placement exception was used for the structural hotfix.

Only image namespace changes (`Extensions/` removal for aircraft/GAA) are normalized. Draw sizes are compared as displayed logical quads, using texture dimensions, frame columns/rows and scale. GPU pixels, image decoding and texture filtering require separate platform checks. The trace includes only the default player gun; existing all-20-weapon and 14-formation tests remain required. It does not substitute for every original enemy, all normal campaign progression, actual phone installation or performance tests.

The public result is `docs/evidence/new-unit-hotfix-20261109/core-trace-comparison.json`. Full traces and local paths remain under ignored `build/unit-contract/`. A negative control that changes one observed enemy HP value must produce a failing comparison, rather than silently accepting differences.
