# afdtd
Accelerated FDTD acoustic simulation

Included are several implementations of FDTD acoustic simulation:
- FDTD - Vanilla FDTD without acceleration
- FDTD_TBB - Parallel implementation using Intel's Thread Building Blocks (TBB)
- FDTD_1EKBESTF - Prioritized computation implementation based on my research (published at DAFx2014)
- FDTD_1EKBEST and others - Alternative versions of the prioritized computation implementation to test particular implementation details
