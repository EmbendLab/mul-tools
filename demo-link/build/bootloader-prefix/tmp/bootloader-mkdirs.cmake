# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/alt/esp/v5.3.2/esp-idf/components/bootloader/subproject"
  "/home/alt/git_bench/mul-tools/demo-link/build/bootloader"
  "/home/alt/git_bench/mul-tools/demo-link/build/bootloader-prefix"
  "/home/alt/git_bench/mul-tools/demo-link/build/bootloader-prefix/tmp"
  "/home/alt/git_bench/mul-tools/demo-link/build/bootloader-prefix/src/bootloader-stamp"
  "/home/alt/git_bench/mul-tools/demo-link/build/bootloader-prefix/src"
  "/home/alt/git_bench/mul-tools/demo-link/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/alt/git_bench/mul-tools/demo-link/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/alt/git_bench/mul-tools/demo-link/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
