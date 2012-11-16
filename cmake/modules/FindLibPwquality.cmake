#
# Try to find libarchive.
#
# Once done, this will define:
#
#  LIBPWQUALITY_FOUND — System has libpwquality.
#  LIBPWQUALITY_INCLUDE_DIR — libpwquality include directory.
#  LIBPWQUALITY_LIBRARY — Path to the library files of the libpwquality library.
#
# Copyright © 2012, Fabian Kosmale, <0inkane@googlemail.com>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the <organization> nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

include(CheckLibraryExists)

if (LIBPWQUALITY_LIBRARY AND LIBPWQUALITY_INCLUDE_DIR)
  # In cache already.
  set(LIBPWQUALITY_FOUND TRUE)
else (LIBPWQUALITY_INCLUDE_DIR)

  find_path(LIBPWQUALITY_INCLUDE_DIR libpwquality.h
    ${GNUWIN32_DIR}/include
  )

  find_library(LIBPWQUALITY_LIBRARY NAMES libpwquality
    PATHS
    ${GNUWIN32_DIR}/lib
  )

    # Ensure that they are cached.
    set(LIBARCHIVE_INCLUDE_DIR ${LIBPWQUALITY_INCLUDE_DIR} CACHE INTERNAL "libarchive include path")
    set(LIBARCHIVE_LIBRARY ${LIBPWQUALITY_INCLUDE_DIR} CACHE INTERNAL "Libraries needed to use libarchive")

endif (LIBPWQUALITY_LIBRARY AND LIBPWQUALITY_INCLUDE_DIR)
