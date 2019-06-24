# RTspice

A netlist-based realtime audio circuit simulator

## Requirements
* [CMake](https://cmake.org/)
* C++17
* [CUDA](https://developer.nvidia.com/cuda-zone)
*  [Boost.Spirit](https://www.boost.org/)
*  [JACK Audio Connection Kit](http://jackaudio.org/)
* [Catch2](https://github.com/catchorg/Catch2)
* [Qt5](https://www.qt.io/)

## Getting started

I recommend to obtain the dependencies through your distribution's package
manager (Ubuntu has JACK on the default repo and the DBUS version on some PPAs,
arch linux also has most of the dependencies on the default repo, and the rest
can be installed from AUR). The JACK configuration can be tricky if your
distribution depends on PulseAudio, however, the D-Bus version should make
it easier to setup.

Once all dependencies are installed, `git clone --recurse-submodules` this
repository into a directory of your choice, create a build directory
`mkdir build`, configure the project with `cmake .. -DCMAKE_BUILD_TYPE=Release`
and `make` it. Should compilation succeed, you can check that the simulation
works with the compiled tests and run the program with `./rtspice` and you'll
be greeted with the blank entry screen:

[home screen](images/entry_screen.png)

You may then open a netlist file (.net extension), and if the netlist has a
proper syntax, you should see the simulation screen:


[sim screen](images/control_example.png)

Basic information, such as the circuit's name, the number of nonzero elements
in the modified admittance matrix and the number of modified nodes can be seen
top left.

Information regarding JACK processing load, ports and connections lies top right.
Once you've chosen the appropriate input and output connections, press
'Activate' to begin simulation.

For circuits containing parametrized components (i.e. variable resistors,
capacitors or inductors), knobs will be visible on the bottom of the screen.
The 'Log' checkbox switches the knob control pattern from linear to logarithmic.

# How it works?

Traditional circuit simulation programs are based on
[modified nodal analysis](https://en.wikipedia.org/wiki/Modified_nodal_analysis),
where the node voltages and _some_ branch currents are the unknowns in a
(usually sparse) linear system, and can be obtained through regular linear
system solvers.

Circuits that contain dynamic components such as capacitors require modelling
of the dynamic behavior based on some integration method, being reduced to
sources and time-varying resistances. For now, we use
[trapezoidal integration](https://en.wikipedia.org/wiki/Trapezoidal_rule) for
both capacitors and inductors.

Circuits that contain nonlinear components require solving a nonlinear system
of equations, however using the
[Newton-Raphson method](https://en.wikipedia.org/wiki/Newton%27s_method), this
also reduces to solving iterations of linear systems.

Based on this, this program provides an interface for a computer's audio ports
to act as signal sources or measurements for electronic circuits. A difficulty
arises in the needed time to solve said linear systems rapidly enough to keep
up with the sound samples: for instance, a 44.100 kHz sampling rate requires
the solution to be calculated in less than 23 $\mu$s. Considering this, it can
be expected this software to work better with modest-sized circuits, however,
the actual size you'll be able to consistently simulate will depend on your
hardware.


# Netlist syntax

The circuits are described through a netlist, representing which components
should be added and which nodes the components are connected to. Our netlist
syntax is similar but not exactly the same as
[SPICE](http://bwrcs.eecs.berkeley.edu/Classes/IcBook/SPICE/)'s:

* The first line of the netlist is a special comment containing the circuit name:
	* `My little circuit`
* Lines beginning with asterisks(*) are considered comments and are ignored:
	* *`Some witty comment`
* The rest of the lines are considered statements and go towards the circuit definition:
	* `RL 0 1 2.2k`
* Multiline statements can be made prepending the sequenced lines with `+`:
	* `Vin NODE_A NODE_B`<br/>`+EXT IN`

## Components

The currently supported components are listed below:

| Component Type | Syntax | Example | Notes |
|---|---|---|---|
| DC Voltage | `V{ID} {NODE+} {NODE-} DC {VALUE}` | `Vcc 1 0 DC 12` | `VALUE` in Volts |
| Sinusoidal Voltage | `V{ID} {NODE+} {NODE-} SINE {AMPLITUDE} {FREQUENCY} {PHASE}` | `Vac p 0 SINE 120 60 0` | `AMPLITUDE` in Volts, `FREQUENCY` in Hertz and `PHASE` in degrees |
| External Voltage | `V{ID} {NODE+} {NODE-} EXT {INPUT}` | `VIN 1 0 EXT INPUT` | `INPUT` defines the name of an input port to be connected |
| DC Current | `I{ID} {NODE+} {NODE-} DC {VALUE}` | `Ibias b 0 DC 1u` | `VALUE` in Amperes, flows from `NODE+` to `NODE-` |
| Sinusoidal Current | `I{ID} {NODE+} {NODE-} SINE {AMPLITUDE} {FREQUENCY} {PHASE}` | `If p 0 SINE 1n 10k 90` | `AMPLITUDE` in Amperes, `FREQUENCY` in Hertz and `PHASE` in degrees |
| External Current | `I{ID} {NODE+} {NODE-} EXT {INPUT}` | `IIN 1 0 EXT I_INPUT` | `INPUT` defines the name of an input port to be connected |
| Linear Voltage Amplifier | `E{ID} {OUT+} {OUT-} {IN+} {IN-} {Av}` | `E1 0 x 1 0 100` | |
| Linear Current Amplifier | `F{ID} {OUT+} {OUT-} {IN+} {IN-} {Ai}` | `F1 2 9 h u -3`  | |
| Linear Transconductance  | `G{ID} {OUT+} {OUT-} {IN+} {IN-} {Gm}` | `G1 6 0 1 0 10u` |`Gm` in Siemens |
| Linear Transresistance   | `H{ID} {OUT+} {OUT-} {IN+} {IN-} {Rm}` | `H5 8 0 t u -3`  |`Rm` in Ohms |
| Linear Resistor | `R{ID} {NODE_A} {NODE_B} {VALUE}` | `Rload 0 X 22k` | `VALUE` in Ohms |
| Variable Resistor | `R{ID} {NODE_A} {NODE_B} EXT {MAX_VALUE} {PARAM}` | `Rvol OUT 0 EXT 500k Volume` | `MAX_VALUE` in Ohms, `PARAM` defines the name of a knob |
| Linear Capacitor | `C{ID} {NODE_A} {NODE_B} {VALUE}` | `Rbypass 23 A 10u` | `VALUE` in Farads |
| Linear Inductor | `L{ID} {NODE_A} {NODE_B} {VALUE}` | `Lchoke vcc c 10m` | `VALUE` in Henrys |
| Ideal OPAMP | `U{ID} {OUT+} {OUT-} {IN+} {IN-} OPAMP` | `U1 out 0 in out OPAMP` | Usually, `OUT-` should be grounded |
| Basic Diode | `D{ID} {ANODE} {CATHODE} IS={IS} N={N}` | `D1 cut 0 IS=4.3n N=1.9`| `IS` is saturation current in Amperes, `N` is the emission coefficient |
|PROBE | `PROBE {NODE}` | `PROBE OUT`| `PROBE` is how output variables are defined. For each `PROBE`d node, an output port becomes available to the user |

## Inputs, Outputs and Params

The netlist not only describes the circuit, but it also defines where the inputs
are located. For each different `{INPUT}` for external voltages and currents, an
input port will be available to connection, corresponding to that given source.

For potentiometers, each different `{PARAM}` corresponds to a knob that will be
available on the control screen.

For the outputs, the special component `PROBE` marks a node voltage (or branch current)
that will serve as an output port for the system.

# TODO

* Proper potentiometer
  * Currently one can emulate a potentiometer: Rpot X Y Z POT VAL PARAM as:<br/>
    `RpotX X @XY VAL`<br/>
    `RpotY @XY Y EXT -VAL PARAM`<br/>
    `RpotZ Y Z EXT VAL PARAM`<br>

* Subcircuit models (i.e. Ebers-Moll, Compensated OPAMP)
* JFET and MOSFETs
* Nonlinear dynamic components


