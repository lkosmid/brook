Todo before release:
-=-=-=-=-=-=-=-=-=-=
   * Move Addressable<> usage into inner loop static function (MSVC
     optimisation bug)
   * Fix tree build errors
   * Check it definitely works on Vista
   * Glowcode & valgrind everything
   * Harmonise the MSVC project files

Brook GPU v0.5 beta 1, Supercomputing for Home PC's!
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

(Last Revised 24th November 2007)


Installation

   * A quick start guide for getting Brook up and running on Windows
     platforms is provided in QUICK_START.txt. For Linux and Mac OS X
     it's as simple as typing 'make'.


BETA release warnings!

   * This is a beta one release - that means it's early, it's buggy
     and has lots of known issues:
     1. Yes, we know the entire tree doesn't build without error yet
        using make. make brcc, make runtime and make regression all
        SHOULD work unless there's something wrong with your system.
        Other stuff is still being upgraded to use the new features.
     2. Some of the tests are known to fail on the GLSL OpenGL backend
        due to bugs in NVidia's Cg compiler output.
     3. The MSVC project files are in no consistent way between MSVC
        versions. Only the _80 variants are likely to work at all.
     4. Brook's runtime probably leaks memory and resources all over
        the place currently. A full cleanup via Glowcode (on Windows)
        and valgrind (on Linux) will be performed soon.

   * We'd ESPECIALLY like help on the following:
     1. Multi backend usage, in PARTICULAR driving multiple GPU's.

        This is known to be broken for OpenGL on Windows currently for all
        ATI cards due to a driver bug disabling all OpenGL acceleration
        if there is more than one display attached to the current desktop.
        The DX9 backend works well for parallel multi-backend usage.

        This is also known to be broken on Linux currently for all
        ATI cards due to a driver bug preventing the use of more than
        one ATI card at once in X11.

        Furthermore, on Linux ATI drivers currently appear incapable of
        permitting multiple OpenGL contexts to be used simultaneously
        from multiple threads.

     2. Patches fixing up the regression failures on OpenGL (both Linux
        and Windows)


Hardware Prerequisites

   * Any card marketed as supporting Shader Model v3.0 or later by
     either ATI or NVidia should provide all available facilities
     not including integers, bitfields and double-precision floats.

   * Support for Intel SM3.0 or later GPU's is untested. Patches are
     most welcome!

   * Past this, any OpenGL v1.3 or later and GLSL supporting GPU should
     work (apart from driver & GLSL compiler bugs). You need quite a
     few ARB extensions too, but if your driver will do GLSL you will
     almost certainly have all needed extensions.

   * The DirectX 9 runtime will work on ATI r3xx and r4xx video cards
     such as the Radeon 9800 and Radeon X800, as well as NVIDIA nv4x
     cards such as the GeForce 6800. Some nv3x (GeForce FX and
     Quadro FX) cards may also work, but have not been extensively tested.

   * The CPU runtime should work on any platform for which the build
     environment works. For x86 and x64, SSE support is turned ON by
     default - it can be disabled by setting the macro BRT_USE_SSE to 0.
     Alternatively, setting it to 4 enables SSE4 support.


