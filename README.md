# Acceptance $\times$ Efficiency Tool

## Calculate (inclusive) $\alpha\times\epsilon$

Runs with 
```
./run.sh <config.yaml> [-s] [-v]
```
Where ```-s``` means that no histograms will be saved and ```-v``` will print more information.

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
./run.sh cfg/BToKMuMu_LowQ2_cfg.yaml
```

Output:

```
make: `bin/main.exe' is up to date.
Number of cuts: 2
Number of cuts: 12
BToKMuMu_unbiased
Name                All                 Pass                Efficiency %        Uncertainty %       Tot. Efficiency %   Tot. Uncertainty %
mass_cuts           2928774             758883              25.9113             0.0256022           25.9113             0.0256022
filter_cuts         758883              45158               5.95059             0.0271563           1.54187             0.00719958

BToKMuMu_Filtered
Name                All                 Pass                Efficiency %        Uncertainty %       Tot. Efficiency %   Tot. Uncertainty %
eta_cuts            2340032             1529772             65.374              0.0311023           65.374              0.0311023
pt_cuts             1529772             481161              31.4531             0.0375416           20.5622             0.0264203
reco_cuts           481161              479376              99.629              0.00876441          20.4859             0.0263839
cuts_cuts           479376              150238              31.3403             0.0669984           6.42034             0.0160236
HLT_cuts            150238              139671              92.9665             0.065972            5.96876             0.015487
acc_otherL_cuts     139671              96831               69.3279             0.123388            4.13802             0.0130199
eff_otherL_cuts     96831               67989               70.2141             0.146964            2.90547             0.0109798
acc_foundK_cuts     67989               61026               89.7586             0.116278            2.60791             0.0104183
eff_foundK_cuts     61026               55888               91.5806             0.112405            2.38834             0.00998133
pres_cuts           55888               52885               94.6268             0.0953819           2.26001             0.00971585
```


## Creating histograms

This program offers the possibility to output a root file containing histograms of variable before and after each cut. To enable the possibility add a ```histograms``` node to the YAML file. For example:

```
histograms:
  - name: h_lead_pt
    variable: lead_pt
    nbins: 1000
    xmin: 0.
    xmax: 100.
  - name: h_sublead_pt
    variable: lead_pt
    nbins: 1000
    xmin: 0.
    xmax: 100.
  - name: h_lead_eta
    variable: lead_eta
    nbins: 250
    xmin: -2.5
    xmax: 2.5
```