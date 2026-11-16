# completable
completable provides the matchmaker dictionary as an ncurses terminal application<br/>

for information about matchmaker see https://github.com/shtroizel/matchmaker
## clone and initialize
```
git clone https://github.com/shtroizel/completable.git
cd completable
git submodule update --init --recursive
```
## build and install matchmaker + completable; run completable
### options for building matchmaker
```
matchmaker/scripts/build_and_install.py -h
```
### options for building completable
```
scripts/build_and_install.py -h
```
### EXAMPLE: build q only (quick)
```
matchmaker/scripts/build_and_install.py -q
scripts/build_and_install.py -q -i ../install_q
install_q/bin/completable
```
### EXAMPLE: entire dictionary
```
matchmaker/scripts/build_and_install.py
scripts/build_and_install.py
install/bin/completable
```
## shell mode without curses
Press `$` in completable to open its shell, or skip curses with these options (`-h` lists them all):
```
install_q/bin/completable -c ':books' -c 'run'       # run commands and exit
printf ':len\nrun\n' | install_q/bin/completable -s   # read commands from stdin
install_q/bin/completable -s                         # start in the shell; :curses and $ switch
install_q/bin/completable -n -c 'run'                # leave timing out, for output that diffs cleanly
```
## tests
```
cd <build dir> && ctest
```
The curses tests drive completable through a pseudo terminal and need the Python `pyte` module
(they're skipped without it). Pick the Python with `-DPython3_EXECUTABLE=<python>`. Builds with
`matchmaker_DL=ON` run the shell and library tests only with
`-Dcompletable_test_library=<path to libmatchmaker.so>`.
With ThreadSanitizer builds, run `setarch $(uname -m) -R ctest`, since TSan fails with full address space
randomization on newer kernels.
