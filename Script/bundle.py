#!/usr/bin/env python3
"""
Script/bundle.py

Bundling script (macOS) - modified to:
  1) Locate the .app bundle (or use --app-bundle)
  2) Modify the provided Info.plist template (update CFBundleVersion and CFBundleShortVersionString)
     and copy it into <App>.app/Contents/Info.plist (backing up existing one)
  3) Copy the app icon (.icns) into <App>.app/Contents/Resources/ (named according to CFBundleIconFile)
  4) Finally run Qt6's macdeployqt to bundle Qt dependencies into the .app and optionally produce a dmg
  5) Move any produced dmg(s) into --out-dir

Usage example:
  python Script/bundle.py --build-dir build --info Info.plist --icon assets/DrawingBoard.icns \
      --qt-prefix /Users/doyoung/Qt --qt-version-subdir "6.5.3/macos" --short-version 1.2 --build-version 42

Notes:
  - The script finds the first .app under --build-dir if --app-bundle is not given.
  - You can override the macdeployqt executable path with --macdeployqt.
  - Any extra args after "--" are forwarded to macdeployqt (if none provided, "-dmg" is used).
"""
from __future__ import annotations
import argparse
import glob
import os
import plistlib
import shutil
import subprocess
import sys
import shlex
from typing import Optional

def shlex_quote(s: str) -> str:
    if " " in s or '"' in s or "'" in s:
        return "'" + s.replace("'", "'\"'\"'") + "'"
    return s

def run(cmd, cwd=None):
    print("Running:", " ".join(shlex_quote(str(c)) for c in cmd))
    subprocess.check_call(cmd, cwd=cwd)

def find_app(build_dir: str) -> Optional[str]:
    for root, dirs, _ in os.walk(build_dir):
        for d in dirs:
            if d.endswith(".app"):
                return os.path.join(root, d)
    return None

def load_plist(path: str) -> dict:
    with open(path, "rb") as f:
        return plistlib.load(f)

def write_plist(data: dict, path: str):
    # write XML style plist
    with open(path, "wb") as f:
        plistlib.dump(data, f, fmt=plistlib.FMT_XML)

def backup_file(path: str):
    if os.path.exists(path):
        bak = path + ".bak"
        print(f"Backing up existing {path} -> {bak}")
        shutil.copy2(path, bak)

