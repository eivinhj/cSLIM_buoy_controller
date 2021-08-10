# Install script for directory: C:/Users/eivin/ncs/v1.5.1/nrf/lib

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/Zephyr-Kernel")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/masterSource/cSLIM/build/modules/nrf/lib/bin/cmake_install.cmake")
  include("C:/masterSource/cSLIM/build/modules/nrf/lib/nrf_modem_lib/cmake_install.cmake")
  include("C:/masterSource/cSLIM/build/modules/nrf/lib/dk_buttons_and_leds/cmake_install.cmake")
  include("C:/masterSource/cSLIM/build/modules/nrf/lib/at_cmd/cmake_install.cmake")
  include("C:/masterSource/cSLIM/build/modules/nrf/lib/at_notif/cmake_install.cmake")
  include("C:/masterSource/cSLIM/build/modules/nrf/lib/at_host/cmake_install.cmake")
  include("C:/masterSource/cSLIM/build/modules/nrf/lib/at_cmd_parser/cmake_install.cmake")
  include("C:/masterSource/cSLIM/build/modules/nrf/lib/lte_link_control/cmake_install.cmake")
  include("C:/masterSource/cSLIM/build/modules/nrf/lib/flash_patch/cmake_install.cmake")
  include("C:/masterSource/cSLIM/build/modules/nrf/lib/fatal_error/cmake_install.cmake")

endif()

