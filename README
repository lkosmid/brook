				 Brook GPU

(Last Revised 14 October 2004)


Installation

   * A quick start guide for getting Brook up and running on Windows
     platforms is provided in QUICK_START.txt


Hardware Prerequisites

   * The DirectX 9 runtime will work on ATI r3xx and r4xx video cards
     such as the Radeon 9800 and Radeon X800, as well as NVIDIA nv4x
     cards such as the GeForce 6800. Some nv3x (GeForce FX and
     Quadro FX) cards may also work, but have not been extensively tested.

   * The OpenGL runtime will work on r3xx, r4xx, nv3x and nv4x video
     cards. This runtime is compatible with both Windows and Linux
     targets, although compatibility may not be identical across
     platforms.

   * The CPU runtime should work on any platform for which the build
     environment works.

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

   * Developing Brook applications requires a built BRCC compiler and
     Brook runtime static library, as well as the command-line shader
     compilers cgc and (on Windows) fxc.

        * The required version of cgc is 1.3b2. The Cg Toolkit (including
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
     shader formats include cpu, ps20, ps2a, ps2b, arb, fp30 and fp40.

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
          7.0 may work, but are not officially supported.

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

        * A standard install of Visual Studio 7.1 (7.0 version may work),
          including the C++ development tools and platform SDK.

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


Revision History

12/16/03:  Initial Beta release

12/19/03:  Added text about OS for different backends
           Linux support for nv30gl

10/14/04:  Revised text to reflect upcoming 0.4 release.
