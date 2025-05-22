# SPDX-FileCopyrightText: 2021 Roel Standaert <roel@abittechnical.com>
#
# SPDX-License-Identifier: GPL-2.0-only

FROM alpine:3.21.3 AS builder

RUN apk add gcc g++ cmake ninja wt-dev

COPY CMakeLists.txt /swedish/
COPY src /swedish/src/
COPY docroot /swedish/docroot/
COPY approot /swedish/approot/

RUN cd /swedish && mkdir build

RUN cd /swedish/build && cmake .. \
    -GNinja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX=/swedish/install-dir

RUN cd /swedish/build && ninja install

FROM alpine:3.21.3

RUN apk add wt

COPY --from=builder /swedish/install-dir /swedish

VOLUME ["/swedish/docroot/puzzles"]

CMD ["/swedish/bin/swedish.wt", "--approot=/swedish/approot", "--docroot=/swedish/docroot;/css,/resources,/puzzles", "--http-listen=0.0.0.0:8002", "--resources-dir=/usr/share/Wt/resources", "-c", "/swedish/config/wt_config.xml"]
