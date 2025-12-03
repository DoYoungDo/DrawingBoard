#!/usr/bin/env python3
"""
Script/build.py

Configure and build the project (steps 1 & 2 you described):
  1) run cmake configure (with cmake/qt_toolchain.cmake)
  2) run make in the build directory

Usage:
  python Script/build.py                   # default settings
  python Script/build.py --build-dir build --qt-prefix /Users/doyoung/Qt --qt-cmake-dir /Users/doyoung/Qt/6.5.3/macos/lib/cmake --generator "Unix Makefiles" --type Debug -j 8
"""
import argparse
import os
import shlex
import shutil
import subprocess
import sys
import multiprocessing

def run(cmd, cwd=None):
    print("Running:", " ".join(shlex.quote(c) for c in cmd))
    subprocess.check_call(cmd, cwd=cwd)

def main():
    parser = argparse.ArgumentParser(description="Configure and build (cmake + make) using cmake/qt_toolchain.cmake")
    parser.add_argument("--build-dir", default="build", help="Build directory")
    parser.add_argument("--qt-prefix", default=os.environ.get("QT_INSTALL_PREFIX", "/Users/doyoung/Qt"),
                        help="Qt install prefix")
    parser.add_argument("--qt-cmake-dir", default=os.environ.get("Qt6_DIR", "/Users/doyoung/Qt/6.5.3/macos/lib/cmake/Qt6"),
                        help="Qt6 CMake dir (will be passed as -DQt6_DIR)")
    parser.add_argument("--generator", default="Unix Makefiles", help='CMake generator (default: "Unix Makefiles")')
    parser.add_argument("--type", default="Debug", help="CMAKE_BUILD_TYPE (Debug/Release)")
    parser.add_argument("-j", "--jobs", type=int, default=max(1, multiprocessing.cpu_count()),
                        help="Parallel jobs for make")
    parser.add_argument("--cmake-exe", default="cmake", help="cmake executable")
    parser.add_argument("--toolchain", default="cmake/qt_toolchain.cmake", help="Relative path to toolchain file")
    args, extra = parser.parse_known_args()

    build_dir = args.build_dir
    cmake = args.cmake_exe
    qt_prefix = args.qt_prefix
    qt_cmake_dir = args.qt_cmake_dir
    generator = args.generator
    build_type = args.type
    jobs = args.jobs
    toolchain = args.toolchain

    # Ensure build dir parent exists
    os.makedirs(build_dir, exist_ok=True)

    # Step 1: configure
    cmd_config = [
        cmake,
        "-S", ".",
        "-B", build_dir,
        "-G", generator,
        f"-DCMAKE_TOOLCHAIN_FILE={os.path.normpath(toolchain)}",
        f"-DQT_INSTALL_PREFIX={qt_prefix}",
        f"-DQt6_DIR={qt_cmake_dir}",
        f"-DCMAKE_BUILD_TYPE={build_type}",
    ]
    # forward any extra args passed after known args
    if extra:
        cmd_config.extend(extra)

    try:
        run(cmd_config)
    except subprocess.CalledProcessError as e:
        print("cmake configure failed with exit code", e.returncode, file=sys.stderr)
        sys.exit(e.returncode)

    # Step 2: build using make
    # Prefer using cmake --build but we honor user's preference of GNU make generator by invoking make directly.
    use_make = (generator.lower().find("make") != -1) or (os.path.exists(os.path.join(build_dir, "Makefile")))
    try:
        if use_make:
            # Direct make -C build -jN
            cmd_make = ["make", "-C", build_dir, f"-j{jobs}"]
            run(cmd_make)
        else:
            # fallback to cmake --build
            cmd_build = ["cmake", "--build", build_dir, "--", f"-j{jobs}"]
            run(cmd_build)
    except subprocess.CalledProcessError as e:
        print("build failed with exit code", e.returncode, file=sys.stderr)
        sys.exit(e.returncode)

    print("Build finished successfully.")

if __name__ == "__main__":
    main()