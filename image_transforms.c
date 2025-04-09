//Marasescu Stefan-Gabriel, 311CA
#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <math.h>

#include "image_loader.h"

#include "image_transforms.h"

void create_frequency_array(int *f, grayscale_image *image)
{
	for (int i = 0; i < image->height; i++)
		for (int j = 0; j < image->width; j++)
			f[image->matrix[i][j]]++;
}

int is_power_of_two(int x)
{
	while (x != 1) {
		if (x % 2 == 0)
			x = x / 2;
		else
			return 0;
	}

	return 1;
}

void histogram(void *image, image_type type, char command[])
{
	if (!image) {
		printf("No image loaded\n");
		return;
	}

	int max_stars, bins_num, extra_parameter; //x and y

	char comm[100];

	if (sscanf(command, "%s%d%d%d", comm, &max_stars,
			   &bins_num, &extra_parameter) != 3) {
		printf("Invalid command\n");
		return;
	}

	grayscale_image *image_gray = NULL;

	if (type == P2 || type == P5) {
		image_gray = (grayscale_image *)image;
	} else if (type == P3 || type == P6) {
		printf("Black and white image needed\n");
		return;
	}

	if (bins_num < 2 || bins_num > 256 || !is_power_of_two(bins_num)) {
		printf("Invalid set of parameters\n");
		return;
	}

	int frequency_array[256] = {0}; //initialize with 0

	create_frequency_array(frequency_array, image_gray);

	int values_in_bin = 256 / bins_num; //how many values are dealt in one bin

	int max_frequency = 0; //the max sum of frequencies in one bin

	for (int i = 0; i < bins_num; i++) { //iterating trough all the bins
		long frequency_sum = 0;

		for (int j = values_in_bin * i; j < values_in_bin * (i + 1); j++)
			frequency_sum += frequency_array[j];

		if (frequency_sum > max_frequency)
			max_frequency = frequency_sum;
	}

	for (int i = 0; i < bins_num; i++) { //iterating trough all the bins
		double stars_num = 0;
		for (int j = values_in_bin * i; j < values_in_bin * (i + 1); j++)
			stars_num += frequency_array[j];

		stars_num = (stars_num / max_frequency) * max_stars;
		stars_num = floor(stars_num);

		printf("%d\t|\t", (int)stars_num);
		for (int i = 0; i < (int)stars_num; i++)
			printf("*");
		printf("\n");
	}
}

short round_and_clamp(double x)
{
	short rounded_x = round(x);

	if (rounded_x < 0)
		return 0;
	else if (rounded_x > 255)
		return 255;
	else
		return rounded_x;
}

void equalize_image(void *image, image_type type)
{
	if (!image) {
		printf("No image loaded\n");
		return;
	}

	grayscale_image *image_gray = NULL;

	if (type == P2 || type == P5) {
		image_gray = (grayscale_image *)image;
	} else if (type == P3 || type == P6) {
		printf("Black and white image needed\n");
		return;
	}

	int frequency_array[256] = {0}; //initialize with 0

	create_frequency_array(frequency_array, image_gray);

	for (int i = 0; i < image_gray->height; i++) {
		for (int j = 0; j < image_gray->width; j++) {
			double sum = 0;
			for (int k = 0; k <= image_gray->matrix[i][j]; k++)
				sum += frequency_array[k];

			sum = sum / (image_gray->height * image_gray->width) * 255;
			image_gray->matrix[i][j] = round_and_clamp(sum);
		}
	}

	printf("Equalize done\n");
}

