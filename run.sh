#!/usr/bin/env bash

pushd build
cmake ..
make
popd
build/gun -p ws://localhost:3030/gun