Software Prerequisites

   * Running Brook applications requires up-to-date drivers for your
     video card, as well as the latest version of DirectX 9 (if you
     plan to use the DirectX Brook runtime).

        * Drivers for ATI video cards can be found at:
          (http://www.ati.com/support/driver.html)

        * Drivers for NVIDIA video cards can be found at:
          (http://www.nvidia.com/content/drivers/drivers.asp)

     In addition, the environment variable BRT_RUNTIME must be set
     to indicate which runtime should be used for running applications.
     Valid settings are:

        * "cpu" to use the CPU runtime

        * "dx9" to use the DirectX 9 runtime

        * "ogl" to use the OpenGL runtime
        
        * "ctm" to use the AMD CTM runtime
     
     Setting BRT_ADAPTER chooses by index which adapter to use.

   * Developing Brook applications requires a built BRCC compiler and
     Brook runtime static library, as well as the command-line shader
     compilers cgc and (on Windows) fxc.

        * The minimum version of cgc is 1.3b2. The Cg Toolkit (including
          cgc) can be obtained from:
          (http://developer.nvidia.com/object/cg_toolkit.html)

        * The required version of fxc is that included with the "Summer 2004"
          update of the DirectX 9 SDK, available at:
          (http://msdn.microsoft.com/library/default.asp?url=/downloads/list/directx.asp)
          The fxc compiler is in the Utilties/ directory of the SDK install.

     Both of these tools must be in your PATH environment variable in order
     to be available to the BRCC compiler. In addition, your build system
     must have the 'include' directory of the brook distribution in your
     system include path.

   * In general the BRCC compiler can be invoked on a Brook file foo.br as:
     
     brcc foo.br
     
     This will generate a file foo.cpp that can then be compiled with your
     favorite C++ compiler. A few additional options to BRCC that may
     be useful are:

     -h         print a help message listing command-line options
     -k         keep generated Cg/HLSL fragment program code (in foo.cg)
     -y         generate code that assumes 4-output hardware support
     -o prefix  prepend a prefix to all output file names
     -p shader  generate code for specified shader architecture
     -f cgc|fxc favor a particular compiler when both can generate code
     -a arch    assume a particular GPU when generating code

     The '-p shader' can appear multiple times to specify multiple
     shader targets (if no -p is used, all targets are generated). Valid
     shader formats are listed by running the program with -h.

     By default BRCC generates arb code by translating ps20 code created
     by fxc (because fxc generates more compatible code for ATI cards).
     As an example of using the various flags, to compile foo.br to use
     ARB fragment program on a GeForce 6800, one would execute:

     brcc -p arb -f cgc -a 6800 foo.br

   * In order to build BRCC and the Brook runtime on Windows using Cygwin
     you will need:

        * A standard cygwin install with the following additional packages:
          - Devel / Make
          - Devel / Bison
          - Devel / Flex
          - Interpreters / Perl

        * The Microsoft command-line compiler cl.exe and linker
          link.exe available with Visual Studio 7.1, or as a free
          download. These tools must be in your PATH (available from
          the command line). The versions from Visual Studio 6.0 or
          7.0 WILL NOT WORK.

        * Windows platform headers and libs. These are included with
          Microsoft Visual Studio, and are also available for free
          download as the Windows Platform SDK. The appropriate include
          and lib directories must be in your INCLUDE and LIB
          environment variables, respectively.

        * The Microsoft DirectX 9 SDK must be installed (details above)
          and it's Include/ and Lib/ directories must be added to the
          INCLUDE and LIB environment variables.

     With these tools installed you should be able to build BRCC, the runtime
     and the applications by executing 'make' at the top level of the Brook
     distribution.

   * In order to build BRCC and the runtime with Microsoft Visual Studio 7.1
     you will need:

        * A standard install of Visual Studio 7.1, including the C++
          development tools and platform SDK. MSVC 7.0 or earlier is no good.

        * An installation of the Microsoft DirectX 9 SDK, as detailed above.
          The installer should automatically add the appropriate headers
          and libs to Visual Studio's paths.

        * Flex and Bison must be installed (typically as part of a Cygwin
          installation, as detailed abovie) and must be added to the
          Visual Studio paths for executable commands.

     Once these prerequisites are met, it should be possible to open brook.sln
     at the top level of the repository and build both BRCC and the runtime.

   * In order to build BRCC and the runtime on other platforms, you will
     need the standard GNU toolchain including:
        - g++ and gcc
        - flex
        - bison
        - bin-utils
        - perl (for our dependency-tracking script)

     With these tools installed the system can be built by executing 'make'
     at the top level of the Brook distribution.

Building and Running Example Brook Programs

   This distribution includeds various programs that test the Brook
   compiler and runtime infrastructure. Various regression tests can
   be found in prog/tests, and on command-line build configurations
   the regression can be run by executing 'make regression'.

   A small number of larger, more interesting programs reside in
   prog/apps and demonstrate how the various features of the Brook
   language can be used to perform useful work.      

For more information, read the Brook GPU project pages at:
	
	http://graphics.stanford.edu/projects/brookgpu/

