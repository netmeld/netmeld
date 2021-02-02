# =============================================================================
# Copyright 2017 National Technology & Engineering Solutions of Sandia, LLC
# (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# =============================================================================
# Maintained by Sandia National Laboratories <Netmeld@sandia.gov>
# =============================================================================

set(PROJECT_GNU_COMMON_FLAGS
  "-D_FORTIFY_SOURCE=2"
  "-Os"
  "-fdiagnostics-show-option"
  "-pedantic-errors"
  "-Wall"
  "-Wextra"
  "-Wformat=2"
  "-Werror=format-security"
  "-Wno-long-long"
  "-Wfatal-errors"
  "-Wcast-align"
  "-Wcast-qual"
  "-Wconversion"
  "-Wsign-conversion"
  "-Wsign-compare"
  "-Wfloat-equal"
  "-Wlogical-op"
  "-Wmissing-include-dirs"
  "-Wmissing-declarations"
  "-Wredundant-decls"
  #"-Wshadow"
  "-Wswitch-default"
  "-Wswitch-enum"
  "-Wundef"
  "-Wuninitialized"
  "-Winit-self"
  "-Wunreachable-code"
  "-Wwrite-strings"
  #"-Winline"
  "-Wpacked"
  #"-Wpadded"
  "-fstack-protector-all"
  "-Wstack-protector"
  "-ftrapv"
  "-Wl,-z,relro"
  "-Wl,-z,now"
  )

set(PROJECT_GNU_C_FLAGS
  "-std=c17"
  "-Wbad-function-cast"
  "-Wc++-compat"
  "-Wmissing-prototypes"
  "-Wstrict-prototypes"
  "-Wnested-externs"
  "-Wold-style-declaration"
  "-Wold-style-definition"
  "-Wtraditional-conversion"
  )

set(PROJECT_GNU_CXX_FLAGS
    # Following line needed to use boost libraries > 1.62 with cmake
    #"-DBOOST_PHOENIX_NO_VARIADIC_EXPRESSION"
  "-std=c++17"
  #"-Weffc++"
  "-Wnon-virtual-dtor"
  "-Wctor-dtor-privacy"
  "-Wold-style-cast"
  "-Woverloaded-virtual"
  "-Wsign-promo"
  "-Wstrict-null-sentinel"
  #"-Wabi"
  )

set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)

function(encode_compiler_flag_as_token string_value result_var)
  string(STRIP "${string_value}" RESULT)
  string(REPLACE " " "_" RESULT "${RESULT}")
  string(REPLACE "," "_" RESULT "${RESULT}")
  string(REPLACE "." "_" RESULT "${RESULT}")
  string(REPLACE "=" "_" RESULT "${RESULT}")
  string(REPLACE "/" "_" RESULT "${RESULT}")
  string(REPLACE "-" "_" RESULT "${RESULT}")
  string(REPLACE "+" "x" RESULT "${RESULT}")
  set(${result_var} ${RESULT} PARENT_SCOPE)
endfunction()

function(set_pie_flags)
  if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)  # GNU C/C++
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIE -pie -fPIC" PARENT_SCOPE)
  endif()
endfunction()


if(CMAKE_COMPILER_IS_GNUCC)  # GNU C compiler (gcc)
  foreach(LINE ${PROJECT_GNU_ASM_FLAGS})
    set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${LINE}")
  endforeach(LINE)
endif()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)  # GNU C/C++
  set(CMAKE_EXE_LINKER_FLAGS_INIT "${CMAKE_EXE_LINKER_FLAGS_INIT} -fPIE -fPIC")
endif()

if(CMAKE_COMPILER_IS_GNUCC)  # GNU C compiler (gcc)
  foreach(LINE ${PROJECT_GNU_MACHINE_FLAGS})
    # Without checking, add each machine flag to CMAKE_C_FLAGS.
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LINE}")
  endforeach(LINE)
  foreach(LINE ${PROJECT_GNU_COMMON_FLAGS} ${PROJECT_GNU_C_FLAGS})
    # Check whether each requested flag is supported by this gcc.
    encode_compiler_flag_as_token("${LINE}" FLAG)
    check_c_compiler_flag("-Werror ${LINE}" GNUCC_SUPPORTS_FLAG_${FLAG})
    if(GNUCC_SUPPORTS_FLAG_${FLAG})
      # Add each supported flag to CMAKE_C_FLAGS.
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${LINE}")
    endif()
  endforeach(LINE)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)  # GNU C++ compiler (g++)
  foreach(LINE ${PROJECT_GNU_MACHINE_FLAGS})
    # Without checking, add each machine flag to CMAKE_CXX_FLAGS.
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LINE}")
  endforeach(LINE)
  foreach(LINE ${PROJECT_GNU_COMMON_FLAGS} ${PROJECT_GNU_CXX_FLAGS})
    # Check whether each requested flag is supported by this g++.
    encode_compiler_flag_as_token("${LINE}" FLAG)
    check_cxx_compiler_flag("-Werror ${LINE}" GNUCXX_SUPPORTS_FLAG_${FLAG})
    if(GNUCXX_SUPPORTS_FLAG_${FLAG})
      # Add each supported flag to CMAKE_CXX_FLAGS.
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${LINE}")
    endif()
  endforeach(LINE)
endif()

# Print the compiler and linker flags for verification purposes.
message("CMAKE_C_FLAGS   = ${CMAKE_C_FLAGS}")
message("CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")

message("CMAKE_EXE_LINKER_FLAGS = ${CMAKE_EXE_LINKER_FLAGS}")
message("CMAKE_C_COMPILE_OPTIONS_PIE = ${CMAKE_C_COMPILE_OPTIONS_PIE}")
message("CMAKE_CXX_COMPILE_OPTIONS_PIE = ${CMAKE_CXX_COMPILE_OPTIONS_PIE}")
