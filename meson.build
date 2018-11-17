# global project def
project('alibc/extensions', 'c')


# project setup
if get_option('buildtype') == 'debug'
    # make a debug build target!
    add_project_arguments('-DDEBUG=1 -ggdb3', language: 'c')
endif

# headers
includes    = include_directories('include')


# shared libraries
sl_dynabuf  = shared_library('dynabuf', 'lib/dynabuf.c',
                              include_directories: includes)

sl_array    = shared_library('array', 'lib/array.c',
                              include_directories: includes,
                              link_with: sl_dynabuf)

sl_bitmap   = shared_library('bitmap', 'lib/bitmap.c',
                             include_directories: includes,
                             link_with: sl_dynabuf)

sl_hashmap  = shared_library('hashmap', 'lib/hashmap.c',
                              include_directories: includes,
                              link_with: [sl_dynabuf, sl_bitmap])

sl_hashmap_defaults = shared_library('hashmap_defaults',
                                     'lib/hashmap_defaults.c')

# unit test targets
ext_criterion       = dependency('criterion')
if ext_criterion.found()
    exe_array_test      = executable('arraytest', 'tests/test_array.c',
                            link_with: sl_array, dependencies: ext_criterion)
    exe_hashmap_test    = executable('hashtest', 'tests/test_hashmap.c',
                           link_with: [sl_hashmap, sl_hashmap_defaults],
                           dependencies: ext_criterion)

    exe_bitmap_test     = executable('bitmaptest', 'tests/test_bitmap.c',
                            link_with: sl_bitmap, dependencies: ext_criterion)

    # test run targets
    test('array_test', exe_array_test)
    test('hash_test', exe_hashmap_test)
    test('bitmap_test', exe_bitmap_test)
endif
