# General overview of the program

The provided program works as an image editor that can make transformations on images represented in the PPM (rgb) or PGM (grayscale) file formats (P2 = grayscale image represented in a plain-text format, P3 = rgb image represented in a plain-text format, P5 = grayscale image represented in a binary format, P6 = rgb image represented in a binary format). All images given in the input via the `LOAD` command are dynamically allocated (memory is allocated on the program's heap), to ensure efficient memory usage and allowing the storage of larger images. At the end of the program's execution, or when another image is loaded, all dynamically allocated memory is freed, ensuring no memory leaks.

The supported commands for the program are split into 2 categories:

### 1) Image loading (input/output):

- `LOAD <file_name>` — loads an image from a file of one of the 4 supported formats  
- `SAVE <file_name> <ascii>` — saves the current image in a file, text file if the ascii parameter is present, or binary file if not

### 2) Image transforms (editing):

- `SELECT <x1> <y1> <x2> <y2>` — selection of pixels between height  
- `SELECT ALL` — the selection is brought back to the full image  
- `HISTOGRAM <x> <y>` — only supported for grayscale images, prints at stdout the histogram of the image with maximum x stars and y bins  
- `EQUALIZE` — only supported for grayscale images, equalizes image tones and is applied to the full image. Useful for photos taken with incorrect exposure  
- `ROTATE <angle>` — rotates the selection with an angle of 0, ±90, ±180, ±270, or ±360 if the selection is square or rotates the whole image if the selection is the full image  
- `CROP` — reduces the image to only the selected area  
- `APPLY <parameter>` — applies one of the following parameters to the image selection: `EDGE`, `SHARPEN`, `BLUR`, `GAUSSIAN_BLUR`

The `EXIT` command stops the execution of the program.

---

# Data types/structures

The 4 data types defined in this program are:

- `pixel` — has 3 separate values for r, g, b  
- `rgb_image` — stores all data about the image if the format is P3 or P6, including the image itself  
- `grayscale_image` — same as `rgb_image`, but for format P2 or P5  
- `image_type` — simplifies checking for the type of the image in all functions, in order to figure out if the image is grayscale or rgb

In the main function, we only use one void pointer for the image and pass it to every function, where we cast it to a `rgb_image` pointer or a `grayscale_image` pointer depending on the image type. This way, the code remains flexible and general, and this allows every function to work for both grayscale and rgb images, not needing separate functions, making them simpler to follow and maintain.

---

# Image loader functions

When the user types the command `LOAD <file_name>`, we will first call the function `free_image` that frees all the allocated memory for the old image, if an image is already loaded, which calls either `free_rgb_matrix` or `free_grayscale_matrix`, depending on the format of the old image.

After that, the `load_file` function is called, which first opens the given file as a text file and only reads the header (the image format, the dimensions, and the max pixel value). After that, we update the `type` variable accordingly, and depending on the type, we either call `read_image_text`, which opens the file as a text file, or `read_image_binary`, which opens the file as a binary file and both continue reading where we left off.

For the `SAVE <file_name> <ascii>` command, we will call the `save_image` function, which calls one of `print_image_binary` or `print_image_text`, depending on the ascii parameter being present or not.

---

# Image transform functions

These functions are based on image manipulation and processing, some of the usual transformations found in most photo editors. Functions like `create_frequency_array` are used in order to analyze tone frequency in images in order to print the image histogram or apply the formula for equalizing them.

`round_and_clamp` is used for all apply functions, and for equalize, so that pixel values always remain in bounds ([0, 255]).

The `crop` function, instead of allocating a new matrix for the cropped selection and freeing the old matrix afterward, copies the selected part to the top-left corner of the same image, so that it can just reallocate the memory to shrink it to only that corner, for memory efficiency.
