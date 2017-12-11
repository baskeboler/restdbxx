#!/usr/bin/env sh

export CC=gcc-7
export CXX=g++-7

# gflags
git clone https://github.com/gflags/gflags
cd gflags
cmake .
sudo make install -j4
cd ..

# rocksdb
sudo apt-get install -yq \
    libsnappy-dev \
    zlib1g-dev \
    libbz2-dev \
    liblz4-dev \
    libzstd-dev
git clone https://github.com/facebook/rocksdb
cd rocksdb
cmake .
make rocksdb -j4
sudo make install
cd ..


# folly, wangle and proxygen
git clone https://github.com/facebook/proxygen
cd proxygen/proxygen
./deps.sh

