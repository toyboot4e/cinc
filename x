#!/usr/bin/env bash -euE

cd "$(dirname "$0")"
docker run --rm -v "$(pwd):/cinc" -w /cinc compilerbook $@

