#!/usr/bin/env sh

export CC=gcc-7
export CXX=g++-7
git clone https://github.com/facebook/proxygen
cd proxygen/proxygen
./deps.sh

