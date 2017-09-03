all:
	mkdir -p build && cd build && cmake .. && make

clean:
	cd build && make clean && cd .. && rm -rf build
