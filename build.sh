#!/bin/bash
set -x

echo Was this triggered by a webhook?
echo $INCOMING_HOOK_TITLE
echo $INCOMING_HOOK_BODY

# install build system used by Endless Sky (it's not popular enough to already be installed)
python -m pip install scons

# merge in emscripten changes
git config --global user.email "example@example.com"
git config --global user.name "beep boop"
git fetch https://github.com/thomasballinger/endless-sky.git es-wasm-reformatted
git merge FETCH_HEAD

# activate emscripten (built by plugin)
#EMSCRIPTEN_VERSION="1.39.18" ?
#EMSCRIPTEN_VERSION="1.40.1" # works
#EMSCRIPTEN_VERSION="2.0.0" # fails during compilation?
#EMSCRIPTEN_VERSION="2.0.3" # fails during compilation?
#EMSCRIPTEN_VERSION="2.0.4" # fails during compilation
#EMSCRIPTEN_VERSION="2.0.6" # doesn't work (endless-sky.js:13478 Uncaught TypeError: Cannot convert undefined to a BigInt)
#EMSCRIPTEN_VERSION="2.0.6" # works!
EMSCRIPTEN_VERSION="2.0.12" # should include the spaces fix!

# check caches
ls build/emcc || true
ls lib/emcc || true
ls emsdk/upstream/emscripten/cache || true
ls emsdk/upstream/emscripten/cache/wasm-lto/ || true

./emsdk/emsdk activate $EMSCRIPTEN_VERSION || exit $?
source emsdk/emsdk_env.sh

#echo "Checking if cache has been cleared?"
ls emsdk/upstream/emscripten/cache
ls emsdk/upstream/emscripten/cache/wasm-lto/

make output/index.html || exit $?

#echo "After making, what's the cache look like?"
#ls ./emsdk/upstream/emscripten/cache/
#ls ./emsdk/upstream/emscripten/cache/wasm-lto/

echo Deploy url:
echo $DEPLOY_URL
