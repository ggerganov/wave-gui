[![Build Status](https://travis-ci.org/ggerganov/wave-gui.svg?branch=sdl)](https://travis-ci.org/ggerganov/wave-gui?branch=sdl)

# wave-gui
Yet another data-over-sound tool

[**Live demo *(Chrome only)***](https://ggerganov.github.io/jekyll/update/2018/11/17/wave-gui.html)

Videos:

<a href="http://www.youtube.com/watch?feature=player_embedded&v=m58g1lhoQWg" target="_blank"><img src="http://img.youtube.com/vi/m58g1lhoQWg/0.jpg" alt="CG++ Data over sound" width="360" height="270" border="10" /> </a><a href="http://www.youtube.com/watch?feature=player_embedded&v=-c80-B4f4MM" target="_blank"><img src="http://img.youtube.com/vi/-c80-B4f4MM/0.jpg" alt="CG++ Data over sound" width="360" height="270" border="10" /></a>


## Building & running

    git clone https://github.com/ggerganov/wave-gui
    cd wave-gui
    git submodule update --init
    make
    ./build/main/wave-gui

## Dependencies

- [GLFW3](http://www.glfw.org)
- [FFTW](http://www.fftw.org)
- [SDL2](https://www.libsdl.org)
- [ImGui](https://github.com/ocornut/imgui) (git submodule)

## Thanks

Mike Lubinets (https://github.com/mersinvald) for the Reed-Solomon error correction library
