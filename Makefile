# Makefile for the web build of Endless Sky
endless-sky.js: libwebp
	scons -j 8 mode=emcc music=off opengl=gles threads=off
clean:
	rm -f endless-sky.js
dev: endless-sky.js webpimages
	emrun --serve_after_close --serve_after_exit --browser chrome --private_browsing endless-sky.html
favicon.ico:
	wget https://endless-sky.github.io/favicon.ico
Ubuntu-Regular.ttf:
	curl -Ls 'https://github.com/google/fonts/blob/master/ufl/ubuntu/Ubuntu-Regular.ttf?raw=true' > Ubuntu-Regular.ttf
libwebp: libwebp.tar.gz
	tar -xzf libwebp.tar.gz
	mv libwebp-1.1.0 libwebp
libwebp.tar.gz:
	wget -O libwebp.tar.gz https://github.com/webmproject/libwebp/archive/v1.1.0.tar.gz
webpimages: images
	python3 convert.py
output/index.html: endless-sky.js endless-sky.html favicon.ico title.png endless-sky.data Ubuntu-Regular.ttf webpimages
	rm -rf output
	mkdir -p output
	cp endless-sky.html output/index.html
	cp endless-sky.wasm endless-sky.wasm.map endless-sky.data endless-sky.js output/
	cp -r js/ output/js
	cp dataversion.js output/
	cp title.png output/
	cp favicon.ico output/
	cp Ubuntu-Regular.ttf output/
test: output/index.html
	cd output; (sleep 1; python3 -m webbrowser http://localhost:8000) & python -m http.server
deploy: output/index.html
	@if curl -s https://play-endless-sky.com/dataversion.js | diff - dataversion.js; \
		then \
			echo 'uploading all files except endless-sky.data...'; \
			aws s3 sync --exclude endless-sky.data output s3://play-endless-sky.com/live;\
		else \
			echo 'uploading all files, including endless-sky.data...'; \
			aws s3 sync output s3://play-endless-sky.com/live;\
	fi
	aws cloudfront create-invalidation --distribution-id E2TZUW922XPLEF --paths /\*