void *crop_grayscale_image(void *image)
{
	if (!image) {
		printf("No image loaded\n");
		return NULL;
	}

	grayscale_image *image_gray = (grayscale_image *)image;

	/*moving the selected area to the top left corner of the
	image, in order to realloc the memory without losing data*/
	for (int i = image_gray->y1; i < image_gray->y2 ; i++)
		for (int j = image_gray->x1; j < image_gray->x2; j++)
			image_gray->matrix[i - image_gray->y1][j - image_gray->x1] =
			image_gray->matrix[i][j];

	int new_height = image_gray->y2 - image_gray->y1; //cropped image height
	int new_width = image_gray->x2 - image_gray->x1; //cropped image width

	for (int i = new_height; i < image_gray->height; i++)
		free(image_gray->matrix[i]); //freeing all rows after the new height

	//reducing the size of the matrix to the size of the selection
	short **new_matrix = (short **)
	realloc(image_gray->matrix, new_height * sizeof(short *));

	if (!new_matrix) {
		free_grayscale_matrix(new_height, image_gray->matrix);
		return NULL;
	}

	image_gray->matrix = new_matrix;

	for (int i = 0; i < new_height; i++) {
		short *new_row = NULL;
		new_row = (short *)
		realloc(image_gray->matrix[i], new_width * sizeof(short));

		if (!new_row) {
			free_grayscale_matrix(new_height, image_gray->matrix);
			return NULL;
		}

		image_gray->matrix[i] = new_row;
	}

	//updating all fields that changed
	image_gray->height = new_height;
	image_gray->width = new_width;
	//the new selected area is the whole cropped image
	image_gray->x1 = 0;
	image_gray->y1 = 0;
	image_gray->x2 = new_width;
	image_gray->y2 = new_height;

	printf("Image cropped\n");
	return (void *)image_gray;
}

void *crop_rgb_image(void *image)
{
	if (!image) {
		printf("No image loaded\n");
		return NULL;
	}

	rgb_image *image_color = (rgb_image *)image;

	/*moving the selected area to the top left corner of the
	image, in order to realloc the memory without losing data*/
	for (int i = image_color->y1; i < image_color->y2 ; i++)
		for (int j = image_color->x1; j < image_color->x2; j++) {
			int l = i - image_color->y1;
			int c = j - image_color->x1;
			image_color->matrix[l][c].r = image_color->matrix[i][j].r;
			image_color->matrix[l][c].g = image_color->matrix[i][j].g;
			image_color->matrix[l][c].b = image_color->matrix[i][j].b;
		}

	int new_height = image_color->y2 - image_color->y1; //cropped image height
	int new_width = image_color->x2 - image_color->x1; //cropped image width

	for (int i = new_height; i < image_color->height; i++)
		free(image_color->matrix[i]); //freeing all rows after the new height

	//reducing the size of the matrix to the size of the selection
	pixel **new_matrix = (pixel **)
	realloc(image_color->matrix, new_height * sizeof(pixel *));

	if (!new_matrix) {
		free_rgb_matrix(new_height, image_color->matrix);
		return NULL;
	}

	image_color->matrix = new_matrix;

	for (int i = 0; i < new_height; i++) {
		pixel *new_row = NULL;
		new_row = (pixel *)
		realloc(image_color->matrix[i], new_width * sizeof(pixel));

		if (!new_row) {
			free_rgb_matrix(new_height, image_color->matrix);
			return NULL;
		}

		image_color->matrix[i] = new_row;
	}

	//updating all fields that changed
	image_color->height = new_height;
	image_color->width = new_width;
	//the new selected area is the whole cropped image
	image_color->x1 = 0;
	image_color->y1 = 0;
	image_color->x2 = new_width;
	image_color->y2 = new_height;

	printf("Image cropped\n");
	return (void *)image_color;
}

