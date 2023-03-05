# Acceptance $\times$ Efficiency Tool

## Calculate (inclusive) $\alpha\times\epsilon$

Runs with 
```
./run.sh <config.yaml>
```

Clean with
```
make clean
```

## Dependencies

This tool need the [yaml-cpp](https://github.com/jbeder/yaml-cpp#how-to-build) package to work.
I assume the installation will happen in ```~/local/```
Otherwise, please edit the LOCAL_INCLUDE and LOCAL_LIB variables in the makefile.

```
git clone https://github.com/jbeder/yaml-cpp.git
cd yaml-cpp
mkdir build
cd build
cmake --DCMAKE_INSTALL_PREFIX=~/local/ ..
make && make install
# clean build directory
cd ..
rm -r build
```

If you want you can ```rm -rf yaml-cpp``` as well.