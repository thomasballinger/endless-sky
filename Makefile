dev: hello.html
	emrun --serve_after_close --serve_after_exit --browser chrome --private_browsing hello.html
hello.html:
	em++\
		-o hello.html\
		-O3\
		-flto\
		--source-map-base http://localhost:6931/\
		-s WASM_MEM_MAX=2147483648\
		-s INITIAL_MEMORY=838860800\
		-s ALLOW_MEMORY_GROWTH=1\
		-g\
		-s DISABLE_EXCEPTION_CATCHING=0\
		-s ASSERTIONS=2\
		-s DEMANGLE_SUPPORT=1\
		-s EMULATE_FUNCTION_POINTER_CASTS=1\
		-s FETCH=1\
		source/main.cpp
hello-original.html:
	mkdir -p build/emcc
	mkdir -p lib/emcc
	em++ -o build/emcc/main.o\
		-c\
		-O3\
		-flto\
		-g\
		-s DISABLE_EXCEPTION_CATCHING=0\
		-s ASSERTIONS=2\
		-s DEMANGLE_SUPPORT=1\
		-s EMULATE_FUNCTION_POINTER_CASTS=1\
		-s FETCH=1 source/main.cpp
	emar rcS lib/emcc/libendless-sky.a
	emranlib lib/emcc/libendless-sky.a
	em++\
		-o hello.html\
		-O3\
		-flto\
		--source-map-base http://localhost:6931/\
		-s WASM_MEM_MAX=2147483648\
		-s INITIAL_MEMORY=838860800\
		-s ALLOW_MEMORY_GROWTH=1\
		-g\
		-s DISABLE_EXCEPTION_CATCHING=0\
		-s ASSERTIONS=2\
		-s DEMANGLE_SUPPORT=1\
		-s EMULATE_FUNCTION_POINTER_CASTS=1\
		-s FETCH=1\
		build/emcc/main.o\
		lib/emcc/libendless-sky.a\
		-lopenal\
		-lidbfs.js\
		-lGLESv2
clean:
	rm -rf build
	rm -rf lib
	rm -f hello.html
	rm -f hello.data
	rm -f hello.js
	rm -f hello.wasm
	rm -f hello.wasm.map
