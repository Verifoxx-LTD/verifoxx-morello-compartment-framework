# VERIFOXX MORELLO COMPARTMENT FRAMEWORK

This is a framework, and example code to exercise it, which makes it easier to compartmentalise a software library running on Morello.
The library should implement one or more well-defined APIs which are used by a front-end executable.

The executable is called the "capability manager" and is responsible for loading the library entity (as a shared object library) into a compartment.  Calls can then be made from the executable into the compartment using a *proxy* for the existing API.  This proxy comprises boiler plate code which can transfer execution into the compartment.  The result is that for the caller, an API method call appears to be processed locally in the capability manager and for the callee the API call appears to have been made locally within the compartment.

Compartments may require access to system services which are not available to compartmentalised code.  To deal with this, the capability manager can provide a *service callback* API which allows a compartment to call back into the capability manager to carry out some action on behalf of the compartment.  This is handled via the same mechanism, whereby boiler-plate code provides a seamless transition between the compartment and capability manager to service the callback function.

Although many aspects are generic CHERI, this framework is designed for Morello.  All compartment code runs in the restricted PE state and all capability manager code runs in the executive PE state.
Furthermore this framework is intended for Morello Aarch64 Linux.  All compartment code is built to a dynamic shared object ELF file which is loaded at runtime by a the capability manager exeuctable using *dlopen()*.
This leads to some complexities because the auxiliary vector for the executable will be used by the loader to resolve any relocations within the shared object file, and hence the compartment will end up with executive permissions.  To resolve this, all symbols are "patched" after loading (this time consuming exercise would be better handled by modifying the loader's library source code).

This has additional restrictions as follows:
- Lazy symbol binding is not possible because all symbols are patched straight after loading
- Any libraries, e.g libc, needed by both executable and compartment library must be loaded twice.  Therefore either the executable is built statically, or if it is built dynamically then *dlmopen()* is used to load the library into a separate namespace (static or dynamic compilation is a build option)

In order to make the framework work properly it is necessary to create some fairly repetitive code for each API function to be called in the compartment and for those which must be called back as a capability service.  Unfortunately due to the PE state transition it is not possible to use RTTI (at least, this has not been solved in the time available for this project) and so a form of static type resolution is used.  Consequently, each method call needs a small wrapper class to manage its data arguments although variadic templates are used where possible to make the proxy API calls work.
This does though mean that the framework is (modern) C++ and not C, and so any code that interacts with it must also be C++ (although the bulk of a compartment library can be in C).

This code was inspired by the exercise of porting WAMR (WebAssembly Micro-Runtime) to CHERI.  WAMR comprises a large codebase with very many API functions, which is designed to be loaded as a libary and used from a thin front-end (either user native code or a WAMR provided example executable).  When examining WAMR compartmentalisation it was relealised there was too much code and too many API functions to individually load them into a compartment and so a generic solution of directly loading the WAMR code into a compartment was needed.
This code builds on some techniques provided in Arm examples for Morello compartmentalisation.

## Support Platforms and Toolchains
The framework supports Morello Aarch64 Purecap only.  Hybrid-cap is not supported.
Gnu toolchain and GlibC must be used at this time.  This is because the Arm LLVM port is designed to use the Morello MUSL libc port and unfortunately MUSL will not work with the runtime symbol resolution patching because:
1. The designers of MUSL could not see a need for implementing dlopen() in a static executable, and therefore this is not supported
2. MUSL does not provide an implementation of dlmopen() which must be used in the case that the capability manager is built dynamically

The code is buildable with LLVM/clang, and so if an alternative C standard library providing dynamic loading capability is available then this could be used.

## Building on Linux
The project uses CMake.
A CMakePresets is provided.  This is intended to use with Microsoft Visual Studio, whereby you can remotely cross-compile for Morello on a WSL2 Ubuntu installation.
Alternatively, you can use the CMakePresets.json directly or provide config flags for use in the CMakeLists.txt.  The following config flags are available:
- CAPMGR_BUILD_STATIC=1|0          		: Whether to build the capability manager executable static, or dynamic (with runtime dependencies).  Static is preferred unless there are dependencies which are only available dynamically.
- MORELLO_PURECAP_LIBS_FOLDER=<path>    : Value to set for *Rpath* for any dynamic shared oject or executable.  On Morello, it is expected the default Linux library paths contain non-purecap aarch64 libraries and therefore the path for purecap flavours should be explicitly set.  The example provided in this repository has a runtime dependency on libc.so and libm.so.

### The Toolchain File on CHERI platforms
The Cmake build can use the toolchain file *toolchain.cmake* to build for CHERI platforms.  You should edit this file accordingly to specify the path to the GCC toolchain.
The default provided assumes that the location is on your path.

To use the toolchain file for CHERI you should set an environment variable CHERI_GNU_TOOLCHAIN_DIR, which is the root of the GNU toolchain.  Or you can provide this directly by passing *-DCHERI_GNU_TOOLCHAIN_DIR=/path/to/gnu/root* as a Cmake argument.

### Building with CMakePresets
Two presets are defined:
- Debug armC64+ Purecap
- Release armC64+ Purecap

Both of these are purcap only; one will include debug symbols in the build.

### Building without CMakePresets
Pass arguments directly to CMake; the following shell commands acheive this:


``` Bash
mkdir build && cd build
cmake .. --toolchain ../toolchain.cmake [-DCHERI_GNU_TOOLCHAIN_DIR=<path>] -DCMAKE_BUILD_TYPE=Debug|Release --install-prefix=<path> \
	[-DCAPMGR_BUILD_STATIC=1|0] [-DMORELLO_PURECAP_LIBS_FOLDER=<path>]

cmake --build .
cmake --install .
```

Where:
- CHERI_GNU_TOOLCHAIN_DIR is path to the morello gnu toolchain root on the build machine, not required if this is specified in an environment variable
- CAPMGR_BUILD_STATIC is 1 for making a static capability manager executable, 0 for requiring .so libs at runtime (default 1)
- MORELLO_PURECAP_LIBS_FOLDER is where to find shared object libraries at runtime on the Morello machine (default "/purecap-lib")

### Bulding on the Morello Target
This document assumes you will be cross-compiling, however you can build on the Morello target itself.
To do this *either* update the *toolchain.cmake* file *or* supply the CHERI_GNU_TOOLCHAIN_DIR flag if the GNU toolchain is not on your path (on the Morello board).
The cmake script will then resolve the correct toolchain binaries, as if the architecure where it is being run is aarch64 then it is assumed to be building on the morello target.
Otherwise, it will cross-compile.

## Building on Windows with Visual Studio and WSL2
The file *CMakePresets.json* is provided to support visual studio C++ CMake remote builds on a Linux Ubuntu machine running under WSL2.

To use VS with CMake and this project "out of the box" you will need to have first carried out the following pre-requisite steps:
1. Enable WSL2 in Windows (10 or 11) and installed a suitable Ubuntu distro
2. Install CMake on the Ubuntu machine, you will need version 3.19 or newer (note: not 100% guaranteed to work with earlier than 3.24!)
3. Set up the Arm Morello GNU Toolchain on your WSL2 distribution for cross-compilation to Morello (i.e linux-x86_64 -> aarch64(+c64))
4. Add the path to the GNU toolchain binaries (gcc, g++ etc.) to your sign-on bash shell path (e.g in .bashrc)
5. Add *export CHERI_GNU_TOOLCHAIN_DIR=/path/to/toolchain/root* to your sign-on bash shell path (e.g in .bashrc)
5. Install Visual Studio 2019 or newer with the C++ for Linux / CMake component

### Launching the project
In Visual Studio, select to open and choose CMake and then locate the *CMakeLists.txt* in the WAMR root folder.  Visual Studio will automatically detect the CMakePresets.json.

You must then select the your Ubuntu machine as the build target.  You can then choose your configuration, the following are available:
- Debug armC64+ PureCap			(internal name "ARMc64-purecap-debug")
- Release armC64+ PureCap		(internal name "ARMc64-purecap-release")

All options are then set up correctly.  Visual Studio will automatically build makefiles via CMake and you can build the codebase.
** NOTE: Modify flags CHERI_GNU_TOOLCHAIN_DIR, CAPMGR_BUILD_STATIC and MORELLO_PURECAP_LIBS_FOLDER as desired in CMakePresets.json **

### Troubleshooting Windows Builds
1. If your project is located on a virtual drive under windows (i.e a subst drive) then you will have to make this available to WSL2.  Although most files are copied to WSL2 via *rsync*, the Ubuntu installation will still need your virtual drive mounted (i.e available as */mnt/drive_letter*).

There are a number of ways to do this, but by far the easiest is to create a symbolic link.  For example, assuming your windows substituted drive is N: which maps to C:\Verifoxx then proceed as follows on Ubunutu:

``` Bash
sudo ln -s /mnt/c/Verifoxx /mnt/n
```

2. You should ensure your .bashrc is able to be executed in "headless" mode i.e when there is no terminal.  The easiest way to achieve this is to add the following line near the top of your .bashrc:

``` Bash
[[ $- != *i* ]] && return
```

3. You are recommended to add the path to the toolchain binaries to your .bashrc file.  *Ensure this is BEFORE the line shown in the no terminal mode, above.*  However it can be supplied by modifying CMakePresets.json.  The problem though with this is it will reduce the portability of the project.


### Debugging on Morello Board with Visual Studio
You can remotely debug from Visual Studio with the target being the Morello board.  However, this requires some setup because the debug target is not the same as the build target.

You can either run gdb on the morello board, or run gdbserver on the morello board and gdb locally.  The former is recommended, and this is the method described below.

#### Prerequisites
1. The Morello board must have a known IP address on the internal network (e.g configure it with a static IP)
2. The Morello board should be running sshd and accept remote connections
3. You may need to enable connections in your windows firewall

#### Visual Studio Setup
1. In Tools -> Options, add a remote connection.  You will need to supply the IP address of the morello board along with the login credentials (user="root", pass="morello").
2. Once a build has succeeded, under Debug configure a launch configuration.  This will open a file *launch.vs.json* which you must then edit to resemble the below:

``` JSON
{
  "version": "0.2.1",
  "defaults": {},
  "configurations": [
    {
      "args": [
      ],
      "cwd": "/purecap-lib",
      "debuggerConfiguration": "gdb",
      "env": {},
      "gdbPath": "/root/morello_gnu/bin/aarch64-none-linux-gnu-gdb",
      "name": "Morello CAPMGR",
      "project": "CMakeLists.txt",
      "projectTarget": "cap-mgr",
      "remoteMachineName": "192.168.0.39",
      "type": "cppgdb",
      "targetArchitecture": "arm64",
      "externalConsole": true,
      "deployDirectory": "/purecap-lib",
      "deployDebugRuntimeLibraries": "true",
      "disableDeploy": false,
      "program": "/purecap-lib/cap-mgr",
      "MIMode": "gdb",
      "miDebuggerServerAddress": "192.168.0.39",
      "deploy": [
        {
          "targetMachine": "192.168.0.39",
          "sourcePath": "${cmake.remoteBuildRoot}/ARMc64-purecap-debug/libcompartment.so",
          "targetPath": "/purecap-lib/libcompartment.so",
          "deploymentType": "RemoteRemote",
          "executable": false
        },
        {
          "targetMachine": "192.168.0.39",
          "sourcePath": "${cmake.remoteBuildRoot}/ARMc64-purecap-debug/cap-mgr",
          "targetPath": "/purecap-lib/cap-mgr",
          "deploymentType": "RemoteRemote",
          "executable": true
        }
      ]
    }
  ]
}
```

Where:
- *args* are command-line arguments to pass to *cap-mgr*, run *cap-mgr --help* for details
- *cwd* is the working folder for debug session
- *debuggerConfiguration* should be gdb
- *gdbPath* is the filepath for the gdb binary.  Version shown above is built for the morello board (i.e runs on morello).
- *name* is the local display name of the debug configuration
- *projectTarget* is *cap-mgr* for the main executable
- *remoteMachineName* is the IP address of the Arm morello board on the local network
- *targetArchitecture* must be an architecture known to visual studio; the nearest we can get is "arm64"
- *externalConsole* should be true to correctly view terminal output in VS Debugger window
- *deployDirectory* is the folder to copy the target to on debugger launch (ideally same as *cwd*)
- *disableDeploy* should be false
- *program* is the main executable to launch, *cap-mgr*
- *miDebuggerServerAddress* should be the Morello IP address
- *deploy* provides a list of files to copy to Morello on debug launch, we copy the executable and the shared object library
- *targetMachine* is the IP of the Morello box
- *targetPath* is the copy destination; for simplicity we put everything in the same folder
- *sourcePath* is the install location after CMake install step, default is shown

You can now proceed to select debug target as per *name*, above, and then debug.  The process will launch remotely on the Morello board connecting across your internal network.
Note that if the executable was built statically you can debug into the compartment library, but if the executable was built dynamically then you will not be able to halt the processor inside the compartment.


#### Debugging and Intellisense Operation
There are limitations to how well intellisense and debugging can work with Visual Studio, because it has to be told the architecture is Arm64 and it doesn't know about Morello.

However, the CMakePresets.json has been specifically set up - and this is why it also needs to use a toolchain file - to make the process as effective as possible:
- C and C++ headers will be picked up from the GNU toolchain on the Linux machine, not the VS local flavour
- C/C++ flags and built-in macro definitions should work in Intellisense, for example __CHERI_PURE_CAPABILITY__ is defined for morello pure-cap.
- Debugging should work fairly seamlessly although you are unlikely to be able to debug into library functions and e.g STL headers (this feature can be enabled/disabled in VS Tools -> Options)

The only real problem is that compiler builtins which are pure-cap specific will likely not work.


## Running the Example
The capability manager executable provided takes command line options as shown:

``` Bash
cap-mgr [--comp-lib=/path/to/libcompartment.so] [-v=0|1|2|3|4] [--dump_tables]
```

Where:
- *--comp-lib* is the pathname of the compartment library to load at runtime.  By default this is *./libcompartment.so*
- *-v=n* is the debug level where 0 is minimal logging and 4 is verbose logging.  Logging is to stdout.  This affects the cap-mgr logging but the setting is also passed to the compartment via an API call exposed by the compartment example.
- *--dump_tables* if provided will dump all compartment ELF relocation tables to stdout, irrespective of the logging level selected

The example, which can be found in *main()* will:
- load the compartment library and perform symbol resolution patching
- call each of the example API methods via the *compartment proxy* which will cause a call into the compartment
- display information on the result of each operations

Note that some of the example API methods involve a service callback to the capability manager during processing.


### Install Location
Performing the install step (e.g "install cap-mgr" from Visual Studio, or *cmake --install* from command-line) will generate:
- <install-dir>/bin/cap-mgr
- <install-dir>/lib/libcompartment.so

By default, <install-dir> will be *${HOME}/install/ARMc64-purecap-debug* or *${HOME}/install/ARMc64-purecap-release*


## Operation and Source Code

### Folders
Folders within the project are:
- compartment/	: Compartment entry point and wrapper to call into a compartment function implementation
- common/	: Code shared between compartment and capability manager
- utils/	: Logger and utility classes for managing capabilities
- capgmr/	: Capability Manager framework files
- capmgr/cap_relocs/	: Responsible for patching relocation tables of loaded libraries
- capmgr/cap_relocs/link_map_internal/	: Access to internal GNU libC structures that are not normally available through the Std C API
- example_usage/    : Files to implement the example compartment API and example service callback API that exercise the framework
- main.cpp	: Example main() which parses command args, loads the compartment library and calls into the example compartment API for demo purposes
- *cmake*   : Build files

### How it Works

TO DO

### How to use with your own APIs

TO DO
