# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/albert/test/esp/esp-idf/components/bootloader/subproject")
  file(MAKE_DIRECTORY "/home/albert/test/esp/esp-idf/components/bootloader/subproject")
endif()
file(MAKE_DIRECTORY
  "/home/albert/test/esp/weather-box/build/bootloader"
  "/home/albert/test/esp/weather-box/build/bootloader-prefix"
  "/home/albert/test/esp/weather-box/build/bootloader-prefix/tmp"
  "/home/albert/test/esp/weather-box/build/bootloader-prefix/src/bootloader-stamp"
  "/home/albert/test/esp/weather-box/build/bootloader-prefix/src"
  "/home/albert/test/esp/weather-box/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/albert/test/esp/weather-box/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/albert/test/esp/weather-box/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
