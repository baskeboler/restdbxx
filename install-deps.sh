#!/usr/bin/env sh

export CC=gcc-5
export CXX=g++-5
export EXTRA_CXXFLAGS="-Wno-format -Wno-format-truncation"

# rocksdb
sudo apt-get install -yq \
    libgflags-dev \
    libsnappy-dev \
    zlib1g-dev \
    libbz2-dev \
    liblz4-dev \

git clone --depth 1 https://github.com/facebook/rocksdb
cd rocksdb
OPTS="-DTRAVIS" make static_lib -j4
sudo make install
cd ..


# folly, wangle and proxygen
git clone --depth 1 https://github.com/facebook/proxygen
cd proxygen/proxygen
./deps.sh

