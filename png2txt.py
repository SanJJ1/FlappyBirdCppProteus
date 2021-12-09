# The script was used to convert folder of .pngs
# to a folder of .txts, with following format:
#
# width
# height
# pixel 1 ARGB as 32-bit value
# pixel 2 ARGB as 32-bit value
# ...
# pixel width * height ARGB as 32-bit value
import imageio as iio
import numpy as np
# import sys
import os  # used for iterating through files in folder

# np.set_printoptions(threshold=sys.maxsize)

input_folder = 'sprites'
output_folder = "s"

for subdir, dirs, files in os.walk(input_folder):
    for file in files:
        # filepath of file to be converted
        input_filepath = subdir + os.sep + file

        # converts png to a h * w numpy array, where each element is an array
        # of the form [R, G, B, A]
        rgbaArray = np.uint32(iio.imread(input_filepath))

        # converts each element in previous array
        # to a 32-bit integer, representing an ARGB value.
        argbFlattened = (rgbaArray[..., 3] << 24) | (rgbaArray[..., 0] << 16) | (rgbaArray[..., 1] << 8) | rgbaArray[
            ..., 2]

        # converts each value to full transparent if not already fully opaque,
        # essentially normalizing transparent channel to either 0 or 255.
        argbNormalized = [i if i >= 0xFF000000 else 0 for i in list(np.ravel(argbFlattened))]

        # creates a string ready for outputting to text file
        s = '\n'.join(str(z) for z in [rgbaArray.shape[1]] + [rgbaArray.shape[0]] + argbNormalized)

        # creates output filepath in new folder, with same names as old files,
        # using .txt extension instead of .png.
        file_name = input_filepath.split("\\")[1][:-3]
        file_extension = "txt"
        output_filepath = output_folder + "\\" + file_name + file_extension

        # string to file
        with open(output_filepath, "w") as f:
            f.write(s)
        print("finished converting", input_filepath, "to", output_filepath)
