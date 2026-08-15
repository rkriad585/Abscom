test_dep = declare_dependency(
  include_directories: inc,
  link_with: abscom_lib.get_static_lib(),
  dependencies: ws_deps + platform_deps,
)

foreach t : ['test_dynarray', 'test_string', 'test_hash', 'test_hashmap', 'test_platform', 'test_abs',
             'test_matrix', 'test_stats', 'test_csv', 'test_path', 'test_thread',
             'test_regex', 'test_except', 'test_datetime', 'test_gen', 'test_encode', 'test_env',
             'test_server', 'test_events', 'test_plugins', 'test_func', 'test_introspect', 'test_itertools',
             'test_sort', 'test_crypto', 'test_ws']
  exe = executable(t, t + '.c', dependencies: test_dep)
  test(t, exe)
endforeach