int rotate_selection_grayscale(grayscale_image *image_gray, int angle)
{
	int selection_height = image_gray->y2 - image_gray->y1;

	int selection_width = image_gray->x2 - image_gray->x1;

	short **rotated_matrix = NULL;

	if (angle / 90 == 1 || angle / 90 == -3) { //90 degrees to the right
		rotated_matrix =
		alloc_grayscale_matrix(selection_width, selection_height);

		if (!rotated_matrix)
			return 1;

		for (int i = image_gray->y1, c = selection_height - 1;
				 i < image_gray->y2; i++, c--)
			for (int j = image_gray->x1, r = 0; j < image_gray->x2; j++, r++)
				rotated_matrix[r][c] = image_gray->matrix[i][j];

	} else if (angle / 90 == -1 || angle / 90 == 3) { //90 degrees to the left
		rotated_matrix =
		alloc_grayscale_matrix(selection_width, selection_height);

		if (!rotated_matrix)
			return 1;

		for (int i = image_gray->y1, c = 0; i < image_gray->y2; i++, c++)
			for (int j = image_gray->x1, r = selection_width - 1;
					 j < image_gray->x2; j++, r--)
				rotated_matrix[r][c] = image_gray->matrix[i][j];

	} else if (angle / 90 == 2 || angle / 90 == -2) { //rotate with 180 degrees
		rotated_matrix =
		alloc_grayscale_matrix(selection_height, selection_width);

		if (!rotated_matrix)
			return 1;

		for (int i = image_gray->y1, r = selection_height - 1;
				 i < image_gray->y2; i++, r--)
			for (int j = image_gray->x1, c = selection_width - 1;
					 j < image_gray->x2; j++, c--)
				rotated_matrix[r][c] = image_gray->matrix[i][j];
	}

	if (image_gray->x1 != 0 || image_gray->y1 != 0 ||
		image_gray->x2 != image_gray->width ||
		image_gray->y2 != image_gray->height) { //selection isn't the full image
		for (int i = image_gray->y1, r = 0; i < image_gray->y2; i++, r++)
			for (int j = image_gray->x1, c = 0; j < image_gray->x2; j++, c++)
				image_gray->matrix[i][j] = rotated_matrix[r][c];

		if (angle / 90 == 1 || angle / 90 == -1 ||
			angle / 90 == 3 || angle / 90 == -3)
			free_grayscale_matrix(selection_width, rotated_matrix);
		else if (angle / 90 == 2 || angle / 90 == -2)
			free_grayscale_matrix(selection_height, rotated_matrix);
	} else { //the selection is the full image
		free_grayscale_matrix(image_gray->height, image_gray->matrix);
		image_gray->matrix = rotated_matrix;
		if (angle / 90 == 1 || angle / 90 == -1 ||
			angle / 90 == 3 || angle / 90 == -3) {
			image_gray->height = selection_width;
			image_gray->width = selection_height;
			image_gray->x2 = image_gray->width;
			image_gray->y2 = image_gray->height;
		} //updated matrix new size if rotated right or left
	}

	return 0;
}

int rotate_selection_rgb(rgb_image *image_color, int angle)
{
	int selection_height = image_color->y2 - image_color->y1;

	int selection_width = image_color->x2 - image_color->x1;

	pixel **rotated_matrix = NULL;

	if (angle / 90 == 1 || angle / 90 == -3) { //90 degrees to the right
		rotated_matrix = alloc_rgb_matrix(selection_width, selection_height);

		if (!rotated_matrix)
			return 1;

		for (int i = image_color->y1, c = selection_height - 1;
				 i < image_color->y2; i++, c--)
			for (int j = image_color->x1, r = 0;
					 j < image_color->x2; j++, r++) {
				rotated_matrix[r][c].r = image_color->matrix[i][j].r;
				rotated_matrix[r][c].g = image_color->matrix[i][j].g;
				rotated_matrix[r][c].b = image_color->matrix[i][j].b;
			}

	} else if (angle / 90 == -1 || angle / 90 == 3) { //90 degrees to the left
		rotated_matrix = alloc_rgb_matrix(selection_width, selection_height);

		if (!rotated_matrix)
			return 1;

		for (int i = image_color->y1, c = 0; i < image_color->y2; i++, c++)
			for (int j = image_color->x1, r = selection_width - 1;
					 j < image_color->x2; j++, r--) {
				rotated_matrix[r][c].r = image_color->matrix[i][j].r;
				rotated_matrix[r][c].g = image_color->matrix[i][j].g;
				rotated_matrix[r][c].b = image_color->matrix[i][j].b;
			}

	} else if (angle / 90 == 2 || angle / 90 == -2) { // rotate with 180 degrees
		rotated_matrix = alloc_rgb_matrix(selection_height, selection_width);

		if (!rotated_matrix)
			return 1;

		for (int i = image_color->y1, r = selection_height - 1;
				 i < image_color->y2; i++, r--)
			for (int j = image_color->x1, c = selection_width - 1;
					 j < image_color->x2; j++, c--) {
				rotated_matrix[r][c].r = image_color->matrix[i][j].r;
				rotated_matrix[r][c].g = image_color->matrix[i][j].g;
				rotated_matrix[r][c].b = image_color->matrix[i][j].b;
			}
	}

	if (image_color->x1 != 0 || image_color->y1 != 0 ||
	    image_color->x2 != image_color->width ||
		image_color->y2 != image_color->height) {
		//selection isn't the full image
		for (int i = image_color->y1, r = 0; i < image_color->y2; i++, r++)
			for (int j = image_color->x1, c = 0; j < image_color->x2; j++, c++)
				image_color->matrix[i][j] = rotated_matrix[r][c];

		if (angle / 90 == 1 || angle / 90 == -1 ||
		    angle / 90 == 3 || angle / 90 == -3)
			free_rgb_matrix(selection_width, rotated_matrix);
		else if (angle / 90 == 2 || angle / 90 == -2)
			free_rgb_matrix(selection_height, rotated_matrix);
	} else { //selection is the full image
		free_rgb_matrix(image_color->height, image_color->matrix);
		image_color->matrix = rotated_matrix;
		if (angle / 90 == 1 || angle / 90 == -1 ||
		    angle / 90 == 3 || angle / 90 == -3) {
			image_color->height = selection_width;
			image_color->width = selection_height;
			image_color->x2 = image_color->width;
			image_color->y2 = image_color->height;
		} //updated matrix new size
	}

	return 0;
}

