//Marasescu Stefan-Gabriel, 311CA
#ifndef IMAGE_TRANSFORMS_H
#define IMAGE_TRANSFORMS_H

#include "image_loader.h"

/*functions that deal with all image editing, making
selections and calculating tone frequency for the histogram
and equalize functions*/

int is_power_of_two(int x);

void create_frequency_array(int *f, grayscale_image *image);

void histogram(void *image, image_type type, char command[]);

short round_and_clamp(double x);

void equalize_image(void *image, image_type type);

void *crop_grayscale_image(void *image);

void *crop_rgb_image(void *image);

int rotate_selection_grayscale(grayscale_image *image_gray, int angle);

int rotate_selection_rgb(rgb_image *image_color, int angle);

void rotate_selection(void *image, image_type type, char command[]);

void apply_edge(rgb_image *image, pixel **new_matrix,
				int start_x, int start_y, int end_x, int end_y);

void apply_sharpen(rgb_image *image, pixel **new_matrix,
				   int start_x, int start_y, int end_x, int end_y);

void apply_blur(rgb_image *image, pixel **new_matrix,
				int start_x, int start_y, int end_x, int end_y);

void apply_gaussian_blur(rgb_image *image, pixel **new_matrix,
						 int start_x, int start_y, int end_x, int end_y);

void apply(void *image, image_type type, char command[]);

void select(void *image, image_type type, char command[]);

void select_all(void *image, image_type type);

#endif
