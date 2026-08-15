project('abscom', 'c',
  version: '0.1.0',
  license: 'MIT',
  default_options: [
    'c_std=gnu11',
    'warning_level=3',
  ])

inc = include_directories('include')

cc = meson.get_compiler('c')
ws_deps = []
platform_deps = []
if ['windows', 'cygwin'].contains(host_machine.system())
  ws_deps += cc.find_library('ws2_32')
else
  platform_deps += cc.find_library('m', required: false)
  platform_deps += cc.find_library('pthread', required: false)
endif

srcs = files(
  'src/abs_dynarray.c',
  'src/abs_string.c',
  'src/abs_hash.c',
  'src/abs_hashmap.c',
  'src/abs_time.c',
  'src/abs_fs.c',
  'src/abs_matrix.c',
  'src/abs_stats.c',
  'src/abs_csv.c',
  'src/abs_path.c',
  'src/abs_thread.c',
  'src/abs.c',
)

abscom_lib = both_libraries('abscom', srcs,
  include_directories: inc,
  dependencies: ws_deps + platform_deps,
  c_args: ['-DABS_BUILDING_LIBRARY'],
  version: meson.project_version(),
  install: true)

abscom_dep = declare_dependency(
  include_directories: inc,
  link_with: abscom_lib.get_static_lib(),
  dependencies: ws_deps + platform_deps,
)

install_headers(
  'include/abscom/abs.h',
  'include/abscom/abs_common.h',
  'include/abscom/abs_dynarray.h',
  'include/abscom/abs_string.h',
  'include/abscom/abs_hash.h',
  'include/abscom/abs_hashmap.h',
  'include/abscom/abs_time.h',
  'include/abscom/abs_fs.h',
  subdir: 'abscom',
)

pkg = import('pkgconfig')
pkg.generate(abscom_lib,
  description: 'A C11 library of reusable data structures and a Python-inspired dynamic runtime',
  filebase: 'abscom',
  version: meson.project_version(),
  install_dir: get_option('libdir') / 'pkgconfig',
)

subdir('tests')
subdir('examples')
