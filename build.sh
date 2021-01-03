#!/bin/bash

# install scons
python -m pip install scons

# merge in emscripten changes
git config --global user.email "example@example.com"
git config --global user.name "beep boop"
git fetch https://github.com/thomasballinger/endless-sky.git es-wasm-reformatted
git merge FETCH_HEAD

# build emscripten
EMSCRIPTEN_VERSION="latest"

TEMPDIR=`mktemp -d`
(cd ${TEMPDIR} &&
 git clone https://github.com/emscripten-core/emsdk.git &&
 cd emsdk &&
 ./emsdk install ${EMSCRIPTEN_VERSION} &&
 ./emsdk activate ${EMSCRIPTEN_VERSION}) || exit $?
source ${TEMPDIR}/emsdk/emsdk_env.sh
make output/index.html || exit $?
rm -rf ${TEMPDIR}
