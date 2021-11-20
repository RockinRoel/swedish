# SPDX-FileCopyrightText: 2021 Roel Standaert <roel@abittechnical.com>
#
# SPDX-License-Identifier: GPL-2.0-only

FROM alpine:latest AS builder

RUN apk add git gcc g++ cmake ninja boost-dev graphicsmagick-dev postgresql-dev zlib-dev

RUN cd / && git clone --depth 1 https://github.com/emweb/wt.git -b bootstrap5

RUN mkdir /wt/build && cd /wt/build && cmake .. \
    -GNinja \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DWT_WRASTERIMAGE_IMPLEMENTATION=GraphicsMagick \
    -DENABLE_SQLITE=OFF \
    -DENABLE_POSTGRES=ON \
    -DBUILD_TESTS=OFF \
    -DENABLE_LIBWTTEST=OFF \
    -DCMAKE_INSTALL_PREFIX=/wt/install-dir

RUN cd /wt/build && ninja install

COPY CMakeLists.txt /swedish/
COPY src /swedish/src/
COPY docroot /swedish/docroot/
COPY approot /swedish/approot/

RUN cd /swedish && mkdir build

RUN cd /swedish/build && cmake .. \
    -GNinja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX=/swedish/install-dir \
    -DWt_DIR=/wt/install-dir/lib/cmake/wt

RUN cd /swedish/build && ninja install

FROM alpine

RUN apk add boost-program_options boost-filesystem boost-thread graphicsmagick libpq

COPY --from=builder /swedish/install-dir /swedish
COPY --from=builder /wt/install-dir/lib/libwt*.so.* /swedish/lib/
COPY --from=builder /wt/install-dir/share /swedish/share/

ENV LD_LIBRARY_PATH=/swedish/lib

VOLUME ["/swedish/docroot/puzzles"]

CMD ["/swedish/bin/swedish.wt", "--approot=/swedish/approot", "--docroot=/swedish/docroot", "--http-listen=0.0.0.0:8002", "--resources-dir=/swedish/share/Wt/resources", "-c", "/swedish/config/wt_config.xml"]