void rotate_selection(void *image, image_type type, char command[])
{
	if (!image) {
		printf("No image loaded\n");
		return;
	}

	int angle = 0;

	char comm[100];

	if (sscanf(command, "%s%d", comm, &angle) != 2)
		return;

	if (angle < -360 || angle > 360 || angle % 90 != 0) {
		printf("Unsupported rotation angle\n");
		return;
	}

	if (angle == 360 || angle == -360 || angle == 0) {
		printf("Rotated %d\n", angle);
		return;
	}

	if (type == P2 || type == P5) { //grayscale
		grayscale_image *image_gray = (grayscale_image *)image;

		if ((image_gray->x1 != 0 || image_gray->y1 != 0 ||
		     image_gray->x2 != image_gray->width ||
			 image_gray->y2 != image_gray->height) &&
		     (image_gray->x2 - image_gray->x1 !=
			  image_gray->y2 - image_gray->y1)) {
			//the selection isn't square or full image
			printf("The selection must be square\n");
			return;
		}

		if (rotate_selection_grayscale(image_gray, angle) == 1)
			return; //something went wrong

	} else if (type == P3 || type == P6) { //rgb
		rgb_image *image_color = (rgb_image *)image;

		if ((image_color->x1 != 0 || image_color->y1 != 0 ||
			 image_color->x2 != image_color->width ||
			 image_color->y2 != image_color->height) &&
		     (image_color->x2 - image_color->x1 !=
			 image_color->y2 - image_color->y1)) {
			//the selection isn't square or full image
			printf("The selection must be square\n");
			return;
		}

		if (rotate_selection_rgb(image_color, angle) == 1)
			return; //something went wrong
	}

	printf("Rotated %d\n", angle);
}

void apply_edge(rgb_image *image, pixel **new_matrix,
				int start_x, int start_y, int end_x, int end_y)
{
	int kernel[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};

	for (int i = image->y1 + start_y; i < image->y2 - end_y; i++)
		for (int j = image->x1 + start_x; j < image->x2 - end_x; j++) {
			double r = 0, g = 0, b = 0;

			for (int l = i - 1; l <= i + 1; l++)
				for (int c = j - 1; c <= j + 1; c++) {
					r += image->matrix[l][c].r * kernel[l - i + 1][c - j + 1];
					g += image->matrix[l][c].g * kernel[l - i + 1][c - j + 1];
					b += image->matrix[l][c].b * kernel[l - i + 1][c - j + 1];
				}

			int l = i - (image->y1 + start_y);
			int c = j - (image->x1 + start_x);
			new_matrix[l][c].r = round_and_clamp(r);
			new_matrix[l][c].g = round_and_clamp(g);
			new_matrix[l][c].b = round_and_clamp(b);
		}
}

