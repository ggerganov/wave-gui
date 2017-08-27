# wave-gui
Yet another data-over-sound tool

<a href="http://www.youtube.com/watch?feature=player_embedded&v=m58g1lhoQWg" target="_blank"><img src="http://img.youtube.com/vi/m58g1lhoQWg/0.jpg" alt="CG++ Data over sound" width="360" height="270" border="10" /></a>

## Building & running

    git clone https://github.com/ggerganov/wave-gui
    cd wave-gui
    git submodule update --init
    mkdir build && cd build && cmake ..
    make
    ./main/wave-gui
    
## Dependencies

- [GLFW3](http://www.glfw.org)
- [FFTW](http://www.fftw.org)
- [PortAudio](http://www.portaudio.com)
