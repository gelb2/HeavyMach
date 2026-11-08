#!/usr/bin/env python3
"""Bake the 63 pre-hotfix images without regenerating approved art.

A read-only pre-hotfix checkout is required. Generated files are standard RGBA8
PNG, not CgBI. Common decoder/premultiply/compatibility round trips are measured
by the native helper; full application/device validation remains a separate gate.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
PROFILES = ("apple", "web", "android")
BASELINE_SOURCES = (
    "Sources/iOS/HMMedia.mm", "Sources/Web/WebMedia.cpp",
    "Android/app/src/main/cpp/Media.cpp", "Sources/iOS/HMCompatibilityPixels.hpp",
    "Sources/Core/VTOL.cpp", "Sources/Core/Fighters.cpp", "Sources/Core/GroundAAArt.inc",
    "vendor/stb/stb_image.h",
)

def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def run(command):
    result = subprocess.run(list(map(str, command)), check=True, text=True, capture_output=True)
    return result.stdout

def bake_authored_units(authoring, output, evidence, verify=False):
    """Bake explicitly authored additions; preserve the adopted 63-image baseline."""
    spec = json.loads(authoring.read_text())
    evidence.mkdir(parents=True, exist_ok=True)
    helper = ROOT / "tools/prepare_unit_images.mm"
    executable = evidence / "prepare_unit_images"
    run(["xcrun", "clang++", "-std=c++20", "-O2", "-ffp-contract=off", helper,
         "-I", ROOT / "vendor/stb", "-I", ROOT / "Sources/iOS",
         "-framework", "CoreGraphics", "-lz", "-o", executable])
    manifest = json.loads((output / "manifest.json").read_text())
    names = [job["name"] for job in spec["images"]]
    if len(set(names)) != len(names):
        raise SystemExit("Duplicate authored image")
    for job in spec["images"]:
        if Path(job["name"]).name != job["name"] or sha(ROOT/job["source"]) != job["source_sha256"]:
            raise SystemExit("Invalid image name or source hash: " + job["name"])
        if len(job["size"]) != 2 or job["size"][0] <= 0 or job["size"][1] != 0:
            raise SystemExit("Authored unit images require uniform width-based scaling")
    with tempfile.TemporaryDirectory(prefix="authored-images-", dir=evidence) as temp:
        stage = Path(temp)
        for profile in PROFILES:
            entries = [entry for entry in manifest["profiles"][profile] if entry["file"] not in names]
            for job in spec["images"]:
                path = stage / profile / job["name"]
                path.parent.mkdir(parents=True, exist_ok=True)
                ref = evidence / profile / Path(job["name"]).stem
                ref.parent.mkdir(parents=True, exist_ok=True)
                command = [executable, profile, ROOT/job["source"], *job["crop"], *job["size"], path, ref]
                if job.get("transform"):
                    command.append(job["transform"])
                measured = json.loads(run(command))
                target = output / profile / job["name"]
                if verify:
                    if not target.exists() or sha(target) != sha(path):
                        raise SystemExit("Authored image differs: " + str(target))
                entry = {"file":job["name"], "source":job["source"],
                    "source_sha256":job["source_sha256"], "source_crop":job["crop"],
                    "source_transform":job.get("transform", "none"),
                    "width":measured["width"], "height":measured["height"],
                    "bytes":path.stat().st_size, "sha256":sha(path),
                    "baseline_premultiplied_sha256":sha(str(ref)+".baseline.rgba"),
                    "decoded_premultiplied_sha256":sha(str(ref)+".decoded.rgba"),
                    "upload_rgba_sha256":sha(str(ref)+".upload.rgba"),
                    "decode_equal":measured["decode_equal"], "upload_equal":measured["upload_equal"]}
                if verify:
                    recorded = [r for r in manifest["profiles"][profile] if r["file"] == job["name"]]
                    if recorded != [entry]:
                        raise SystemExit("Authored image manifest differs: " + profile + "/" + job["name"])
                entries.append(entry)
            manifest["profiles"][profile] = entries
        if not verify:
            for profile in PROFILES:
                for name in names:
                    target = output / profile / name
                    target.parent.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(stage/profile/name, target)
    manifest["authored_additions"] = {"manifest":str(authoring.relative_to(ROOT)),
        "sha256":sha(authoring), "generator_sha256":sha(__file__), "helper_sha256":sha(helper)}
    if not verify:
        (output/"manifest.json").write_text(json.dumps(manifest, ensure_ascii=False, indent=2)+"\n")
    result = {"authored_images":len(names), "profiles":list(PROFILES),
        "decode_upload_roundtrips":len(names)*len(PROFILES), "verified_only":verify}
    (evidence/("verify.json" if verify else "execution.json")).write_text(json.dumps(result, indent=2)+"\n")
    print(json.dumps(result))

def jobs(baseline):
    result = []
    for name, width in (("VTOL_Yak141",112), ("VTOL_Harrier",108), ("VTOL_F35B",116),
                        ("Lux_F22",112), ("Lux_Su57",116), ("Lux_MiG31",120)):
        result.append({"name":name+".png", "source":"Art/Extensions/"+name+".png",
                       "crop":[0,0,0,0], "size":[width,0]})
    manifest = json.loads((baseline/"Art/GroundAA/20261028-shilka-turret/manifest.json").read_text())
    for image in manifest["images"]:
        result.append({"name":Path(image["name"]).name,
                       "source":"Art/"+image["source"], "crop":image["crop"], "size":image["size"]})
    if len(result)!=63 or len({j["name"] for j in result})!=63:
        raise SystemExit("Expected 63 unique baseline images")
    for f in manifest["files"]:
        if sha(baseline/f["path"])!=f["sha256"]:
            raise SystemExit("Approved source hash changed: "+f["path"])
    return result

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline-root",type=Path)
    parser.add_argument("--authoring",type=Path,help="Bake explicit new-unit art without altering adopted baseline images")
    parser.add_argument("--output",type=Path,default=ROOT/"Resources/RuntimeImages")
    parser.add_argument("--evidence",type=Path,default=ROOT/"build/unit-images/evidence")
    parser.add_argument("--verify",action="store_true",help="Compare regenerated outputs without changing packaged files")
    args=parser.parse_args();output=args.output.resolve();evidence=args.evidence.resolve()
    if args.authoring:
        bake_authored_units(args.authoring.resolve(), output, evidence, args.verify)
        return
    if not args.baseline_root:
        parser.error("--baseline-root or --authoring is required")
    baseline=args.baseline_root.resolve()
    # Historical preservation is a separate operation. Never silently erase
    # subsequently authored unit images when reproducing that older baseline.
    current_manifest = output/"manifest.json"
    if current_manifest.exists() and json.loads(current_manifest.read_text()).get("authored_additions"):
        raise SystemExit("Historical baseline reproduction requires a separate --output directory; use --authoring to verify or update new units")
    if output==baseline or output.is_relative_to(baseline/"Art") or output.is_relative_to(baseline/"Resources/OriginalImages"):
        raise SystemExit("Refusing to overwrite baseline art")
    evidence.mkdir(parents=True,exist_ok=True)
    entries=jobs(baseline)
    helper=ROOT/"tools/prepare_unit_images.mm";executable=evidence/"prepare_unit_images"
    compile_command=["xcrun","clang++","-std=c++20","-O2","-ffp-contract=off",helper,
                     "-I",baseline/"vendor/stb","-I",baseline/"Sources/iOS",
                     "-framework","CoreGraphics","-lz","-o",executable]
    run(compile_command)
    compiler=run(["xcrun","clang++","--version"]).splitlines()[0]
    source_hashes={p:sha(baseline/p) for p in BASELINE_SOURCES}
    # Compare the filter implementation to baseline source as part of provenance.
    manifest={"schema":1,"purpose":"Offline preservation of the pre-hotfix 63 logical images",
              "format":{"encoding":"standard PNG (not CgBI)","bit_depth":8,"color_type":6,
                        "alpha":"straight; exact inverse of current float32 premultiply",
                        "interlace":0,"chunks":["IHDR","IDAT","IEND"],"filter":0,"zlib_level":9},
              "baseline_source_sha256":source_hashes,
              "generator_sha256":{"tools/prepare_unit_images.py":sha(__file__),"tools/prepare_unit_images.mm":sha(helper)},
              "apple_bake_environment":{"platform":platform.platform(),"compiler":compiler},
              "roundtrip_scope":"Native helper: baseline crop/filter and common stb/options/premultiply plus compatibility/POT; full application/device verification is separate",
              "profiles":{}}
    with tempfile.TemporaryDirectory(prefix="unit-images-",dir=evidence) as temporary:
        stage=Path(temporary)
        for profile in PROFILES:
            destination=stage/profile;destination.mkdir()
            details=[]
            for job in entries:
                reference=evidence/profile/job["name"][:-4];reference.parent.mkdir(parents=True,exist_ok=True)
                path=destination/job["name"]
                command=[executable,profile,baseline/job["source"],*job["crop"],*job["size"],path,reference]
                measured=json.loads(run(command))
                details.append({"file":job["name"],"source":job["source"],"source_sha256":sha(baseline/job["source"]),
                                "source_crop":job["crop"],"width":measured["width"],"height":measured["height"],
                                "bytes":path.stat().st_size,"sha256":sha(path),
                                "baseline_premultiplied_sha256":sha(str(reference)+".baseline.rgba"),
                                "decoded_premultiplied_sha256":sha(str(reference)+".decoded.rgba"),
                                "upload_rgba_sha256":sha(str(reference)+".upload.rgba"),
                                "decode_equal":measured["decode_equal"],"upload_equal":measured["upload_equal"]})
            manifest["profiles"][profile]=details
        (stage/"manifest.json").write_text(json.dumps(manifest,ensure_ascii=False,indent=2)+"\n")
        if args.verify:
            expected={str(p.relative_to(stage)):sha(p) for p in stage.rglob("*") if p.is_file()}
            actual={str(p.relative_to(output)):sha(p) for p in output.rglob("*") if p.is_file()}
            if expected!=actual:raise SystemExit("Generated runtime images differ from tracked output")
        else:
            output.mkdir(parents=True,exist_ok=True)
            for profile in PROFILES:
                target=output/profile
                if target.exists():shutil.rmtree(target)
                shutil.copytree(stage/profile,target)
            shutil.copy2(stage/"manifest.json",output/"manifest.json")
    execution={"command":["python3","tools/prepare_unit_images.py","--baseline-root",str(baseline),"--output",str(output),"--evidence",str(evidence)]+(["--verify"] if args.verify else []),
               "compile_command":list(map(str,compile_command)),"profiles":list(PROFILES),"images_per_profile":63,
               "successful_roundtrips":189,"manifest_sha256":sha(output/"manifest.json"),"verified_only":args.verify}
    (evidence/"execution.json").write_text(json.dumps(execution,ensure_ascii=False,indent=2)+"\n")
    print(json.dumps(execution,ensure_ascii=False))

if __name__=="__main__":main()
