endless-sky.html:
	rm -f libwebp/src/*/*.o
	python -c 's = open("source/ImageBuffer.cpp").read().replace("""#include "sha1.hpp"\n""", ""); open("source/ImageBuffer.cpp", "w").write(s)'
	CXXFLAGS="-DNO_AUDIO" scons -j 8 mode=emcc music=off opengl=gles threads=off
clean:
	rm endless-sky.html
dev: endless-sky.html
	emrun --serve_after_close --serve_after_exit --browser chrome --private_browsing endless-sky.html
deploy: build
	mkdir -p output
	cp endless-sky.html output/index.html
	cp endless-sky.wasm endless-sky.wasm.map endless-sky.data endless-sky.js output/
	cp images/_menu/title.webp output/
	aws s3 sync output s3://play-endless-sky.com/live