void apply_sharpen(rgb_image *image, pixel **new_matrix,
				   int start_x, int start_y, int end_x, int end_y)
{
	int kernel[3][3] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};

	for (int i = image->y1 + start_y; i < image->y2 - end_y; i++)
		for (int j = image->x1 + start_x; j < image->x2 - end_x; j++) {
			double r = 0, g = 0, b = 0;

			for (int l = i - 1; l <= i + 1; l++)
				for (int c = j - 1; c <= j + 1; c++) {
					r += image->matrix[l][c].r * kernel[l - i + 1][c - j + 1];
					g += image->matrix[l][c].g * kernel[l - i + 1][c - j + 1];
					b += image->matrix[l][c].b * kernel[l - i + 1][c - j + 1];
				}

			int l = i - (image->y1 + start_y);
			int c = j - (image->x1 + start_x);
			new_matrix[l][c].r = round_and_clamp(r);
			new_matrix[l][c].g = round_and_clamp(g);
			new_matrix[l][c].b = round_and_clamp(b);
		}
}

void apply_blur(rgb_image *image, pixel **new_matrix,
				int start_x, int start_y, int end_x, int end_y)
{
	int kernel[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};

	for (int i = image->y1 + start_y; i < image->y2 - end_y; i++)
		for (int j = image->x1 + start_x; j < image->x2 - end_x; j++) {
			double r = 0, g = 0, b = 0;

			for (int l = i - 1; l <= i + 1; l++)
				for (int c = j - 1; c <= j + 1; c++) {
					r += image->matrix[l][c].r * kernel[l - i + 1][c - j + 1];
					g += image->matrix[l][c].g * kernel[l - i + 1][c - j + 1];
					b += image->matrix[l][c].b * kernel[l - i + 1][c - j + 1];
				}

			int l = i - (image->y1 + start_y);
			int c = j - (image->x1 + start_x);
			new_matrix[l][c].r = round_and_clamp(r / 9);
			new_matrix[l][c].g = round_and_clamp(g / 9);
			new_matrix[l][c].b = round_and_clamp(b / 9);
		}
}

void apply_gaussian_blur(rgb_image *image, pixel **new_matrix,
						 int start_x, int start_y, int end_x, int end_y)
{
	int kernel[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};

	for (int i = image->y1 + start_y; i < image->y2 - end_y; i++)
		for (int j = image->x1 + start_x; j < image->x2 - end_x; j++) {
			double r = 0, g = 0, b = 0;

			for (int l = i - 1; l <= i + 1; l++)
				for (int c = j - 1; c <= j + 1; c++) {
					r += image->matrix[l][c].r * kernel[l - i + 1][c - j + 1];
					g += image->matrix[l][c].g * kernel[l - i + 1][c - j + 1];
					b += image->matrix[l][c].b * kernel[l - i + 1][c - j + 1];
				}

			int l = i - (image->y1 + start_y);
			int c = j - (image->x1 + start_x);
			new_matrix[l][c].r = round_and_clamp(r / 16);
			new_matrix[l][c].g = round_and_clamp(g / 16);
			new_matrix[l][c].b = round_and_clamp(b / 16);
		}
}

