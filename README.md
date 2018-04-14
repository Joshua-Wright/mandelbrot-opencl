# mandelbrot-opencl

An interactive mandelbrot set explorer using OpenCL/OpenGL texture sharing.

## howto
* click and drag to pan
* mouse scrollwheel to zoom
* `p` to pause rendering
* `ESC` to quit
* `r` to reload fractal kernel source from a file named `mandelbrot.cl` in the current directory
* `d` to dump location and zoom information

You can also run it with width and height specified on command line. Default size is 1024x1024.

## credits
* pradeep for his [CLGLInterop examples](https://github.com/9prady9/CLGLInterop)
* [inferno colormap](https://bids.github.io/colormap/)
