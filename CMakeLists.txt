cmake_minimum_required(VERSION 3.16)

project(abscom
  VERSION 0.2.6
  DESCRIPTION "A C11 library of reusable data structures and a Python-inspired dynamic runtime"
  LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

include(GNUInstallDirs)

option(ABSCOM_BUILD_STATIC   "Build the static library" ON)
option(ABSCOM_BUILD_SHARED   "Build the shared library" ON)
option(ABSCOM_BUILD_TESTS    "Build the test suite" ON)
option(ABSCOM_BUILD_EXAMPLES "Build the examples" ON)

set(ABSCOM_SOURCES
  src/abs_dynarray.c
  src/abs_string.c
  src/abs_hash.c
  src/abs_hashmap.c
  src/abs_time.c
  src/abs_fs.c
  src/abs_matrix.c
  src/abs_stats.c
  src/abs_complex.c
  src/abs_ml.c
  src/abs_scalar.c
  src/abs_img.c
  src/abs_plot.c
  src/abs_df.c
  src/abs_csv.c
  src/abs_path.c
  src/abs_thread.c
  src/abs_except.c
  src/abs_regex.c
  src/abs_datetime.c
  src/abs_gen.c
  src/abs_encode.c
  src/abs_env.c
  src/abs_server.c
  src/abs_events.c
  src/abs_plugins.c
  src/abs_func.c
  src/abs_introspect.c
  src/abs_itertools.c
  src/abs_sort.c
  src/abs_crypto.c
  src/abs_ws.c
  src/abs_geom.c
  src/abs.c
)

set(ABSCOM_PUBLIC_HEADERS
  include/abscom/abs.h
  include/abscom/abs_common.h
  include/abscom/abs_dynarray.h
  include/abscom/abs_string.h
  include/abscom/abs_hash.h
  include/abscom/abs_hashmap.h
  include/abscom/abs_time.h
  include/abscom/abs_fs.h
)

# Platform libraries: ws2_32 for sockets on Windows, libm and pthread elsewhere.
set(ABSCOM_PLATFORM_LIBS "")
if(WIN32)
  list(APPEND ABSCOM_PLATFORM_LIBS ws2_32)
else()
  find_library(ABSCOM_MATH_LIBRARY NAMES m)
  if(ABSCOM_MATH_LIBRARY)
    list(APPEND ABSCOM_PLATFORM_LIBS ${ABSCOM_MATH_LIBRARY})
  endif()
  find_package(Threads)
  if(Threads_FOUND)
    list(APPEND ABSCOM_PLATFORM_LIBS Threads::Threads)
  endif()
  find_library(ABSCOM_DL_LIBRARY NAMES dl)
  if(ABSCOM_DL_LIBRARY)
    list(APPEND ABSCOM_PLATFORM_LIBS ${ABSCOM_DL_LIBRARY})
  endif()
endif()

# Static library - the primary target used by the tests and examples.
if(ABSCOM_BUILD_STATIC)
  add_library(abscom_static STATIC ${ABSCOM_SOURCES})
  target_include_directories(abscom_static
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  target_compile_definitions(abscom_static PRIVATE ABS_BUILDING_LIBRARY)
  target_link_libraries(abscom_static PUBLIC ${ABSCOM_PLATFORM_LIBS})
  set_target_properties(abscom_static PROPERTIES
    OUTPUT_NAME abscom
    POSITION_INDEPENDENT_CODE ON)
endif()

if(ABSCOM_BUILD_SHARED)
  add_library(abscom_shared SHARED ${ABSCOM_SOURCES})
  target_include_directories(abscom_shared
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  target_compile_definitions(abscom_shared
    PRIVATE ABS_BUILDING_LIBRARY
    PUBLIC  ABS_USE_LIBRARY)
  target_link_libraries(abscom_shared PUBLIC ${ABSCOM_PLATFORM_LIBS})
  set_target_properties(abscom_shared PROPERTIES
    OUTPUT_NAME abscom
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

if(ABSCOM_BUILD_TESTS AND ABSCOM_BUILD_STATIC)
  enable_testing()
  foreach(t IN ITEMS test_dynarray test_string test_hash test_hashmap test_platform test_abs
                   test_matrix test_ml test_stats test_math test_csv test_path test_thread
                   test_regex test_except test_datetime test_gen test_encode test_env
                   test_server test_events test_plugins test_func test_introspect test_itertools
                   test_sort test_crypto test_ws test_ultra test_geom test_macros)
    add_executable(${t} tests/${t}.c)
    target_link_libraries(${t} PRIVATE abscom_static)
    add_test(NAME ${t} COMMAND ${t})
  endforeach()
endif()

if(ABSCOM_BUILD_EXAMPLES AND ABSCOM_BUILD_STATIC)
  foreach(ex IN ITEMS demo py_demo data_demo v6_demo sci_demo lang_demo framework_demo sort_demo crypto_demo ml_demo ml_train_demo math_demo ds_demo ultra_demo geom_demo demo_macros)
    add_executable(${ex} examples/${ex}.c)
    target_link_libraries(${ex} PRIVATE abscom_static)
  endforeach()
endif()

set(ABSCOM_INSTALL_TARGETS "")
if(ABSCOM_BUILD_STATIC)
  list(APPEND ABSCOM_INSTALL_TARGETS abscom_static)
endif()
if(ABSCOM_BUILD_SHARED)
  list(APPEND ABSCOM_INSTALL_TARGETS abscom_shared)
endif()

install(TARGETS ${ABSCOM_INSTALL_TARGETS}
  EXPORT AbscomTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES ${ABSCOM_PUBLIC_HEADERS}
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/abscom)

install(EXPORT AbscomTargets
  NAMESPACE abscom::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Abscom)

# Package config so consumers can use find_package(abscom CONFIG).
include(CMakePackageConfigHelpers)
configure_package_config_file(
  cmake/AbscomConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/AbscomConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Abscom)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/AbscomConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)
install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/AbscomConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/AbscomConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Abscom)

# pkg-config file, mirroring the one the Meson build generates.
configure_file(cmake/abscom.pc.in abscom.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/abscom.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
