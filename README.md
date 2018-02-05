[![Build Status](https://travis-ci.org/ggerganov/wave-gui.svg?branch=master)](https://travis-ci.org/ggerganov/wave-gui?branch=master)

# wave-gui
Yet another data-over-sound tool

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
- [PortAudio](http://www.portaudio.com)

## Thanks

Mike Lubinets (https://github.com/mersinvald) for the Reed-Solomon error correction library
