# global project def
project('alibc.extensions', 'c')

# ========= INSTALLATION CONTROL =========
# Install compiled libraries?
should_install_libs = false
if get_option('install_libs') == 'not_subproject'
    should_install_libs = not meson.is_subproject()
else
    should_install_libs = (get_option('install_libs') == 'yes')
endif

# Install headers?
should_install_headers = false
if get_option('install_headers') == 'not_subproject'
    should_install_headers = not meson.is_subproject()
else
    should_install_headers = (get_option('install_headers') == 'yes')
endif
# ========= END INSTALLATION CONTROL =========

# ========= BUILDING CONTROL =========
# Build tests?
should_build_tests = false
if get_option('build_tests') == 'yes'
    should_build_tests = true
elif get_option('build_tests') == 'not_subproject'
    should_build_tests = not meson.is_subproject()
endif
# ========= END BUILDING CONTROL =========

# ========= PROJECT VARIABLES =========
# Properties of the C compiler
cc = meson.get_compiler('c')

if get_option('buildtype') == 'debug'
    # enable alibc debug functions and debugging symbols.
    add_project_arguments('-DDEBUG=1',  '-ggdb3', language: 'c')
endif

# headers
includes    = include_directories('include')

# ========= END PROJECT VARIABLES =========

# ========= LIBRARY BUILD TARGETS =========
sl_dynabuf = library(
    'alc_dynabuf', 'lib/dynabuf.c',
    include_directories: includes,
    install: should_install_libs
)

sl_array = library(
    'alc_array', 'lib/array.c',
    include_directories: includes,
    link_with: sl_dynabuf,
    install: should_install_libs
)

sl_bitmap = library(
    'alc_bitmap', 'lib/bitmap.c',
    include_directories: includes,
    link_with: sl_dynabuf,
    install: should_install_libs
)

sl_hashmap = library(
    'alc_hashmap', 'lib/hashmap.c',
    include_directories: includes,
    link_with: [sl_dynabuf, sl_bitmap],
    install: should_install_libs
)

sl_hashmap_defaults = library(
    'alc_hashmap_defaults',
    'lib/hashmap_defaults.c',
    include_directories: includes,
    install: should_install_libs
)

sl_set = library(
    'alc_set', 'lib/set.c',
    include_directories: includes,
    link_with: [sl_bitmap, sl_dynabuf],
    install: should_install_libs
)

sl_iterator = library(
    'alc_iter', 'lib/iterator.c',
    include_directories: includes,
    install: should_install_libs
)

sl_array_iter = library(
    'alc_array_iter', 'lib/array_iterator.c',
    include_directories: includes,
    link_with: [sl_iterator, sl_array],
    install: should_install_libs
)

sl_hashmap_iter = library(
    'alc_hashmap_iter', 'lib/hashmap_iterator.c',
    include_directories: includes,
    link_with: [sl_iterator, sl_hashmap, sl_bitmap, sl_dynabuf],
    install: should_install_libs
)

sl_set_iter = library(
    'alc_set_iter', 'lib/set_iterator.c',
    include_directories: includes,
    link_with: [sl_iterator, sl_set, sl_bitmap, sl_dynabuf],
    install: should_install_libs
)
# ========= END LIBRARY BUILD TARGETS =========

# ========= DEPENDENCY OBJECTS FOR SUPERPROJECT BUILDS =========
dep_dynabuf = declare_dependency(
    include_directories: includes,
    link_with: sl_dynabuf
)

dep_array = declare_dependency(
    include_directories: includes,
    link_with: [sl_array, sl_dynabuf]
)

dep_bitmap = declare_dependency(
    include_directories: includes,
    link_with: [sl_bitmap, sl_dynabuf]
)

dep_hashmap = declare_dependency(
    include_directories: includes,
    link_with: [sl_hashmap, sl_bitmap, sl_dynabuf]
)

dep_hashmap_funcs = declare_dependency(
    include_directories: includes,
    link_with: sl_hashmap_defaults
)

dep_set = declare_dependency(
    include_directories: includes,
    link_with: [sl_set, sl_bitmap, sl_dynabuf]
)

dep_iterator = declare_dependency(
    include_directories: includes,
    link_with: [sl_iterator]
)

dep_array_iter = declare_dependency(
    include_directories: includes,
    link_with: [sl_iterator, sl_array, sl_array_iter]
)

dep_hashmap_iter = declare_dependency(
    include_directories: includes,
    link_with: [sl_iterator, sl_hashmap, sl_bitmap, sl_hashmap_iter, sl_dynabuf]
)

dep_set_iter = declare_dependency(
    include_directories: includes,
    link_with: [sl_iterator, sl_set, sl_bitmap, sl_set_iter, sl_dynabuf]
)
# ========= END DEPENDENCY OBJECTS FOR SUPERPROJECT BUILDS =========

# ========= UNIT TEST BUILD TARGETS =========
ext_cmocka       = dependency('cmocka', required: false)
if ext_cmocka.found() and should_build_tests
    exe_dynabuf_test = executable(
      'dynabuftest', 'tests/test_dynabuf.c',
      include_directories: includes,
      link_with: [sl_dynabuf],
      dependencies: ext_cmocka
    )

    exe_array_test = executable(
        'arraytest', 'tests/test_array.c',
        include_directories: includes,
        link_with: [sl_dynabuf, sl_array, sl_iterator, sl_array_iter],
        dependencies: ext_cmocka
    )

    exe_bitmap_test = executable(
        'bitmaptest', 'tests/test_bitmap.c',
        include_directories: includes,
        link_with: sl_bitmap, dependencies: ext_cmocka
    )

    exe_set_test = executable(
        'settest', 'tests/test_set.c',
        link_with: [
            sl_set, sl_hashmap_defaults, sl_iterator, sl_set_iter
        ],
        include_directories: includes,
        dependencies: ext_cmocka
    )

    exe_hashmap_test = executable(
        'hashtest', 'tests/test_hashmap.c',
        include_directories: includes,
        link_with:
            [
                sl_hashmap, sl_hashmap_defaults,
                sl_iterator, sl_hashmap_iter, 
                sl_bitmap, sl_dynabuf, sl_bitmap
            ],
        dependencies: ext_cmocka
    )

    # test run targets
    test('dynabuf_test', exe_dynabuf_test)
    test('array_test', exe_array_test, args: '-j1')
    test('bitmap_test', exe_bitmap_test)
    test('set_test', exe_set_test)
    test('hash_test', exe_hashmap_test)
endif
# ========= END UNIT TEST BUILD TARGETS =========

# ========= HEADER INSTALLATION CONFIG =========
if should_install_headers
    install_subdir(
        get_option('includedir'),
        strip_directory: true,
        install_dir: join_paths(get_option('prefix'), 'include')
    )
endif
