This application is intended to demonstrate how an application can utilize BRT calls to interactively render a simulation running on the GPU with Brook.

This application runs a simple (i.e. non-physical) fluid surface simulation and renders the result. The user can click and drag in the application window to interact with the simulation.

Notes:

- The kernels used in this application reside in the file simulationKernel.br, and calculate fluid height stepping, smoothing, and normal-map generation.

- The test driver is located in renderWindow.hpp/cpp. This class demonstrates how to manually initialize the brook runtime system, and how to retrieve the underlying texture representation of GPU streams.

- This app only works with the DX9 Brook runtime, and thus currently only supports the Radeon R300 family of accelerators.

- The "fibble" directory contains support code that wraps some of DirectX (in an effort to keep the demo small). Not all features are wrapped, however, so the main application still contains some raw DX code.