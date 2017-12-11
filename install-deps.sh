#!/usr/bin/env sh

export CC=gcc-7
export CXX=g++-7

# gflags
git clone --depth 1 https://github.com/gflags/gflags
cd gflags
cmake .
make all -j4
sudo make install
cd ..
sudo ldconfig
# rocksdb
sudo apt-get install -yq \
    libsnappy-dev \
    zlib1g-dev \
    libbz2-dev \
    liblz4-dev \
    libzstd-dev

git clone --depth 1 https://github.com/facebook/rocksdb
cd rocksdb
OPTS="-DTRAVIS" make static_lib -j4
sudo make install
cd ..


# folly, wangle and proxygen
git clone --depth 1 https://github.com/facebook/proxygen
cd proxygen/proxygen
./deps.sh

