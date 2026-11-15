"""Compile the ordinary Hunter protocol integration tests with a native C++17 compiler."""
from pathlib import Path
import argparse
import subprocess
import tempfile

p = argparse.ArgumentParser(description=__doc__)
p.add_argument("--cxx", default="g++")
args = p.parse_args()
root = Path(__file__).resolve().parents[1]
core = root / "libraries/LayerTime-Core/src"
sensors = root / "sensors/src"
with tempfile.TemporaryDirectory(prefix="hunter-harness-tests-") as temp:
    exe = Path(temp) / "tests"
    cmd = [args.cxx, "-std=c++17", "-O0", "-Wall", "-Wextra", "-I"+str(root/"src"), "-I"+str(core), "-I"+str(sensors)]
    cmd += [str(root/"test/test_harness_protocol.cpp")]
    cmd += [str(x) for x in sorted(core.rglob("*.cpp"))]
    cmd += [str(x) for x in sorted(sensors.rglob("*.cpp"))]
    subprocess.run(cmd+["-o", str(exe)], check=True)
    subprocess.run([str(exe)], check=True)
