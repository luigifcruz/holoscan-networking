project('holoscan-networking', ['cpp'],
  version: '1.0.0',
  default_options: [
    'cpp_std=c++17',
    'warning_level=2',
    'buildtype=release'
  ]
)

# Define source files
adv_network_dir = 'operators/advanced_network/advanced_network'

sources = files(
  adv_network_dir / 'common.cpp',
  adv_network_dir / 'manager.cpp',
)

sources += files(
  adv_network_dir / 'dpdk_log.cpp',
  adv_network_dir / 'managers/dpdk/adv_network_dpdk_mgr.cpp',
  adv_network_dir / 'managers/dpdk/adv_network_dpdk_stats.cpp',
)

# Compile definitions
compile_defs = [
  '-DALLOW_EXPERIMENTAL_API',
  '-DDOCA_ALLOW_EXPERIMENTAL_API',
  '-DANO_MGR_DPDK=1',
]

# Include directories
inc_dirs = include_directories('operators/advanced_network')

# Dependencies
deps = [
  dependency(
    'holoscan',
    modules: [
      'holoscan::core',
      'holoscan::logger',
    ],
    required: false,
  ),
  dependency('libdpdk', required: false),
]

# Build library
holoscan_networking_lib = library('holoscan-networking',
  sources,
  include_directories: inc_dirs,
  dependencies: deps,
  cpp_args: compile_defs,
  install: true,
  version: meson.project_version(),
  soversion: '1',
  gnu_symbol_visibility: 'default',
)

# Create dependency object for use by other projects
holoscan_networking_dep = declare_dependency(
  link_with: holoscan_networking_lib,
  include_directories: inc_dirs,
  dependencies: deps,
  compile_args: compile_defs,
)

# Install headers
install_subdir('operators/advanced_network/advanced_network',
  install_dir: get_option('includedir'),
  strip_directory: false,
  exclude_files: ['*.cpp', '*.cu'],
)

# Generate pkg-config file
pkg = import('pkgconfig')
pkg.generate(
  holoscan_networking_lib,
  description: 'NVIDIA Holoscan Advanced Networking Library',
  name: 'holoscan-networking',
  filebase: 'holoscan-networking',
  subdirs: 'advanced_network',
)
