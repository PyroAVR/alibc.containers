# global project def
project('alibc.extensions', 'c')

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


# unit tests
executable('arraytest', 'test_array.c', link_with: sl_array)
executable('hashtest', 'test_hashmap.c',
           link_with: [sl_hashmap, sl_hashmap_defaults])
