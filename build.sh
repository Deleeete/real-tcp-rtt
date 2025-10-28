#!/bin/bash
set -e

mkdir -p build

clang server.c -O3 -o build/server
clang client.c -O3 -o build/client
