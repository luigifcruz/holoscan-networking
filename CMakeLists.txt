cmake_minimum_required(VERSION 3.24)

project(holoscan-networking LANGUAGES CXX CUDA)

option(HOLOSCAN_NETWORKING_ENABLE_DPDK "Build Advanced Networking DPDK manager sources" OFF)
option(HOLOSCAN_NETWORKING_ENABLE_GPUNETIO "Build Advanced Networking GPUNetIO/DOCA manager sources" OFF)
option(HOLOSCAN_NETWORKING_ENABLE_RIVERMAX "Build Advanced Networking Rivermax manager sources" ON)

set(HOLOSCAN_NETWORKING_CUDA_ARCHITECTURES "80;90" CACHE STRING "CUDA SM architectures to target")

if(NOT HOLOSCAN_NETWORKING_ENABLE_DPDK
   AND NOT HOLOSCAN_NETWORKING_ENABLE_GPUNETIO
   AND NOT HOLOSCAN_NETWORKING_ENABLE_RIVERMAX)
  message(FATAL_ERROR "Enable at least one manager via HOLOSCAN_NETWORKING_ENABLE_* options.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_ARCHITECTURES "${HOLOSCAN_NETWORKING_CUDA_ARCHITECTURES}")

set(ADV_NETWORK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/operators/advanced_network/advanced_network")

set(HOLOSCAN_NETWORKING_SOURCES
    "${ADV_NETWORK_DIR}/common.cpp"
    "${ADV_NETWORK_DIR}/kernels.cu"
    "${ADV_NETWORK_DIR}/manager.cpp"
)

if(HOLOSCAN_NETWORKING_ENABLE_DPDK)
  list(APPEND HOLOSCAN_NETWORKING_SOURCES
    "${ADV_NETWORK_DIR}/managers/dpdk/adv_network_dpdk_mgr.cpp"
    "${ADV_NETWORK_DIR}/managers/dpdk/adv_network_dpdk_stats.cpp"
  )
endif()

if(HOLOSCAN_NETWORKING_ENABLE_GPUNETIO)
  list(APPEND HOLOSCAN_NETWORKING_SOURCES
    "${ADV_NETWORK_DIR}/managers/gpunetio/adv_network_doca_mgr.cpp"
    "${ADV_NETWORK_DIR}/managers/gpunetio/adv_network_doca_mgr_obj.cpp"
    "${ADV_NETWORK_DIR}/managers/gpunetio/adv_network_doca_kernels.cu"
  )
endif()

if(HOLOSCAN_NETWORKING_ENABLE_RIVERMAX)
  list(APPEND HOLOSCAN_NETWORKING_SOURCES
    "${ADV_NETWORK_DIR}/managers/rivermax/rivermax_mgr_impl/adv_network_rivermax_mgr.cpp"
    "${ADV_NETWORK_DIR}/managers/rivermax/rivermax_mgr_impl/rivermax_config_manager.cpp"
    "${ADV_NETWORK_DIR}/managers/rivermax/rivermax_mgr_impl/rivermax_mgr_service.cpp"
    "${ADV_NETWORK_DIR}/managers/rivermax/rivermax_mgr_impl/burst_manager.cpp"
    "${ADV_NETWORK_DIR}/managers/rivermax/rivermax_mgr_impl/rivermax_queue_configs.cpp"
  )
endif()

add_library(holoscan-networking ${HOLOSCAN_NETWORKING_SOURCES})

set_target_properties(holoscan-networking PROPERTIES
  CUDA_SEPARABLE_COMPILATION ON
  CUDA_RESOLVE_DEVICE_SYMBOLS ON
  POSITION_INDEPENDENT_CODE ON
)

target_compile_features(holoscan-networking PUBLIC cxx_std_17)

target_include_directories(holoscan-networking
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/operators/advanced_network"
)

set(HOLOSCAN_NETWORKING_COMPILE_DEFINITIONS
  ALLOW_EXPERIMENTAL_API
  DOCA_ALLOW_EXPERIMENTAL_API
)

if(HOLOSCAN_NETWORKING_ENABLE_DPDK)
  list(APPEND HOLOSCAN_NETWORKING_COMPILE_DEFINITIONS ANO_MGR_DPDK=1)
endif()
if(HOLOSCAN_NETWORKING_ENABLE_GPUNETIO)
  list(APPEND HOLOSCAN_NETWORKING_COMPILE_DEFINITIONS ANO_MGR_GPUNETIO=1)
endif()
if(HOLOSCAN_NETWORKING_ENABLE_RIVERMAX)
  list(APPEND HOLOSCAN_NETWORKING_COMPILE_DEFINITIONS ANO_MGR_RIVERMAX=1)
endif()

target_compile_definitions(holoscan-networking
  PUBLIC ${HOLOSCAN_NETWORKING_COMPILE_DEFINITIONS}
)

find_package(holoscan QUIET CONFIG)
if(holoscan_FOUND)
  target_link_libraries(holoscan-networking PUBLIC holoscan::core)
else()
  message(STATUS "holoscan package not found - link it manually if needed.")
endif()
