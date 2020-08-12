import os
import platform
from SCons.Node.FS import Dir

def pathjoin(*args):
	return os.path.join(*args)

# Load environment variables, including some that should be renamed.
# If we are compiling on Windows, then we need to change the toolset to MinGW.
is_windows_host = platform.system().startswith('Windows')
scons_toolset = ['mingw' if is_windows_host else 'default']
env = DefaultEnvironment(tools = scons_toolset, ENV = os.environ)

if 'CXX' in os.environ:
	env['CXX'] = os.environ['CXX']
if 'CXXFLAGS' in os.environ:
	env.Append(CCFLAGS = os.environ['CXXFLAGS'])
if 'LDFLAGS' in os.environ:
	env.Append(LINKFLAGS = os.environ['LDFLAGS'])
if 'AR' in os.environ:
	env['AR'] = os.environ['AR']
if 'RANLIB' in os.environ:
	env['RANLIB'] = os.environ['RANLIB']
if 'DIR_ESLIB' in os.environ:
	path = os.environ['DIR_ESLIB']
	env.Prepend(CPPPATH = [pathjoin(path, 'include')])
	env.Append(LIBPATH = [pathjoin(path, 'lib')])

# The Steam runtime has an out-of-date libstdc++, so link it in statically:
chroot_name = os.environ.get('SCHROOT_CHROOT_NAME', '')
if 'steamrt' in chroot_name:
	env.Append(LINKFLAGS = ["-static-libstdc++"])

opts = Variables()
opts.AddVariables(
	EnumVariable("mode", "Compilation mode", "release", allowed_values=("release", "debug", "profile", "emcc")),
	EnumVariable("opengl", "Whether to use OpenGL or OpenGL ES", "desktop", allowed_values=("desktop", "gles")),
	EnumVariable("music", "Whether to use music", "on", allowed_values=("on", "off")),
	EnumVariable("threads", "Whether to use threads", "on", allowed_values=("on", "off")),
	PathVariable("BUILDDIR", "Directory to store compiled object files in", "build", PathVariable.PathIsDirCreate),
	PathVariable("BIN_DIR", "Directory to store binaries in", ".", PathVariable.PathIsDirCreate),
	PathVariable("DESTDIR", "Destination root directory, e.g. if building a package", "", PathVariable.PathAccept),
	PathVariable("PREFIX", "Directory to install under (will be prefixed by DESTDIR)", "/usr/local", PathVariable.PathIsDirCreate),
)
opts.Update(env)
Help(opts.GenerateHelpText(env))

# Required build flags. To enable SSE or other optimizations, pass CXXFLAGS via the environment
#   $ CXXFLAGS=-msse3 scons
#   $ CXXFLAGS=-march=native scons
# or modify the `flags` variable:
flags = [] if env["mode"] == "emcc" else [
    "-std=c++11",
    "-Wall",
    "-Werror",
    "-Wold-style-cast"
]
common_flags = [""]
if env["mode"] != "debug":
	flags += ["-O3", "-flto"]
	env.Append(LINKFLAGS = ["-O3", "-flto"])
if env["mode"] == "debug":
	flags += ["-g"]
elif env["mode"] == "profile":
	flags += ["-pg"]
	env.Append(LINKFLAGS = ["-pg"])
env.Append(CCFLAGS = flags)
# Omit emitting a symbol table when creating/updating static libraries, because Scons
# will run ranlib. If we are using gcc-ranlib, assume support for thin archives as well.
create_thin_archives = any(env.get(var, '').startswith('gcc') for var in ('AR', 'RANLIB'))
env.Replace(ARFLAGS = 'rcST' if create_thin_archives else 'rcS')

game_libs = [
	"winmm",
	"mingw32",
	"sdl2main",
	"sdl2.dll",
	"png.dll",
	"turbojpeg.dll",
	"jpeg.dll",
	"mad.dll",
	"openal32.dll",
	"glew32.dll",
	"opengl32",
] if is_windows_host else [
	"SDL2",
	"png",
	"jpeg",
	"openal",
	"webp",
	"webpdemux",
] if env["mode"] != "emcc" else [
	"openal",
]
env.Append(LIBS = game_libs)

