import os
import sys
import argparse
import tempfile
import subprocess
import pathlib
from shutil import which

this_file_dir = pathlib.Path(os.path.dirname(os.path.abspath(__file__)))
runtime_c_file = this_file_dir / ".." / "runtime" / "runtime.c"
external_llvm_project = this_file_dir / ".." / ".." /"external" /"llvm-project"

clang_dir = str((external_llvm_project/"clang"))
llvm_dir = str(external_llvm_project/"llvm")
this_file_dir = str(this_file_dir)

os.environ["PATH"] = os.pathsep.join([clang_dir, llvm_dir, this_file_dir, os.environ["PATH"]])
calcc_path = which("calcc")
clang_path = which("clang")
llc_path = which("llc")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("file", type=str)
    parser.add_argument("--output", "-o", default=None, type=str, required=False)
    parser.add_argument("--verbose", action="store_true")
    args = parser.parse_args()

    if args.verbose:
        sys.stderr.write(f"Use calcc: {calcc_path}\n")
        sys.stderr.write(f"Use clang: {clang_path}\n")
        sys.stderr.write(f"Use llc: {llc_path}\n")

    expr = open(args.file).read().strip()

    with tempfile.TemporaryDirectory(prefix="calcc") as d:
        expr_ll_file = os.path.join(d, "expr.ll")
        expr_o_file = os.path.join(d, "expr.o")
        runtime_o_file = os.path.join(d, "runtime.o")

        subprocess.check_call(args=[
            clang_path,
            "-w",
            "-c",
            runtime_c_file,
            "-o",
            runtime_o_file,
        ])

        subprocess.check_call(args=[calcc_path, expr, "-o", expr_ll_file])
        subprocess.check_call(args=[
            llc_path,
            "--filetype=obj",
            expr_ll_file,
            "-o",
            expr_o_file,
        ])

        out = "a.out" if args.output is None else args.output

        subprocess.check_call(args=[
            clang_path,
            expr_o_file,
            runtime_o_file,
            "-lc",
            "-lm",
            "-o",
            out,
        ])
