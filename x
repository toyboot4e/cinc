#!/usr/bin/env bash -euE

# docker build -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile

cd "$(dirname "$0")"
docker run --rm -v "$(pwd):/cinc" -w /cinc compilerbook $@