if env["mode"] == "emcc":
	if env["music"] != "off":
		print("emcc requires music=off")
		Exit(1)
	if env["opengl"] != "gles":
		print("emcc requires opengl=gles")
		Exit(1)
	if env["threads"] != "off":
		print("emcc requires threads=off")
		Exit(1)
	flags += ["-g4"]
	env['CXX'] = "em++"
	env['CC'] = "emcc"
	env['AR'] = "emar"
	env['RANLIB'] = "emranlib"
	common_flags += [
		"-s", "DISABLE_EXCEPTION_CATCHING=0",
		"-s", "USE_SDL=2",
		"-s", "USE_LIBPNG=1",
		"-s", "USE_LIBJPEG=1",
		"-s", "USE_WEBGL2=1",
		"-s", "ASSERTIONS=2",
		"-s", "DEMANGLE_SUPPORT=1",
		"-s", "GL_ASSERTIONS=1",
		"-s", "MIN_WEBGL_VERSION=2",
		"-s", "EMULATE_FUNCTION_POINTER_CASTS=1",
		"-s", "FETCH=1"
	]
	env.Append(LINKFLAGS = [
		"--source-map-base", "http://localhost:6931/",
		"-s", "WASM_MEM_MAX=2147483648", # 2GB
		"-s", "INITIAL_MEMORY=838860800", # 800MB
		"-s", "ALLOW_MEMORY_GROWTH=1",
		"-s", "EXTRA_EXPORTED_RUNTIME_METHODS=['callMain']",
		"--preload-file", "data",
		"--preload-file", "images",
		"--preload-file", "sounds",
		"--preload-file", "credits.txt",
		"--preload-file", "keys.txt",
		"--preload-file", "recent.txt",
		#"--emrun",  # useful in dev, but causes hundreds of errors in prod
		"-g4"
	])
	env.Append(LIBS = [
		"idbfs.js"
	]);

if env["music"] == "off":
	flags += ["-DES_NO_MUSIC"]

if env["threads"] == "off":
	flags += ["-DES_NO_THREADS"]
else:
	env.Append(LIBS = [
		"pthread"
	]);

if env["opengl"] == "desktop":
	env.Append(LIBS = [
		"GL",
		"GLEW"
	]);
else:
	env.Append(LIBS = [
		"GLESv2"
	]);
	flags += ["-DES_GLES"]


# Required build flags. If you want to use SSE optimization, you can turn on
# -msse3 or (if just building for your own computer) -march=native.
env.Append(CCFLAGS = flags)
env.Append(CCFLAGS = common_flags)
env.Append(LINKFLAGS = common_flags)


if env["music"] == "on":
	# libmad is not in the Steam runtime, so link it statically:
	if 'steamrt_scout_i386' in chroot_name:
		env.Append(LIBS = File("/usr/lib/i386-linux-gnu/libmad.a"))
	elif 'steamrt_scout_amd64' in chroot_name:
		env.Append(LIBS = File("/usr/lib/x86_64-linux-gnu/libmad.a"))
	else:
		env.Append(LIBS = "mad")


binDirectory = '' if env["BIN_DIR"] == '.' else pathjoin(env["BIN_DIR"], env["mode"])
buildDirectory = pathjoin(env["BUILDDIR"], env["mode"])
libDirectory = pathjoin("lib", env["mode"])
VariantDir(buildDirectory, "source", duplicate = 0)

# Find all regular source files.
def RecursiveGlob(pattern, dir_name=buildDirectory):
	# Start with source files in subdirectories.
	matches = [RecursiveGlob(pattern, sub_dir) for sub_dir in Glob(pathjoin(str(dir_name), "*"))
		if isinstance(sub_dir, Dir)]
	# Add source files in this directory, except for main.cpp
	matches += Glob(pathjoin(str(dir_name), pattern), exclude=["*/main.cpp"])
	return matches

# By default, invoking scons will build the backing archive file and then the game binary.
sourceLib = env.StaticLibrary(pathjoin(libDirectory, "endless-sky"), RecursiveGlob("*.cpp", buildDirectory))
outname = "endless-sky"
if env["mode"] == "emcc":
    outname += ".js"
exeObjs = [Glob(pathjoin(buildDirectory, f)) for f in ("main.cpp",)]
if is_windows_host:
	windows_icon = env.RES(pathjoin(buildDirectory, "WinApp.rc"))
	exeObjs.append(windows_icon)
sky = env.Program(pathjoin(binDirectory, outname), exeObjs + sourceLib)
env.Default(sky)


