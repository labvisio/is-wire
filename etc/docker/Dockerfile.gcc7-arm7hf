FROM conanio/gcc7-armv7hf as build

ADD --chown=conan:sudo . /is-wire

RUN cd /is-wire                                                                             \
 && ./bootstrap.sh                                                                          \  
 && conan create . is/stable -s compiler.libcxx=libstdc++11 --build=missing -o shared=False \
 && sudo rm -rf /is-wire
