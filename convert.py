import glob
import subprocess
import re

files = glob.glob("images/**/*.png", recursive=True)
processed = 0
count = len(files)

print(f'found {count} files')
pad_len = len(str(count))

for png_path in files:
    processed = processed + 1
    print(f'processing file {str(processed).rjust(pad_len)} of {count}, {str(int(processed * 100 / count)).rjust(3)}%: {png_path}')
    webp_path = png_path.replace(".png", ".webp")
    #subprocess.run(["cwebp", png_path, "-o", webp_path, "-lossless", "-m", "6"])

    # TODO: once converted the animation frames should be muxed

webp_files = glob.glob("images/**/*.webp", recursive=True)
regex = r"/(.*?)([+~-])([0-9]+)\.webp"
to_mux = {}

for webp_path in webp_files:
    matches = re.finditer(regex, webp_path, re.MULTILINE)
    for matchNum, match in enumerate(matches, start=1):
        if to_mux[match.group(1)]:
            to_mux[match.group(1)] = [].extend(to_mux[match.group(1)]).append(webp_path)
