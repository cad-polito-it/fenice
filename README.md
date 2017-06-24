Fenice
======

[![License: BSD-2](https://img.shields.io/badge/license-bsd-green.svg)](/LICENSE.md)
![Status: Obsolete](https://img.shields.io/badge/status-obsolete-red.svg)
![Language: C](https://img.shields.io/badge/language-C-blue.svg)
![Gate-level: edf](https://img.shields.io/badge/gate--level-edf-8877cc.svg)
![Version: 3.65](https://img.shields.io/badge/version-3.65-lightgrey.svg)
![](https://www.google-analytics.com/collect?v=1&t=pageview&tid=UA-28094298-5&uid=666&dp=squillero%2Ffenice)

A customizable fault-simulation and gate-level editing library for sequential circuits. *Fenice* implements a fault-parallel, event-driven algorithm vaguely based on *PROOFS* by Thomas M. Niermann, Wu-Tung Cheng & Janak H. Patel (DOI: [10.1109/43.124398](http://dx.doi.org/10.1109/43.124398)), with some tricks copied from *HOPE* by Hyung Ki Lee and Dong Sam Ha (DOI: [10.1109/43.536711](http://dx.doi.org/10.1109/43.536711)). 

The first version dates back to 1994, the second, to 1996, while version 3 was coded in 2000. Fenice v3.65 includes limited support for transient faults and 3-values simulation. *Molokh* is a stand-alone fault simulator, in version 3 it became a mere example for using the library.

The code and all backups of an early version were mysteriously deleted from Politecnico's server, and I recovered spare fragments of the source from the different machines I worked on, hence the name *fenice* ([*phoenix*](https://en.wikipedia.org/wiki/Phoenix_(mythology))) — the library arose back from the ashes of its predecessor. The name *molokh* refers to [Moloch](https://en.wikipedia.org/wiki/Moloch), the Canaanite god usually associated with human sacrifices.

**Notez bien**: The code was heavily tweaked for gcc 2.x on SPARC, and it won't probably compile anymore.

**Copyright © 2000 Giovanni Squillero. All rights reserved.**

Redistribution and use in source and binary forms, with or without modification, are permitted under the terms of the [BSD 2-clause "Simplified" License](/LICENSE.md).
