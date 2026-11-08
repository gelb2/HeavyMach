#!/usr/bin/env python3
"""Capture identical deterministic gameplay observations against two checkouts.

No reference traces are silently updated. `capture` refuses an existing output.
`compare` reports every changed channel, keeping intentional VTOL map changes
separate. Draw comparisons describe logical geometry; GPU pixels are NOT tested.
"""
import argparse
import collections
import gzip
import hashlib
import json
import pathlib
import subprocess
import sys

HERE = pathlib.Path(__file__).resolve().parents[1]

def sha(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b''): h.update(chunk)
    return h.hexdigest()

def command(args, log):
    r = subprocess.run(args, stdout=log, stderr=subprocess.STDOUT)
    if r.returncode: raise RuntimeError(f'command failed ({r.returncode}): {args}; see {log.name}')

def capture(args):
    tree, out = args.checkout.resolve(), args.output.resolve()
    out.mkdir(parents=True, exist_ok=False)
    observer = HERE / 'Tests/UnitContractTrace.cpp'
    def input_hashes():
        paths = [*sorted((tree / 'Sources/Core').glob('*')), *sorted((tree / ('Resources/RawData' if args.baseline else 'Resources/RuntimeData')).glob('*'))]
        return {str(p.relative_to(tree)): sha(p) for p in paths if p.is_file()}
    inputs_before = input_hashes()
    wrapper = out / 'wrapper'; wrapper.mkdir()
    (wrapper / 'CMakeLists.txt').write_text(
        'cmake_minimum_required(VERSION 3.20)\nproject(UnitContract LANGUAGES CXX)\n'
        'set(CMAKE_CXX_STANDARD 20)\nset(HM_BUILD_VALIDATION_TESTS ON CACHE BOOL "" FORCE)\n'
        f'add_subdirectory("{tree.as_posix()}" product)\n'
        f'add_executable(UnitContractTrace "{observer.as_posix()}")\n'
        'target_link_libraries(UnitContractTrace PRIVATE HeavyMachValidationCore)\n'
        'target_compile_options(UnitContractTrace PRIVATE -ffp-contract=off)\n' +
        ('target_compile_definitions(UnitContractTrace PRIVATE HM_UNIT_CONTRACT_BASELINE=1)\n' if args.baseline else ''))
    with (out / 'build.log').open('w') as log:
        command(['cmake', '-S', str(wrapper), '-B', str(out / 'cmake'), '-DCMAKE_BUILD_TYPE=Debug'], log)
        command(['cmake', '--build', str(out / 'cmake'), '--target', 'UnitContractTrace', '-j', '4'], log)
    trace = out / 'trace.tsv.gz'
    with (out / 'run.log').open('w') as err, trace.open('wb') as raw:
        p = subprocess.Popen([str(out / 'cmake/UnitContractTrace'), str(tree)], stdout=subprocess.PIPE, stderr=err)
        with gzip.GzipFile(fileobj=raw, mode='wb', mtime=0) as compressed:
            for chunk in iter(lambda: p.stdout.read(1024 * 1024), b''): compressed.write(chunk)
        code = p.wait()
    meta = {'checkout': str(tree), 'baseline_factory': args.baseline,
            'revision': subprocess.check_output(['git', '-C', str(tree), 'rev-parse', 'HEAD'], text=True).strip(),
            'status': subprocess.check_output(['git', '-C', str(tree), 'status', '--porcelain'], text=True),
            'observer_sha256': sha(observer), 'trace_sha256': sha(trace), 'exit_code': code,
            'input_hashes': input_hashes(), 'input_hashes_before_build': inputs_before,
            'scope': 'deterministic core state, RNG, enemy weapon AI, live bullets, ordered logical draws, events, save bytes; not GPU pixels or real-device performance'}
    (out / 'capture.json').write_text(json.dumps(meta, indent=2) + '\n')
    if meta['input_hashes'] != inputs_before: raise RuntimeError('Inputs changed during build/capture; preserve this result but do not use it as a clean comparison')
    if code: raise RuntimeError(f'trace failed ({code}); see {out / "run.log"}')
    print(json.dumps({'output': str(out), 'trace_sha256': meta['trace_sha256']}))

def groups(path):
    with gzip.open(path, 'rt') as f:
        current, rows = None, []
        for line in f:
            fields = line.rstrip('\n').split('\t')
            key = tuple(fields[:2])
            if current is not None and key != current:
                yield current, rows
                rows = []
            current = key; rows.append(fields)
        if current is not None: yield current, rows

