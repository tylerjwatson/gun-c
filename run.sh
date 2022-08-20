#!/usr/bin/env bash

pushd build
cmake ..
make
popd
build/gun
