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

1. Do not use CMSSW. This tools depends on RDataFrame. It needs ROOT>6.14. The default lxplus ROOT is great.

2. This tool need the [yaml-cpp](https://github.com/jbeder/yaml-cpp#how-to-build) package to work.
I assume the installation will happen in ```~/local/```
Otherwise, please edit the ```LOCAL_INCLUDE``` and ```LOCAL_LIB``` variables in the makefile.

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

## Example

Run:

```
./run.sh cfg/BToKshortMuMu_cfg.yaml
```
(ignore warnings)

Output:

```
g++ -O2 -Wall `root-config --cflags --libs` -I/afs/cern.ch/user/a/alexandg/local/include -L/afs/cern.ch/user/a/alexandg/local/lib -lyaml-cpp -I./include  -c src/main.cc -o obj/main.o
src/main.cc:28:37: warning: ‘ApplyDefines’ function uses ‘auto’ type specifier without trailing return type [enabled by default]
                   unsigned int i = 0)
                                     ^
src/main.cc:37:37: warning: ‘ApplyFilters’ function uses ‘auto’ type specifier without trailing return type [enabled by default]
                   unsigned int i = 0)
                                     ^
g++  obj/main.o -O2 -Wall `root-config --cflags --libs` -I/afs/cern.ch/user/a/alexandg/local/include -L/afs/cern.ch/user/a/alexandg/local/lib -lyaml-cpp -I./include  -o bin/main.exe
BToKshortMuMu_unbiased
filter_cuts: pass=30892      all=337689     -- eff=9.15 % cumulative eff=9.15 %
BToKshortMuMu_Filtered
filter_cuts: pass=227270     all=228727     -- eff=99.36 % cumulative eff=99.36 %
eta_cuts  : pass=148240     all=227270     -- eff=65.23 % cumulative eff=64.81 %
pt_cuts   : pass=48936      all=148240     -- eff=33.01 % cumulative eff=21.39 %
reco_cuts : pass=48770      all=48936      -- eff=99.66 % cumulative eff=21.32 %
cuts_cuts : pass=13944      all=48770      -- eff=28.59 % cumulative eff=6.10 %
HLT_cuts  : pass=12893      all=13944      -- eff=92.46 % cumulative eff=5.64 %
acc_reco_cuts: pass=7925       all=12893      -- eff=61.47 % cumulative eff=3.46 %
eff_recon_cuts: pass=1119       all=7925       -- eff=14.12 % cumulative eff=0.49 %
pres_cuts : pass=1072       all=1119       -- eff=95.80 % cumulative eff=0.47 %
```

