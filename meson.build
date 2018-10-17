# global project def
project('alibc/extensions', 'c')


# project setup
# unit test targets
ext_criterion       = dependency('criterion')
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

sl_hashmap  = shared_library('hashmap', 'lib/hashmap.c',
                              include_directories: includes,
                              link_with: sl_dynabuf)

sl_hashmap_defaults = shared_library('hashmap_defaults',
                                     'lib/hashmap_defaults.c')


# unit test targets
if ext_criterion.found()
    exe_array_test      = executable('arraytest', 'tests/test_array.c',
                            link_with: sl_array, dependencies: ext_criterion)
    exe_hashmap_test    = executable('hashtest', 'tests/test_hashmap.c',
                           link_with: [sl_hashmap, sl_hashmap_defaults])

    # test run targets
    test('array_test', exe_array_test)
    test('hash_test', exe_hashmap_test)
endif
