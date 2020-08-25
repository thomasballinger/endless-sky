import glob
import subprocess
import re
import os
import shutil

files = glob.glob("images/**/*.png", recursive=True)
processed = 0
count = len(files)

print(f'found {count} files')
pad_len = len(str(count))

webp_output_directory = "webp"
regex = r"(.*?)([+~-])([0-9]+)\.webp"
to_mux = {}
animated_output_directory = "animated"
complete_output_directory = "complete"
"""
FRAME_OPTIONS(i):
 Create animation:
   file_i +di+[xi+yi[+mi[bi]]]
   where:    'file_i' is the i'th animation frame (WebP format),
             'di' is the pause duration before next frame,
             'xi','yi' specify the image offset for this frame,
             'mi' is the dispose method for this frame (0 or 1),
             'bi' is the blending method for this frame (+b or -b)
"""
"""
// Dispose method (animation only). Indicates how the area used by the current
// frame is to be treated before rendering the next frame on the canvas.
typedef enum WebPMuxAnimDispose {
  WEBP_MUX_DISPOSE_NONE,       // Do not dispose.
  WEBP_MUX_DISPOSE_BACKGROUND  // Dispose to background color.
} WebPMuxAnimDispose;

// Blend operation (animation only). Indicates how transparent pixels of the
// current frame are blended with those of the previous canvas.
typedef enum WebPMuxAnimBlend {
  WEBP_MUX_BLEND,              // Blend.
  WEBP_MUX_NO_BLEND            // Do not blend.
} WebPMuxAnimBlend;
"""
frame_options = "+1+0+0+1"

for png_path in files:
    processed = processed + 1
    print(f'processing file {str(processed).rjust(pad_len)} of {count}, {str(int(processed * 100 / count)).rjust(3)}%: {png_path}')
    webp_path = webp_output_directory + "/" + png_path.replace(".png", ".webp")
    os.makedirs(os.path.dirname(webp_path), exist_ok=True)
    #subprocess.run(["cwebp", png_path, "-o", webp_path, "-lossless", "-exact", "-m", "6"])

webp_files = sorted(glob.glob(webp_output_directory + "/" + "images/**/*.webp", recursive=True))
complete_files = webp_files.copy()

for webp_path in webp_files:
    matches = re.finditer(regex, webp_path, re.MULTILINE)
    for matchNum, match in enumerate(matches, start=1):
        my_list = []
        complete_files.remove(webp_path)
        if match.group(1) in to_mux:
            to_mux[match.group(1)].append(webp_path)
        else:
            my_list.append(webp_path)
            to_mux[match.group(1)] = my_list

for webp_mux in to_mux:
    command = ["webpmux"]
    for webp_file in to_mux[webp_mux]:
        command.extend(["-frame", webp_file, frame_options])
    matches = re.match(regex, to_mux[webp_mux][0])
    animated_file = animated_output_directory + "/" + webp_mux + matches.group(2) + str(len(to_mux[webp_mux]) - 1) + ".webp"
    command.extend(["-o", animated_file])
    complete_files.append(animated_file)
    os.makedirs(os.path.dirname(animated_output_directory + "/" + to_mux[webp_mux][0]), exist_ok=True)
    subprocess.run(command)

for f in complete_files:
    os.makedirs(os.path.dirname(complete_output_directory + "/" + f), exist_ok=True)
    shutil.copyfile(f, complete_output_directory + "/" + f)
