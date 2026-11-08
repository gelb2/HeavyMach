#!/usr/bin/env python3
"""Exhaust the scalar float32 inverse; does not read or write image assets.

This models each arithmetic rounding in uint8(float(S)*(float(A)/255.f)).
It does not execute compiled platform loaders or test image/GPU equivalence.
"""
import hashlib
import json
import struct


def f32(value):
    return struct.unpack("<f", struct.pack("<f", value))[0]


def premultiply(straight, alpha):
    scale = f32(f32(alpha) / f32(255))
    return int(f32(f32(straight) * scale))


missing = []
inverse = bytearray()
combinations = 0
for alpha in range(256):
    first_input = {}
    for straight in range(256):
        first_input.setdefault(premultiply(straight, alpha), straight)
    for channel in range(alpha + 1):
        combinations += 1
        if channel not in first_input:
            missing.append({"alpha": alpha, "premultiplied_channel": channel})
        else:
            inverse.append(first_input[channel])

result = {
    "model": "IEEE754 binary32 rounding after division and multiplication; uint8 truncation",
    "expression": "uint8(float(S) * (float(A) / 255.f))",
    "alpha_range": [0, 255],
    "premultiplied_channel_range": "0 <= C <= A",
    "straight_input_range": [0, 255],
    "tie_rule": "lowest S among exact preimages",
    "tested_alpha_channel_combinations": combinations,
    "missing_preimages": missing,
    "all_valid_premultiplied_channels_representable": not missing,
    "inverse_table_order": "alpha ascending, then channel ascending",
    "inverse_table_sha256": hashlib.sha256(inverse).hexdigest(),
    "limits": [
        "Scalar arithmetic model only; no compiled platform loader execution.",
        "No PNG assets, 63-image bake, color conversion, POT, or GPU comparison performed.",
        "Targets with C > A or nonzero RGB at A = 0 are outside this proof.",
    ],
}
print(json.dumps(result, ensure_ascii=False, indent=2))
raise SystemExit(0 if not missing else 1)
