# Data transfer module

Sampleapp is an example of how to use protocol in program.

Tools for compiling sampleapp:

- gcc (>=4.9)
- g++ (>=4.9)
- automake 

Makefile has multiple [flags](#flags)

Targets for Makefile:

- clean (cleaning all compiled objects)
- sampleapp (making application for debugging module)
- unittests (making and executing unit tests for data transfer module)
- staticlib (making static library)

Example:
```sh
$ make sampleapp DEVICE=ZIGBEE LOGGING=STD
```

Run sampleapp:
```sh
$ sudo ./bin/sampleapp <path to XBEE pipe>
```

Example:
```sh
$ sudo ./bin/sampleapp /dev/ttyUSB0
```


<a name="flags">Flags for make</a>

- DEVICE
  - ZIGBEE
  - RF24 (it cannot be used now)
- LOGGING
  - STD (logs will be printed to standard output)
  - FILE (logs will be stored in file with name ProtocolLog_yyyy_mm_dd_HH_MM_ss.log)
