test_dep = declare_dependency(
  include_directories: inc,
  link_with: abscom_lib.get_static_lib(),
  dependencies: ws_deps,
)

foreach t : ['test_dynarray', 'test_string', 'test_hash', 'test_hashmap', 'test_platform', 'test_py']
  exe = executable(t, t + '.c', dependencies: test_dep)
  test(t, exe)
endforeach