# The testing infrastructure ignores "mode" specification (i.e. we only test optimized output).
# (If we add support for code coverage output, this will likely need to change.)
testBuildDirectory = pathjoin("tests", env["BUILDDIR"])
VariantDir(testBuildDirectory, pathjoin("tests", "src"), duplicate = 0)
test = env.Program(
	target=pathjoin("tests", "endless-sky-tests"),
	source=RecursiveGlob("test_*.cpp", testBuildDirectory) + sourceLib,
	 # Add Catch header & additional test includes to the existing search paths
	CPPPATH=(env.get('CPPPATH', []) + [pathjoin('tests', 'include')]),
	# Do not link against the actual implementations of SDL, OpenGL, etc.
	LIBS=[],
)
# Invoking scons with the `build-tests` target will build the unit test framework
env.Alias("build-tests", test)
# Invoking scons with the `test` target will build (if necessary) and
# execute the unit test framework (always). All non-hidden tests are run.
catch2_args = " " + " ".join([
	"-i",
	"--warn NoAssertions",
	"--order rand",
	"--rng-seed 'time'",
])
test_runner = env.Action(test[0].abspath + catch2_args, 'Running tests...')
env.Alias("test", test, test_runner)
env.AlwaysBuild("test")

if env["mode"] == "emcc":
    env.Command("title.png", "images/_menu/title.png", Copy("$TARGET", "$SOURCE"))

def create_data_version_javascript(env, target, source):
	import hashlib
	hash_md5 = hashlib.md5()
	with open(str(source[0]), "rb") as f:
		for chunk in iter(lambda: f.read(4096), b""):
			hash_md5.update(chunk)
	hash = hash_md5.hexdigest()

	with open(str(target[0]), 'w') as f:
		f.write('// autogenerated file, do not edit\n')
		f.write('// This is the md5 hash of the data file expected\n')
		f.write('var endlessSkyDataVersion = "')
		f.write(hash)
		f.write('";\n')

if env["mode"] == "emcc":
	env.Command(
		target="dataversion.js",
		source="endless-sky.data",
		action=create_data_version_javascript)

# Install the binary:
env.Install("$DESTDIR$PREFIX/games", sky)

# Install the desktop file:
env.Install("$DESTDIR$PREFIX/share/applications", "endless-sky.desktop")

# Install app center metadata:
env.Install("$DESTDIR$PREFIX/share/appdata", "endless-sky.appdata.xml")

# Install icons, keeping track of all the paths.
# Most Ubuntu apps supply 16, 22, 24, 32, 48, and 256, and sometimes others.
sizes = ["16x16", "22x22", "24x24", "32x32", "48x48", "128x128", "256x256", "512x512"]
icons = []
for size in sizes:
	destination = "$DESTDIR$PREFIX/share/icons/hicolor/" + size + "/apps/endless-sky.png"
	icons.append(destination)
	env.InstallAs(destination, "icons/icon_" + size + ".png")

# If any of those icons changed, also update the cache.
# Do not update the cache if we're not installing into "usr".
# (For example, this "install" may actually be creating a Debian package.)
if env.get("PREFIX").startswith("/usr/"):
	env.Command(
		[],
		icons,
		"gtk-update-icon-cache -t $DESTDIR$PREFIX/share/icons/hicolor/")

# Install the man page.
env.Command(
	"$DESTDIR$PREFIX/share/man/man6/endless-sky.6.gz",
	"endless-sky.6",
	"gzip -c $SOURCE > $TARGET")

# Install the data files.
def RecursiveInstall(env, target, source):
	rootIndex = len(env.Dir(source).abspath) + 1
	for node in env.Glob(pathjoin(source, '*')):
		if node.isdir():
			name = node.abspath[rootIndex:]
			RecursiveInstall(env, pathjoin(target, name), node.abspath)
		else:
			env.Install(target, node)
RecursiveInstall(env, "$DESTDIR$PREFIX/share/games/endless-sky/data", "data")
RecursiveInstall(env, "$DESTDIR$PREFIX/share/games/endless-sky/images", "images")
RecursiveInstall(env, "$DESTDIR$PREFIX/share/games/endless-sky/sounds", "sounds")
env.Install("$DESTDIR$PREFIX/share/games/endless-sky", "credits.txt")
env.Install("$DESTDIR$PREFIX/share/games/endless-sky", "keys.txt")

# Make the word "install" in the command line do an installation.
env.Alias("install", "$DESTDIR$PREFIX")
