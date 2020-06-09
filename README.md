# alibc.containers
__Fast, Light, unopinionated data structures in pure C__

## Dependencies
To build:
 - [Meson](https://mesonbuild.com)
 - A modern C compiler (tested with GCC and Clang)

To test:
 - [Cmocka](https://cmocka.org)

## Building

```
$ meson <builddir>
$ ninja -C <builddir>
# ninja -C <builddir> install
```

Installation is disabled when `alibc.containers` is used as a Meson subproject.
If you want to install anyway, this setting is overridden by running:

```
$ cd <builddir>
$ meson configure -D<subproject_dir_name>:install_libs=yes
$ meson configure -D<subproject_dir_name>:install_headers=yes
```

## Testing
Test targets are enabled by default, but are disabled for convenience when
`alibc.containers` is used in a subproject. To force building/not building the
test suite:

```
$ cd <builddir>
$ meson configure -D<subproject_dir_name>:build_tests=yes/no
```

After building, running the tests is as simple as:

```
$ ninja -C <builddir> test
```



## Code Coverage
Meson provides automatic code coverage for test targets, so alibc.containers can
be tested using the standard meson methods.  As an introduction for the
unititiated, the sequence is:

```bash
$ meson <builddir>
$ cd <builddir>
$ meson configure -Db_coverage=true
$ ninja test # this is just to make sure the tests run.
$ ninja coverage
```
