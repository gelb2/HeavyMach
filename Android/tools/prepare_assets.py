#!/usr/bin/env python3
"""Stage the Android runtime package without modifying original resources."""
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[2]
DEST=Path(sys.argv[1]).resolve()
if ROOT/'Android/app/build' not in DEST.parents:raise SystemExit('Output must be under Android/app/build')
sys.path.insert(0,str(ROOT/'tools'))
from stage_runtime_assets import stage
stage('android',DEST)
