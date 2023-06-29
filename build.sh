#!/bin/bash

set -e

conan build . \
    --build=missing \
    --output-folder=build/ \
    -s build_type=Release \
    -s compiler.libcxx=libstdc++11 \
    -o is-wire/*:build_tests=True \
    -o is-wire/*:build_benchmarks=True