# SPDX-FileCopyrightText: 2021 Roel Standaert <roel@abittechnical.com>
#
# SPDX-License-Identifier: GPL-2.0-only

FROM alpine:latest AS builder

RUN apk add gcc g++ cmake ninja graphicsmagick-dev postgresql-dev zlib-dev linux-headers

# We need to build Boost ourselves because of issue #688: https://github.com/boostorg/spirit/issues/688
# 1.75.0 is the last version without the regression
RUN mkdir /boost && \
    cd /boost && \
    wget https://boostorg.jfrog.io/artifactory/main/release/1.75.0/source/boost_1_75_0.tar.bz2 && \
    tar xf boost_1_75_0.tar.bz2 && \
    cd boost_1_75_0 && \
    ./bootstrap.sh && \
    ./b2 link=shared --prefix=/boost/install-dir --with-atomic --with-chrono --with-date_time --with-program_options --with-filesystem --with-thread install && \
    cd .. && \
    rm boost_1_75_0.tar.bz2 && \
    rm -rf boost_1_75_0

ARG WT_VERSION=4.6.1

RUN mkdir /wt && \
    cd /wt && \
    wget https://github.com/emweb/wt/archive/refs/tags/${WT_VERSION}.tar.gz -O wt-${WT_VERSION}.tar.gz && \
    tar xf wt-${WT_VERSION}.tar.gz && \
    mkdir wt-${WT_VERSION}/build && \
    cd wt-${WT_VERSION}/build && \
    cmake .. \
      -GNinja \
      -DCMAKE_CXX_STANDARD=17 \
      -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DWT_WRASTERIMAGE_IMPLEMENTATION=GraphicsMagick \
      -DENABLE_SQLITE=OFF \
      -DENABLE_POSTGRES=ON \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_TESTS=OFF \
      -DENABLE_LIBWTTEST=OFF \
      -DBOOST_PREFIX=/boost/install-dir \
      -DCMAKE_INSTALL_PREFIX=/wt/install-dir && \
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
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX=/swedish/install-dir \
    -DBOOST_ROOT=/boost/install-dir \
    -DWt_DIR=/wt/install-dir/lib/cmake/wt

RUN cd /swedish/build && ninja install

FROM alpine

RUN apk add graphicsmagick libpq

COPY --from=builder /swedish/install-dir /swedish
COPY --from=builder /boost/install-dir/lib/libboost*.so.* /swedish/lib/
COPY --from=builder /wt/install-dir/lib/libwt*.so.* /swedish/lib/
COPY --from=builder /wt/install-dir/share /swedish/share/

ENV LD_LIBRARY_PATH=/swedish/lib

VOLUME ["/swedish/docroot/puzzles"]

CMD ["/swedish/bin/swedish.wt", "--approot=/swedish/approot", "--docroot=/swedish/docroot;/css,/resources,/puzzles", "--http-listen=0.0.0.0:8002", "--resources-dir=/swedish/share/Wt/resources", "-c", "/swedish/config/wt_config.xml"]