def main():
    parser = argparse.ArgumentParser(description="Bundle macOS .app: modify Info.plist, copy icon, then run macdeployqt.")
    parser.add_argument("--build-dir", default="build", help="Build directory to search for .app")
    parser.add_argument("--app-bundle", help="Path to the .app bundle (if omitted, the script finds the first .app in build-dir)")
    parser.add_argument("--info", dest="info_plist", default="Info.plist",
                        help="Source Info.plist template to modify and copy into the app's Contents/")
    parser.add_argument("--short-version", dest="short_version", default=None,
                        help="Value to set for CFBundleShortVersionString")
    parser.add_argument("--build-version", dest="build_version", default=None,
                        help="Value to set for CFBundleVersion")
    parser.add_argument("--icon", dest="icon_path", default=None,
                        help="Path to .icns file to copy into Contents/Resources (if omitted, script will try to auto-find)")
    parser.add_argument("--qt-prefix", default=os.environ.get("QT_INSTALL_PREFIX", "/Users/doyoung/Qt"),
                        help="Qt install prefix")
    parser.add_argument("--qt-version-subdir", default="6.5.3/macos",
                        help='Qt version subdir to find macdeployqt (e.g. "6.5.3/macos")')
    parser.add_argument("--macdeployqt", help="Full path to macdeployqt (overrides qt-prefix and qt-version-subdir)")
    parser.add_argument("--out-dir", default="dist", help="Directory to copy final artifacts (dmgs) to")
    parser.add_argument("--no-dmg", action="store_true", help="Do not expect macdeployqt to create a dmg (skip dmg move step)")
    parser.add_argument("extra", nargs=argparse.REMAINDER,
                        help="Extra args forwarded to macdeployqt (use -- before these)")
    args = parser.parse_args()

    build_dir = args.build_dir
    app_bundle = args.app_bundle
    info_plist_template = args.info_plist
    short_version = args.short_version
    build_version = args.build_version
    icon_path = args.icon_path
    qt_prefix = args.qt_prefix
    qt_version_subdir = args.qt_version_subdir
    macdeployqt_path = args.macdeployqt
    out_dir = args.out_dir
    no_dmg = args.no_dmg
    extra = args.extra
    if extra and extra[0] == "--":
        extra = extra[1:]

    # 1) locate app bundle if not provided
    if not app_bundle:
        app_bundle = find_app(build_dir)
        if not app_bundle:
            print(f"Error: could not find any .app under {build_dir}. Please build a macOS bundle first or pass --app-bundle", file=sys.stderr)
            sys.exit(2)
        print(f"Auto-detected app bundle: {app_bundle}")

    if not os.path.isdir(app_bundle):
        print(f"Error: specified app bundle does not exist: {app_bundle}", file=sys.stderr)
        sys.exit(3)

    contents_dir = os.path.join(app_bundle, "Contents")
    resources_dir = os.path.join(contents_dir, "Resources")
    if not os.path.isdir(contents_dir):
        print(f"Error: {app_bundle} does not look like a valid .app (missing Contents/)", file=sys.stderr)
        sys.exit(4)

    # 2) modify Info.plist template and copy into app Contents (backup existing)
    if not os.path.isfile(info_plist_template):
        print(f"Error: Info.plist template not found at {info_plist_template}", file=sys.stderr)
        sys.exit(5)

    try:
        plist_data = load_plist(info_plist_template)
    except Exception as e:
        print(f"Error: failed to read Info.plist template {info_plist_template}: {e}", file=sys.stderr)
        sys.exit(6)

    if build_version is not None:
        print(f"Setting CFBundleVersion = {build_version}")
        plist_data["CFBundleVersion"] = str(build_version)
    if short_version is not None:
        print(f"Setting CFBundleShortVersionString = {short_version}")
        plist_data["CFBundleShortVersionString"] = str(short_version)

    # Determine icon name to use from plist (CFBundleIconFile) or from provided icon path
    icon_basename = None
    if "CFBundleIconFile" in plist_data and plist_data["CFBundleIconFile"]:
        icon_basename = plist_data["CFBundleIconFile"]
    if icon_path:
        if not os.path.isfile(icon_path):
            print(f"Error: icon file not found: {icon_path}", file=sys.stderr)
            sys.exit(7)
        icon_filename = os.path.basename(icon_path)
        icon_stem, icon_ext = os.path.splitext(icon_filename)
        if icon_basename is None:
            icon_basename = icon_stem
    if icon_basename is None:
        icon_basename = "AppIcon"
        print(f"No CFBundleIconFile specified; defaulting to {icon_basename}")

    dest_info = os.path.join(contents_dir, "Info.plist")
    # backup_file(dest_info)
    try:
        write_plist(plist_data, dest_info)
        print(f"Wrote modified Info.plist -> {dest_info}")
    except Exception as e:
        print(f"Error: failed to write modified Info.plist to {dest_info}: {e}", file=sys.stderr)
        sys.exit(8)

    # 3) copy icon into Contents/Resources (name should be <icon_basename>.icns)
    os.makedirs(resources_dir, exist_ok=True)
    dest_icon_name = f"{icon_basename}.icns"
    dest_icon_path = os.path.join(resources_dir, dest_icon_name)

    if icon_path:
        try:
            shutil.copy2(icon_path, dest_icon_path)
            print(f"Copied icon {icon_path} -> {dest_icon_path}")
        except Exception as e:
            print(f"Error: failed to copy icon to {dest_icon_path}: {e}", file=sys.stderr)
            sys.exit(9)
    else:
        # try to find an .icns in common locations
        candidates = []
        for p in [".", "resources", "Resources", "assets", os.path.join("assets", "icons")]:
            if not os.path.isdir(p):
                continue
            for candidate in glob.glob(os.path.join(p, "*.icns")):
                candidates.append(candidate)
        if candidates:
            chosen = candidates[0]
            shutil.copy2(chosen, dest_icon_path)
            print(f"Auto-copied icon {chosen} -> {dest_icon_path}")
        else:
            print(f"Warning: no icon provided and none found in common locations; skipping icon copy. Expected {dest_icon_name} in Contents/Resources", file=sys.stderr)

    # 4) finally run macdeployqt to bundle Qt (and optionally create dmg)
    if macdeployqt_path:
        macdeployqt = macdeployqt_path
    else:
        macdeployqt = os.path.join(qt_prefix, qt_version_subdir, "bin", "macdeployqt6")

    if not os.path.isfile(macdeployqt) or not os.access(macdeployqt, os.X_OK):
        print(f"Error: macdeployqt not found or not executable at {macdeployqt}", file=sys.stderr)
        print("You can specify path with --macdeployqt or adjust --qt-prefix/--qt-version-subdir", file=sys.stderr)
        sys.exit(10)

    cmd = [macdeployqt, app_bundle]
    if extra:
        cmd.extend(extra)
    else:
        cmd.append("-dmg")

    try:
        print("Running macdeployqt (bundling Qt into the app)...")
        run(cmd)
    except subprocess.CalledProcessError as e:
        print("macdeployqt failed with exit code", e.returncode, file=sys.stderr)
        sys.exit(e.returncode)

    # 5) find dmg(s) created by macdeployqt and move to out_dir (unless no_dmg)
    if not no_dmg:
        os.makedirs(out_dir, exist_ok=True)
        app_name = os.path.basename(app_bundle)
        base_name = app_name[:-4] if app_name.endswith(".app") else app_name
        found_dmgs = []
        search_locations = [os.getcwd(), build_dir, os.path.dirname(build_dir)]
        for loc in search_locations:
            if not loc:
                continue
            for p in glob.glob(os.path.join(loc, f"{base_name}*.dmg")):
                found_dmgs.append(os.path.abspath(p))
        for p in glob.glob(f"{base_name}*.dmg"):
            found_dmgs.append(os.path.abspath(p))
        # Deduplicate
        found_dmgs = list(dict.fromkeys(found_dmgs))

        for dmg in found_dmgs:
            try:
                dest = os.path.join(out_dir, os.path.basename(dmg))
                print(f"Moving {dmg} -> {dest}")
                shutil.move(dmg, dest)
            except Exception as e:
                print(f"Warning: could not move {dmg} to {out_dir}: {e}", file=sys.stderr)

        if found_dmgs:
            print("Created dmg(s):")
            for d in found_dmgs:
                print("  ", d)
        else:
            print("macdeployqt completed. No dmg file was found by the script. Check macdeployqt output to see where the dmg was written.")

    print("Bundle step finished.")

if __name__ == "__main__":
    main()