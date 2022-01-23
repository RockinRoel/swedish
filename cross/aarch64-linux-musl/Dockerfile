# SPDX-FileCopyrightText: 2021 Roel Standaert <roel@abittechnical.com>
#
# SPDX-License-Identifier: GPL-2.0-only

FROM --platform=linux/amd64 alpine:3.15 AS builder

RUN apk add g++ cmake ninja

# Errors from this command are ignored
RUN apk -X http://dl-cdn.alpinelinux.org/alpine/v3.15/main -U --allow-untrusted --root /sysroot --initdb --arch aarch64 add alpine-base || true

# Errors from this command are also ignored
RUN apk -X http://dl-cdn.alpinelinux.org/alpine/v3.15/main -X http://dl-cdn.alpinelinux.org/alpine/v3.15/community --root /sysroot --arch aarch64 add graphicsmagick-dev libpq-dev zlib-dev linux-headers || true

RUN cd /root && \
    wget https://more.musl.cc/10-20210301/x86_64-linux-musl/aarch64-linux-musl-cross.tgz && \
    echo 'c8ee0e7fd58f5ec6811e3cec5fcdd8fc47cb2b49fb50e9d7717696ddb69c812547b5f389558f62dfbf9db7d6ad808a5a515cc466b8ea3e9ab3daeb20ba1adf33  aarch64-linux-musl-cross.tgz' | sha512sum -c && \
    tar xf aarch64-linux-musl-cross.tgz && \
    rm aarch64-linux-musl-cross.tgz

RUN echo 'using gcc : aarch64 : aarch64-linux-musl-g++ ;' >/root/user-config.jam

# We need to build Boost ourselves because of issue #688: https://github.com/boostorg/spirit/issues/688
# 1.75.0 is the last version without the regression
RUN PATH=/root/aarch64-linux-musl-cross/bin:$PATH && \
    mkdir /root/boost && \
    cd /root/boost && \
    wget https://boostorg.jfrog.io/artifactory/main/release/1.75.0/source/boost_1_75_0.tar.bz2 && \
    tar xf boost_1_75_0.tar.bz2 && \
    cd boost_1_75_0 && \
    ./bootstrap.sh && \
    ./b2 toolset=gcc-aarch64 link=shared --prefix=/sysroot/usr/local --with-atomic --with-chrono --with-date_time --with-program_options --with-filesystem --with-thread install && \
    cd .. && \
    rm boost_1_75_0.tar.bz2 && \
    rm -rf boost_1_75_0

COPY cross/aarch64-linux-musl/toolchain.cmake /root/

ARG WT_VERSION=4.6.1

RUN mkdir /root/wt && \
    cd /root/wt && \
    wget https://github.com/emweb/wt/archive/refs/tags/${WT_VERSION}.tar.gz -O wt-${WT_VERSION}.tar.gz && \
    tar xf wt-${WT_VERSION}.tar.gz && \
    mkdir wt-${WT_VERSION}/build && \
    cd wt-${WT_VERSION}/build && \
    cmake .. \
      -GNinja \
      -DCMAKE_TOOLCHAIN_FILE=/root/toolchain.cmake \
      -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DWT_WRASTERIMAGE_IMPLEMENTATION=GraphicsMagick \
      -DENABLE_SQLITE=OFF \
      -DENABLE_POSTGRES=ON \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_TESTS=OFF \
      -DENABLE_LIBWTTEST=OFF \
      -DCMAKE_INSTALL_PREFIX=/sysroot/usr/local && \
   ninja install && \
   cd ../.. && \
   rm -rf wt-${WT_VERSION}

COPY CMakeLists.txt /swedish/
COPY src /swedish/src/
COPY docroot /swedish/docroot/
COPY approot /swedish/approot/

RUN cd /swedish && mkdir build
RUN cd /swedish/build && cmake .. \
    -GNinja \
    -DCMAKE_TOOLCHAIN_FILE=/root/toolchain.cmake \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX=/swedish/install-dir

RUN cd /swedish/build && ninja install

FROM alpine:3.15

RUN apk add graphicsmagick libpq

COPY --from=builder /swedish/install-dir /swedish
COPY --from=builder /sysroot/usr/local/lib/libboost*.so.* /swedish/lib/
COPY --from=builder /sysroot/usr/local/lib/libwt*.so.* /swedish/lib/
COPY --from=builder /sysroot/usr/local/share /swedish/share/

ENV LD_LIBRARY_PATH=/swedish/lib

VOLUME ["/swedish/docroot/puzzles"]

CMD ["/swedish/bin/swedish.wt", "--approot=/swedish/approot", "--docroot=/swedish/docroot;/css,/resources,/puzzles", "--http-listen=0.0.0.0:8002", "--resources-dir=/swedish/share/Wt/resources", "-c", "/swedish/config/wt_config.xml"]
