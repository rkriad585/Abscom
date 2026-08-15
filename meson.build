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
math_deps = []
if ['windows', 'cygwin'].contains(host_machine.system())
  ws_deps += cc.find_library('ws2_32')
else
  math_deps = [cc.find_library('m', required: false)]
endif

srcs = files(
  'src/ac_dynarray.c',
  'src/ac_string.c',
  'src/ac_hash.c',
  'src/ac_hashmap.c',
  'src/ac_time.c',
  'src/ac_fs.c',
  'src/ac_py.c',
)

abscom_lib = both_libraries('abscom', srcs,
  include_directories: inc,
  dependencies: ws_deps + math_deps,
  c_args: ['-DAC_BUILDING_LIBRARY'],
  version: meson.project_version(),
  install: true)

abscom_dep = declare_dependency(
  include_directories: inc,
  link_with: abscom_lib.get_static_lib(),
  dependencies: ws_deps + math_deps,
)

install_headers(
  'include/abscom/ac.h',
  'include/abscom/ac_common.h',
  'include/abscom/ac_dynarray.h',
  'include/abscom/ac_string.h',
  'include/abscom/ac_hash.h',
  'include/abscom/ac_hashmap.h',
  'include/abscom/ac_time.h',
  'include/abscom/ac_fs.h',
  'include/abscom/ac_py.h',
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
