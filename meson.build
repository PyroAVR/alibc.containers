# global project def
project('alibc.extensions', 'c')

# project constants
_HEADERS = ['include/alibc/extensions/errors.h',
            'include/alibc/extensions/debug.h',
            'include/alibc/extensions/dynabuf.h',
            'include/alibc/extensions/array.h',
            'include/alibc/extensions/hashmap.h',
            'include/alibc/extensions/set.h',
            'include/alibc/extensions/bitmap.h',
            'include/alibc/extensions/debug.h']


# project setup
if get_option('buildtype') == 'debug'
    # make a debug build target!
    add_project_arguments('-DDEBUG=1 -ggdb3', language: 'c')
endif

# headers
includes    = include_directories('include')


# shared libraries
sl_dynabuf  = library('alc_dynabuf', 'lib/dynabuf.c',
                       include_directories: includes,
                       install: true)

sl_array    = library('alc_array', 'lib/array.c',
                       include_directories: includes,
                       link_with: sl_dynabuf,
                       install: true)

sl_bitmap   = library('alc_bitmap', 'lib/bitmap.c',
                      include_directories: includes,
                      link_with: sl_dynabuf,
                      install: true)

sl_hashmap  = library('alc_hashmap', 'lib/hashmap.c',
                              include_directories: includes,
                              link_with: [sl_dynabuf, sl_bitmap],
                              install: true)

sl_hashmap_defaults = library('alc_hashmap_defaults',
                                 'lib/hashmap_defaults.c',
                                 include_directories: includes,
                                 install: true)

sl_set      = library('alc_set', 'lib/set.c',
                             include_directories: includes,
                             link_with: [sl_bitmap, sl_dynabuf],
                             install: true)


# external dependency creation, for subproject builds with Meson
dep_dynabuf = declare_dependency(include_directories: includes,
                                 link_with: sl_dynabuf)

dep_array   = declare_dependency(include_directories: includes,
                                 link_with: [sl_array, sl_dynabuf])

dep_bitmap  = declare_dependency(include_directories: includes,
                                 link_with: [sl_bitmap, sl_dynabuf])

dep_hashmap = declare_dependency(include_directories: includes,
                                 link_with: [sl_hashmap, sl_bitmap, sl_dynabuf])

dep_hashmap_funcs   = declare_dependency(include_directories: includes,
                                         link_with: sl_hashmap_defaults)

dep_set     = declare_dependency(include_directories: includes,
                                 link_with: [sl_set, sl_bitmap, sl_dynabuf])

# unit test targets
ext_criterion       = dependency('criterion')
if ext_criterion.found()
    exe_array_test      = executable('arraytest', 'tests/test_array.c',
                            include_directories: includes,
                            link_with: sl_array, dependencies: ext_criterion)

    exe_hashmap_test    = executable('hashtest', 'tests/test_hashmap.c',
                            include_directories: includes,
                            link_with: [sl_hashmap, sl_hashmap_defaults],
                            dependencies: ext_criterion)

    exe_bitmap_test     = executable('bitmaptest', 'tests/test_bitmap.c',
                            include_directories: includes,
                            link_with: sl_bitmap, dependencies: ext_criterion)

    exe_set_test        = executable('settest', 'tests/test_set.c',
                            link_with: [sl_set, sl_hashmap_defaults],
                            include_directories: includes,
                            dependencies: ext_criterion)

    # test run targets
    test('array_test', exe_array_test, args: '-j1')
    test('hash_test', exe_hashmap_test, args: '-j1')
    test('bitmap_test', exe_bitmap_test, args: '-j1')
    test('set_test', exe_set_test, args: '-j1')
endif

# installation config

# headers
install_headers(_HEADERS)