void apply(void *image, image_type type, char command[])
{
	if (!image) {
		printf("No image loaded\n");
		return;
	}

	char comm[100], parameter[100];

	if (sscanf(command, "%s%s", comm, parameter) != 2) {
		printf("Invalid command\n");
		return;
	}

	rgb_image *image_color = NULL;

	if (type == P2 || type == P5) {
		printf("Easy, Charlie Chaplin\n");
		return;
	} else if (type == P3 || type == P6)
		image_color = (rgb_image *)image;

	int start_x = 0, start_y = 0, end_x = 0, end_y = 0;
	int new_height = image_color->y2 - image_color->y1;
	int new_width = image_color->x2 - image_color->x1;
	//checking if the selection is at a margin of the image
	if (image_color->x1 == 0) {
		start_x = 1; //leaving the left margin untouched
		new_width--;
	}
	if (image_color->y1 == 0) {
		start_y = 1; //leaving the upper margin untouched
		new_height--;
	}
	if (image_color->x2 == image_color->width) {
		end_x = 1; //leaving the right margin untouched
		new_width--;
	}
	if (image_color->y2 == image_color->height) {
		end_y = 1; //leaving the lower margin untouched
		new_height--;
	}

	pixel **new_matrix = alloc_rgb_matrix(new_height, new_width);

	if (!new_matrix)
		return;

	if (strcmp(parameter, "EDGE") == 0)
		apply_edge(image_color, new_matrix, start_x, start_y, end_x, end_y);
	else if (strcmp(parameter, "SHARPEN") == 0)
		apply_sharpen(image_color, new_matrix, start_x, start_y, end_x, end_y);
	else if (strcmp(parameter, "BLUR") == 0)
		apply_blur(image_color, new_matrix, start_x, start_y, end_x, end_y);
	else if (strcmp(parameter, "GAUSSIAN_BLUR") == 0)
		apply_gaussian_blur(image_color, new_matrix,
							start_x, start_y, end_x, end_y);
	else {
		printf("APPLY parameter invalid\n");
		free_rgb_matrix(new_height, new_matrix);
		return;
	}

	//updating the selection after apply
	for (int i = image_color->y1 + start_y; i < image_color->y2 - end_y; i++)
		for (int j = image_color->x1 + start_x;
			 j < image_color->x2 - end_x; j++) {
			int l = i - (image_color->y1 + start_y);
			int c = j - (image_color->x1 + start_x);
			image_color->matrix[i][j].r = new_matrix[l][c].r;
			image_color->matrix[i][j].g = new_matrix[l][c].g;
			image_color->matrix[i][j].b = new_matrix[l][c].b;
		}

	//freeing the new matrix, we don't use it anymore
	free_rgb_matrix(new_height, new_matrix);

	printf("APPLY %s done\n", parameter);
}

void select(void *image, image_type type, char command[])
{
	if (!image) {
		printf("No image loaded\n");
		return;
	}

	char comm[100];

	int x1, y1, x2, y2;

	if (sscanf(command, "%s%d%d%d%d", comm, &x1, &y1, &x2, &y2) != 5) {
		printf("Invalid command\n");
		return;
	}

	if (x1 > x2) {
		int aux = x1;
		x1 = x2;
		x2 = aux;
	}

	if (y1 > y2) {
		int aux = y1;
		y1 = y2;
		y2 = aux;
	}

	if (x1 < 0 || y1 < 0 || x1 == x2 || y1 == y2)
		printf("Invalid set of coordinates\n");
	else {
		if (type == P2 || type == P5) { //grayscale
			grayscale_image *image_gray = (grayscale_image *)image;
			if (x2 > image_gray->width || y2 > image_gray->height) {
				printf("Invalid set of coordinates\n");
				return;
			}
			image_gray->x1 = x1;
			image_gray->y1 = y1;
			image_gray->x2 = x2;
			image_gray->y2 = y2;
		} else if (type == P3 || type == P6) { //rgb
			rgb_image *image_color = (rgb_image *)image;
			if (x2 > image_color->width || y2 > image_color->height) {
				printf("Invalid set of coordinates\n");
				return;
			}
			image_color->x1 = x1;
			image_color->y1 = y1;
			image_color->x2 = x2;
			image_color->y2 = y2;
		}
		printf("Selected %d %d %d %d\n", x1, y1, x2, y2);
	}
}

void select_all(void *image, image_type type)
{
	if (!image) {
		printf("No image loaded\n");
		return;
	}

	if (type == P2 || type == P5) { //grayscale
		grayscale_image *image_gray = (grayscale_image *)image;
		image_gray->x1 = 0;
		image_gray->y1 = 0;
		image_gray->x2 = image_gray->width;
		image_gray->y2 = image_gray->height;
	} else if (type == P3 || type == P6) { //rgb
		rgb_image *image_color = (rgb_image *)image;
		image_color->x1 = 0;
		image_color->y1 = 0;
		image_color->x2 = image_color->width;
		image_color->y2 = image_color->height;
	}

	printf("Selected ALL\n");
}
