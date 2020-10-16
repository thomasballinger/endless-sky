endless-sky.html:
	scons -j 8 mode=emcc music=off opengl=gles threads=off
clean:
	rm endless-sky.html
dev: endless-sky.html es2.html
	emrun --serve_after_close --serve_after_exit --browser chrome --private_browsing endless-sky.html
favicon.ico:
	wget https://endless-sky.github.io/favicon.ico
deploy: endless-sky.html favicon.ico
	mkdir -p output
	cp endless-sky.html output/index.html
	cp endless-sky.wasm endless-sky.wasm.map endless-sky.data endless-sky.js output/
	cp images/_menu/title.webp output/
	aws s3 sync output s3://play-endless-sky.com/live
	# this costs ~5 cents
	aws cloudfront create-invalidation --distribution-id E2TZUW922XPLEF --paths /\*
