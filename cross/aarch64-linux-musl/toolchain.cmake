# SPDX-FileCopyrightText: 2022 Roel Standaert <roel@abittechnical.com>
#
# SPDX-License-Identifier: GPL-2.0-only

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_SYSROOT /sysroot)

set(CMAKE_C_COMPILER /root/aarch64-linux-musl-cross/bin/aarch64-linux-musl-gcc)
set(CMAKE_CXX_COMPILER /root/aarch64-linux-musl-cross/bin/aarch64-linux-musl-g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
