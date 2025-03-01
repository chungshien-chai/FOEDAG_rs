# -*- mode:cmake -*-

# Copyright 2022 The Foedag team

# GPL License

# Copyright (c) 2022 The Open-Source FPGA Foundation

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.15)
set(subsystem cfgcrypto)
set(test_bin ${subsystem}_test)

project(${subsystem} LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
  set(CRYPTO_LIB ${CFG_BUILD_ROOT_DIR}/FOEDAG/lib/libopenssl_crypto_3.0.9.lib)
  set(SSL_LIB ${CFG_BUILD_ROOT_DIR}/FOEDAG/lib/libopenssl_ssl_3.0.9.lib)
  set(ADDITIONAL_LIB "")
else()
  set(CRYPTO_LIB ${CFG_BUILD_ROOT_DIR}/FOEDAG/lib/libopenssl_crypto_3.0.9.a)
  set(SSL_LIB ${CFG_BUILD_ROOT_DIR}/FOEDAG/lib/libopenssl_ssl_3.0.9.a)
  if (WIN32)
    list(APPEND ADDITIONAL_LIB ws2_32)
  else()
    list(APPEND ADDITIONAL_LIB dl pthread)
  endif()
  set(CMAKE_CXX_FLAGS_DEBUG
  "${CMAKE_CXX_FLAGS_DEBUG} ${TCMALLOC_COMPILE_OPTIONS} -Werror -Wall -O0 -g ${MSYS_COMPILE_OPTIONS} ${MY_CXX_WARNING_FLAGS} ${MEM_SANITIZER_FLAGS}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Werror")
endif()
set(OPENSSL_INC ${CFG_BUILD_ROOT_DIR}/FOEDAG/include)
message("Openssl include ${OPENSSL_INC}")

#################################################################################################################
#
# Two elements will be built
#
# 1. subsystem library (in this case cfgcrypto) - it contains all the core code to help configuration. It will be called by other elements
#
# 2. test_bin executable (in this case cfgcrypto_test exe) - it is external command line entry which eventually call subsystem library (item 1) for unit testing
#
#################################################################################################################

###################
#
# subsystem library
#
###################
add_library(
  ${subsystem} ${CFG_LIB_TYPE}
  CFGOpenSSL.cpp
  CFGCrypto_key.cpp
)
add_dependencies(${subsystem} openssl_build)
target_link_libraries(${subsystem} PRIVATE ${CRYPTO_LIB} ${SSL_LIB} ${ADDITIONAL_LIB})
target_include_directories(${subsystem} PRIVATE ${OPENSSL_INC})

###################
#
# test_bin which also has dependency on its own subsystem library
#
###################
add_executable(
  ${test_bin}
  Test/CFGCrypto_test.cpp
)
target_link_libraries(${test_bin} ${subsystem})
link_directories(${CFG_BUILD_ROOT_DIR}/FOEDAG/lib/)

###################
#
# install 
#   - for header file(s), only put those that we really need to export
#
###################
set(
  SRC_H_INSTALL_LIST
)

set(
  SRC_LIB_INSTALL_LIST
  ${subsystem}
)

foreach(file ${SRC_LIB_INSTALL_LIST})
  install(
    FILES ${CFG_BUILD_ROOT_DIR}/lib/$<TARGET_FILE_NAME:${file}>
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/foedag
  )
endforeach()

foreach(file ${SRC_H_INSTALL_LIST})
  install(
    FILES ${PROJECT_SOURCE_DIR}/${file}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/foedag/CFGCommonRS
  )
endforeach()