def compare(args):
    am = json.loads((args.baseline / 'capture.json').read_text()); bm = json.loads((args.candidate / 'capture.json').read_text())
    if am['exit_code'] or bm['exit_code']: raise RuntimeError('A capture failed; partial trace must not pass comparison')
    if am['observer_sha256'] != bm['observer_sha256']: raise RuntimeError('Observer sources differ; compare identical harness only')
    for meta in (am, bm):
        if 'input_hashes_before_build' in meta and meta['input_hashes_before_build'] != meta['input_hashes']: raise RuntimeError('Source/data inputs changed during capture')
    counts = collections.Counter(); differences = []; coverage = {}; stage_presence = {}; campaign = {}
    left, right = iter(groups(args.baseline / 'trace.tsv.gz')), iter(groups(args.candidate / 'trace.tsv.gz'))
    missing = object()
    from itertools import zip_longest
    for a, b in zip_longest(left, right, fillvalue=missing):
        if a is missing or b is missing or a[0] != b[0]:
            raise RuntimeError('scenario ordering/length changed; refusing to realign or drop traces')
        key, before = a; _, after = b
        if key[0].startswith('MAP:'):
            # Intentional spawn policy is isolated. All original/non-VTOL rows
            # still compare exactly; no broad map-difference waiver.
            def non_vtol(rows): return [r[4:] for r in rows if not r[4].startswith('VTOL_')]
            if non_vtol(before) != non_vtol(after): counts['map_original_or_non_vtol'] += 1
            if before != after:
                counts['map_vtol_policy_change' if args.allow_vtol_map_change and non_vtol(before) == non_vtol(after) else 'map_unapproved'] += 1
            stage_presence[key[0]] = {name: sum(r[4] == name for r in after) for name in ['VTOL_Yak141','VTOL_Harrier','VTOL_F35B']}
            continue
        if key[0].startswith('CAMPAIGN:'):
            campaign[key[0]] = {r[3]: [int(x) for x in r[5:8]] for r in after}
            if before != after:
                counts['map_vtol_policy_change' if args.allow_vtol_map_change else 'map_unapproved'] += 1
            continue
        for channel in sorted(set(r[2] for r in before + after)):
            aa = [r for r in before if r[2] == channel]; bb = [r for r in after if r[2] == channel]
            if aa != bb:
                counts[channel] += 1
                if len(differences) < 40: differences.append({'scenario_tick': key, 'channel': channel, 'baseline': aa[:3], 'candidate': bb[:3]})
            if channel == 'COVERAGE': coverage[key[0]] = {'baseline': aa, 'candidate': bb}
    missions = collections.defaultdict(lambda: [0, 0, 0])
    policy_failures = []
    for stage, widths in campaign.items():
        narrow, wide = widths.get('480'), widths.get('900')
        if narrow is None or wide is None or narrow != wide or sum(narrow) < 1:
            policy_failures.append(stage)
        if narrow:
            mission = int(stage.split(':')[1])
            missions[mission] = [a + b for a, b in zip(missions[mission], narrow)]
    if len(campaign) != 35 or len(missions) != 7 or any(min(c) < 1 for c in missions.values()) or sum(sum(c) for c in missions.values()) != 152:
        policy_failures.append('requires 7 missions with all 3 types, 35 occupied stages, 152 total')
    failures = {k: v for k, v in counts.items() if k != 'map_vtol_policy_change'}
    if policy_failures: failures['vtol_policy'] = policy_failures
    result = {'equal': not failures, 'changed_tick_counts': dict(counts), 'first_differences': differences,
              'vtol_policy': {'stages': len(campaign), 'missions': dict(missions), 'total': sum(sum(c) for c in missions.values()), 'failures': policy_failures, 'scope': 'all three types per mission; at least one VTOL per stage; regeneration at width480/900; no repeated spawn'},
              'coverage_columns': ['moved','enemy_bullet','player_bullet','target_damaged'], 'coverage': coverage,
              'exceptions': ['Only VTOL map entries may change'] if args.allow_vtol_map_change else [],
              'limitation': 'Exact logical command/state comparison; GPU pixels and real-device execution require separate checks.'}
    args.output.write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps({'equal': result['equal'], 'changed_tick_counts': dict(counts), 'vtol_policy_failures': policy_failures, 'report': str(args.output)}))
    return 0 if result['equal'] else 1

p = argparse.ArgumentParser(description=__doc__); sub = p.add_subparsers(dest='action', required=True)
c = sub.add_parser('capture'); c.add_argument('--checkout', type=pathlib.Path, required=True); c.add_argument('--output', type=pathlib.Path, required=True); c.add_argument('--baseline', action='store_true')
c = sub.add_parser('compare'); c.add_argument('--baseline', type=pathlib.Path, required=True); c.add_argument('--candidate', type=pathlib.Path, required=True); c.add_argument('--output', type=pathlib.Path, required=True); c.add_argument('--allow-vtol-map-change', action='store_true')
if __name__ == '__main__':
    args = p.parse_args()
    try: sys.exit(capture(args) if args.action == 'capture' else compare(args))
    except (RuntimeError, OSError, subprocess.CalledProcessError) as e: print(str(e), file=sys.stderr); sys.exit(2)
