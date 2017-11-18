# afdtd
Accelerated FDTD acoustic simulation

Included are several implementations of FDTD acoustic simulation:
- FDTD - Vanilla FDTD without acceleration
- FDTD_TBB - Parallel implementation using Intel's Thread Building Blocks (TBB)
- FDTD_1EKBESTF - Prioritized computation implementation based on my research (published at DAFx2014)
- FDTD_1EKBEST and others - Alternative versions of the prioritized computation implementation to test particular implementation details

ImageIO is a small BMP file utility.
WavIO is a small WAV file utility.
These are both bare-bones and do not support the full file format specs for BMP and WAV, respectively.

The sbpl files are borrowed from https://github.com/sbpl/sbpl

The software has an ability to read in a kind of execution script that is called a "Run" file here. This file describes how to initialize the simulation, which implementation to use, the input and output file names, etc. The Run file can run multiple simulations in order. They were used to orchestrate the gathering of performance data on the various simulation implementations. The "RunfileGenerator" folder contains some code to generate this kind of complex Run file.
