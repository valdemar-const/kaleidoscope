#!/usr/bin/env bash

find projects -type f \( -name "*.cpp" -o -name "*.hpp" \) -print0 | \
    xargs -0 -n 50 clang-format -i
