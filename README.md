## Installation
```
#install and initialize geant4 with -DGEANT4_USE_OPENGL_X11=ON -DGEANT4_USE_QT=ON

git clone https://github.com/rdom/prttools
git clone https://github.com/rdom/eicdirc
cd eicdirc
mkdir build
cd build
cmake ..
make -j4

#neural network
install tensorflow libs, cppflow and run:
cmake -DAI=1 ..


#test event display
./eicdirc -r 0 -theta 30 -x "pi+" -p 6.0 -w 0 -h 11 -c 2031 -l 3 -v 0 -gz 1 -g 1 -ev 0 -e 1
```


## Synopsis
```
eicdirc [OPTION] [ARGUMENT] ... [OPTION] [ARGUMENT]

example:
./eicdirc -r 0 -theta 30 -x "pi+" -p 6.0 -w 0 -h 11 -c 2031 -l 3 -v 0 -gz 1 -g 1 -ev 0 -e 1
```
## Options
```
-o    output file name
-i    input file name
-u    look-up file name
-pdf  PDFs file name
-nn   path to the neural network

-r    run type
                0    simulation
                1    look-up table generation
                2    geometrical reconstruction
                3    likelihood calculation
		4    create pdf
		5    simulate pdf

-field    field type
                0    no field (default)
                1    CORE b-field
                2    ePIC MARCO 1.7T
                3    ePIC MARCO 2.0T
		4    solenoidal 3.0T

-g    geometry configuration
                0    ATHENA one barbox
                1    ePIC one barbox
		2    CORE one barbox
		10   ATHENA whole DIRC
		11   ePIC whole DIRC
		12   CORE whole DIRC
		
-ev   expansion volume type
	        0    prism with lenses (default)
                1    BaBar wedge with focusing prism
                3    prism with plate, lens between bars and plate
		4    tilted EV
		5    prism with plate, lens between plate and EV

-h    number of bars in one radiator box

-c    MCP layout
                0    4x6 standard MCPs (pixel size == mcp size)
                1    4x6 standard MCPs (6.4x6.4 pixels)
		3    one MCP cover all FD plain (custom pixel size)
		4    2x3 LAPD
		2031    4x6 MCPs with compact packing and 32x32 pixels (2 mm x 2 mm)
		
-l    focusing system
                0    no lens
                1    spherical lens
                3    3-layer spherical lens
                6    3-layer cylindrical lens
                10   ideal lens (thickness = 0, ideal focusing)

-theta    polar between particle beam and bar radiator [deg]
      if theta == 0 then thata = [30,160]

-phi  azimuth angle between particle beam and bar radiator [deg]

-e    number of simulated events

-x    particle type
              "pi+"
              "proton"
              "kaon+"
                 ...
              "opticalphoton"
	      "mix_pie"  1 pion 1 electron mix
	      "mix_pimu" 1 pion 1 muon mix
              "mix_pik"  1 pion 1 kaon mix
	      "mix_pip"  1 pion 1 proton mix
	      "mix_kp"   1 kaon 1 proton mix
	      
-p    particle momentum [GeV/c]

-w    physical list
                0    standard EM
                1    without multiple scattering and bremsstrahlung
		2    standard EM with HAD (elastic/inelastic/absorption)
		3    FTFP_BERT
                10   monochromatic Cherenkov light
                11   10 + 1 

-seed seed number for the random generator 

-b    batch mode
               1    run silent (without GUI)

-d    display option
               use /Prt/geom/drawHits 2 
               0    standard (default)
               1    display hit occupancy of current run
               2    display hit occupancy of occuhits.root (needs to be generated)

-timeres   time resolution [ns]
               0.2  (default)

-timecut   time cut constant [ns]
               0.5  (default)  

-trackingres   tracking resolution [rad], applied at tracking layer
               0.0005  (default)
	       100 use realistic, mapped values

-gx target's X offset

-gz target's Z offset 


```

## LUT generation

```
cd eicdirc/build
mkdir data
./eicdirc -o data/lut.root -r 1 -g 1 -c 2031 -l 3 -v 0 -ev 0 -x "opticalphoton" -p "3.18 eV"  -e 1000000 -b 1
```

Visualization of 100 events:
```
./eicdirc -o data/tmp.root -r 1 -g 1 -c 2031 -l 3 -v 0 -ev 0 -x "opticalphoton" -p "3.18 eV"  -e 100
```
![alt text](https://github.com/rdom/eicdirc/raw/master/pic/eicdirc_lut_gen.png)


LUT averaging:
```
cd eicdirc/macro
root -q -b loadlib.C lutmean.C'("../build/data/lut.root")'
```

## Simulation/Reconstruction:

For geometrical reconstruction one needs to generate ~2000 events (example for 6 GeV/c pion/kaon mix @ 30 degree polar angle):
```
cd eicdirc/build
./eicdirc -r 0 -o data/sim.root -theta 30 -x "mix_pik" -p 6 -w 0 -g 1 -c 2031 -l 3 -trackingres 0.0005 -e 2000 -b 1
```
Geometrical reconstruction requires LUT (lut.avr.root which was previously creaded):
```
./eicdirc -r 2 -i data/sim.root -u data/lut.avr.root -o data/reco.root -trackingres 0.0005 -timeres 0.1 -timecut 0.2 -e 2000 -v 2
```
after first execution it will create per-pmt corrections "sim.root_corr.root" which will be automatically applied during next executions (use same command):
```
./eicdirc -r 2 -i data/sim.root -u data/lut.avr.root -o data/reco.root -trackingres 0.0005 -timeres 0.1 -timecut 0.2 -e 2000 -v 3
```
as a result the program will create figures of relevant parameters (in "data/reco") and store PID information in "data/reco.root"

For time imaging one needs to generate ~25000 events.
```
./eicdirc -r 0 -o data/sim.root -theta 30 -x "mix_pik" -p 6 -w 0 -g 1 -c 2031 -l 3 -trackingres 0.0005 -e 25000 -b 1
```
the last 20k events [5000,20000] will be used to generate Probability Density Functions (needed for time imaging):
```
./eicdirc -r 4 -i data/sim.root -u data/lut.avr.root -trackingres 0.0005 -timeres 0.1 -timecut 0.2 -e 5000
```
it will create "data/sim.pdf.root". Note: the start event number for PDFs (5000) is hardcoded at the moment.

The time imaging reconstruction 
```
./eicdirc -r 2 -i data/sim.root -u data/lut.avr.root -pdf data/sim.pdf.root -o data/reco.root -trackingres 0.0005 -timeres 0.1 -timecut 0.2 -e 5000 -v 3

```



## Example of script usage from macro folder

Hit pattern:

```
cd eicdirc/macro
root loadlib.C draw_hp.C'("../build/data/sim.root")'
```
Example of 1k of 6 GeV/c pions @ 30 degree:
![alt text](https://github.com/rdom/eicdirc/raw/master/pic/hp_pi_1k.png)
