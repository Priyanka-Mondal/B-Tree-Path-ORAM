1. g++:

```shell
sudo apt-get update
sudo apt-get install -y autoconf automake build-essential clang curl git libbz2-dev libc++-dev libcrypto++-dev libgflags-dev libgmp-dev libgtest-dev liblz4-dev libsnappy-dev libssl-dev libssl1.0.0 libtool make openssl pkg-config scons unzip yasm zlib1g-dev
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install -y gcc-5 g++-5
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-5 1
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-5 1
```





2. CMake:

```shell
 wget https://cmake.org/files/v3.14/cmake-3.14.0.tar.gz
 tar -xvzf cmake-3.14.0.tar.gz 
 cd cmake-3.14.0/
 ./configure --prefix=/usr/local
 make
 sudo make install
 cd ..
```






3. GRPC:

```shell
git clone --recurse-submodules -b v1.41.0 https://github.com/grpc/grpc
cd grpc
mkdir -p cmake/build
pushd cmake/build
cmake -DgRPC_INSTALL=ON \
    -DgRPC_BUILD_TESTS=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    ../..
make
sudo make install
popd
cd ..
```





4. RocksDB:

```shell
wget https://github.com/facebook/rocksdb/archive/rocksdb-5.7.5.zip
unzip rocksdb-5.7.5.zip
rm rocksdb-5.7.5.zip
cd rocksdb-rocksdb-5.7.5/
make static_lib shared_lib
sudo make install INSTALL_PATH=/usr/local
cd ..
```





5. Boost:

```shell
wget https://boostorg.jfrog.io/artifactory/main/release/1.64.0/source/boost_1_64_0.tar.gz
tar zxf boost_1_64_0.tar.gz
rm boost_1_64_0.tar.gz
cd boost_1_64_0
./bootstrap.sh --prefix=/usr/local
./b2
sudo ./b2 install
cd ..
```





5.DOXYGEN
```shell
sudo apt-get install doxygen
```





6. Relic:

```shell
wget https://github.com/relic-toolkit/relic/archive/94154a8b77e13d7fd24c9081e7fc6cd95956473d.zip -O relic.zip
unzip relic.zip
rm relic.zip
cd relic-94154a8b77e13d7fd24c9081e7fc6cd95956473d
mkdir build
cd build/
cmake ..
make
sudo make install DESTDIR=/usr/local
cd ../..
```




7. Protobuf:

```shell
wget https://github.com/google/protobuf/releases/download/v3.5.1/protobuf-cpp-3.5.1.zip
unzip protobuf-cpp-3.5.1.zip
rm protobuf-cpp-3.5.1.zip
cd protobuf-3.5.1/
./configure --prefix=/usr/local
make
sudo make install
cd ..
```




8. Refresh shared library cache:

```shell
sudo ldconfig
```

9. copy some libraries
```shell
cd usr/lib 
cp libcrypt.a libcrypto.a libcrypt.so libcrypto.so usr/local/lib
```


10. Get and build the code:

```shell
git clone https://github.com/Priyanka-Mondal/BaselinesSSE.git
cd BaselinesSSE.git
```




11. Change the build configuration:

Add the following line to the `SConstruct` file (ex. at line 37):

```python
env.Append(LINKFLAGS = ['-Wl,--copy-dt-needed-entries'])
```

This fixes this error:

```shell
g++ -o horus_debug -Wl,-rpath=/Development/ucsc/Small-Client-SSE/third_party/crypto/library/lib -Wl,-rpath=/Development/ucsc/Small-Client-SSE/third_party/db-parser/library/lib test_horus.o build/horus/AES.o build/horus/AVLTree.o build/horus/Bid.o build/horus/Horus.o build/horus/OMAP.o build/horus/ORAM.o build/horus/PRFORAM.o build/horus/RAMStore.o build/utils/Utilities.o build/utils/logger.o build/utils/rocksdb_wrapper.o build/utils/utils.o -L/usr/local/lib -Lthird_party/crypto/library/lib -Lthird_party/db-parser/library/lib -lcrypto -lsse_crypto -lgrpc++_unsecure -lgrpc -lprotobuf -lpthread -ldl -lsse_dbparser -lrocksdb -lsnappy -lz -lbz2 -llz4 -lcryptopp
/usr/bin/ld: build/horus/AES.o: undefined reference to symbol 'OPENSSL_add_all_algorithms_noconf@@OPENSSL_1.0.0'
//lib/x86_64-linux-gnu/libcrypto.so.1.0.0: error adding symbols: DSO missing from command line
collect2: error: ld returned 1 exit status
scons: *** [horus_debug] Error 1
scons: building terminated because of errors.
```

There should be a better way to fix this, i.e., adding a `-l<libname>` flag, which is done by adding a lib to the `LIBS` env var, but adding ssl (`-lssl`) didn't help and I don't know what will. So this workaround makes the linker use the library when linking, even though the DSO is missing in the linker flags.

12. Build it:

```shell
scons deps
scons
