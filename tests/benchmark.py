import time
import subprocess
import os

def benchmark(linker_cmd, args, label):
    start = time.perf_counter()
    subprocess.run(linker_cmd + args, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    end = time.perf_counter()
    print(f"{label}: {end - start:.4f} seconds")

# Create a dummy large object file for testing if possible, or use the existing one
test_o = "tests/test_hello.o"
if not os.path.exists(test_o):
    subprocess.run(["clang", "-c", "tests/test_hello.cpp", "-o", test_o, "-target", "x86_64-apple-macos"])

print("Benchmarking linkers (Mach-O)...")
benchmark(["./build/fast-linker"], [test_o], "Fast-Linker (Our implementation)")
# Note: lld and mold might need more flags to work with a single Mach-O object on Linux
# benchmark(["ld64.lld"], [test_o], "LLD")
# benchmark(["mold"], ["-run", "ld64.lld", test_o], "Mold")
