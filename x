#!/usr/bin/env bash -euE
#
# Exectutes commands via Docker from this directory
# Not used by the `Makefile`

# docker build -t compilerbook https://www.sigbus.info/compilerbook/Dockerfile

cd "$(dirname "$0")"
docker run --rm -it -v "$(pwd):/cinc" -w /cinc compilerbook "$@"

