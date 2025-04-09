//Marasescu Stefan-Gabriel, 311CA
#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

/*all data types used in the program for storing and
modifying images and their type*/
typedef struct {
	short r, g, b;
} pixel;

typedef struct {
	char type[3];
	int height, width;
	short max_value;
	int x1, y1, x2, y2; //coordinates for the SELECT command
	pixel **matrix;
} rgb_image;

typedef struct {
	char type[3];
	int height, width;
	short max_value;
	int x1, y1, x2, y2; //coordinates for the SELECT command
	short **matrix;
} grayscale_image;

typedef enum {
	INVALID, //invalid
	P2, //grayscale image, text file
	P3, //rgb image, text file
	P5, //grayscale image, binary file
	P6 //rgb image, binary file
} image_type;

/*functions that deal with dynamic memory allocation and freeing,
reading images from files and writing/saving the images in files*/

void free_rgb_matrix(int pos, pixel **matrix);

void free_grayscale_matrix(int pos, short **matrix);

void free_image(void *image, image_type type);

pixel **alloc_rgb_matrix(int height, int width);

short **alloc_grayscale_matrix(int height, int width);

void initialize_grayscale_image(grayscale_image *image, char type[],
								int height, int width, short max_value);

void initialize_rgb_image(rgb_image *image, char type[],
						  int height, int width, short max_value);

void *read_image_text(FILE *input_file, char type[],
					  int height, int width, short max_value);

void *read_image_binary(char file_name[], long pos, char type[],
						int height, int width, short max_value);

void *load_file(image_type *type, char command[]);

int print_image_text(void *image, image_type type, char file_name[]);

int print_image_binary(void *image, image_type type, char file_name[]);

void save_image(void *image, image_type type, char command[]);

#endif
