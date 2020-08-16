import glob
import subprocess

files = glob.glob("images/**/*.png", recursive=True)
processed = 0
count = len(files)

print(f'found {count} files')
pad_len = len(str(count))

for png_path in files:
    processed = processed + 1
    print(f'processing file {str(processed).rjust(pad_len)} of {count}, {str(int(processed * 100 / count)).rjust(3)}%: {png_path}')
    webp_path = png_path.replace(".png", ".webp")
    subprocess.run(["cwebp", png_path, "-o", webp_path, "-lossless", "-m", "6"])

    # TODO: once converted the animation frames should be muxed
