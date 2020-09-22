# completable
completable provides the matchmaker dictionary as a terminal application<br/>

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
scripts/build_and_install.py -q
install_q/bin/completable
```
### EXAMPLE: entire dictionary
```
matchmaker/scripts/build_and_install.py
scripts/build_and_install.py
install/bin/completable
```
