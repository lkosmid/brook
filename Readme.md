This repository adds an OpenGL ES 2 backend for the Raspberry Pi on top of the
latest Brook release from the official Brook repository (https://sourceforge.net/projects/brook).

Installation

   * A quick start guide for getting Brook up and running on Windows
     platforms is provided in QUICK_START.txt. For Linux and Mac OS X
     it's as simple as typing 'make'. For Raspberry Pi and other non-x86 Linux
     platforms follow the steps in QUICK_START_RaspberryPi.txt

Execution

   * In addition, the environment variable BRT_RUNTIME must be set
     to indicate which runtime should be used for running applications.
     Valid settings are:

        * "cpu" to use the CPU runtime
        * "gles" to use the OpenGL ES 2 runtime
        * "dx9" to use the DirectX 9 runtime
        * "ogl" to use the OpenGL runtime
        * "ctm" to use the AMD CTM runtime

     On the Raspberry Pi only the CPU and OpenGL ES 2 runtimes are available.
     
     Setting BRT_ADAPTER chooses by index which adapter to use.

   * Developing Brook applications requires a built BRCC compiler and
     Brook runtime static library, as well as the command-line shader
     compilers cgc and (on Windows) fxc.

        * The minimum version of cgc is 1.3b2, although the latest 3.1.0013 is recommended.
          The Cg Toolkit (including cgc) can be obtained from:
          (http://developer.nvidia.com/object/cg_toolkit.html)
	  See QUICK_START_RaspberryPi.txt about how to install it and use it on
          non-x86 platforms, like the ARM-based Raspberry Pi.

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

   * In order to build BRCC and the runtime on Linux platforms, you will
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
   be found in prog/tests. 

   A small number of larger, more interesting programs reside in
   prog/apps and demonstrate how the various features of the Brook
   language can be used to perform useful work.      

For more information on the original implementation of Brook, read the Brook
GPU project pages at: http://graphics.stanford.edu/projects/brookgpu/

For information about the OpenGL ES 2 backend you can take a look in the
following publications:

   * Matina Maria Trompouki, Leonidas Kosmidis, Brook Auto: High-Level Certification-Friendly Programming for GPU-powered Automotive Systems, DAC '18, Proceedings of the 55h Annual Design Automation Conference 2018 [\[ACM\]](https://dl.acm.org/citation.cfm?id=3196002) [\[pre-print\]](https://upcommons.upc.edu/handle/2117/116182?locale-attribute=en)
   * Matina Maria Trompouki, Leonidas Kosmidis, Brook GLES Pi: Democratising Accelerator Programming, HPG '18 Proceedings of the Conference on High-Performance Graphics 2018 [\[ACM\]](https://dl.acm.org/citation.cfm?id=3231582) [\[pre-print\]](https://upcommons.upc.edu/handle/2117/125336?locale-attribute=en)

In the case you use our OpenGL ES 2 Brook implementation for a publication, we would appreciate a citation to the above publications.
