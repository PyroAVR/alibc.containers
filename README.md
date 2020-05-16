# alibc.containers
__Fast, Light, unopinionated data structures in pure C__

## Code Coverage
Meson provides automatic code coverage for test targets, so alibc.containers can
be tested using the standard meson methods.  As an introduction for the
unititiated, the sequence is:

```bash
meson <builddir>
cd <builddir>
meson configure -Db_coverage=true
ninja test # this is just to make sure the tests run.
ninja coverage
```
