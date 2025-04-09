//Marasescu Stefan-Gabriel, 311CA
#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <math.h>

#include "image_loader.h"

#include "image_transforms.h"

void free_rgb_matrix(int pos, pixel **matrix)
{
	if (!matrix)
		return;

	for (int i = 0; i < pos; i++)
		free(matrix[i]);

	free(matrix);
}

void free_grayscale_matrix(int pos, short **matrix)
{
	if (!matrix)
		return;

	for (int i = 0; i < pos; i++)
		free(matrix[i]);

	free(matrix);
}

void free_image(void *image, image_type type)
{
	if (!image)
		return;

	if (type == P3 || type == P6) {//rgb
		rgb_image *image_color = (rgb_image *)image;
		free_rgb_matrix(image_color->height, image_color->matrix);
		free(image_color);
	} else if (type == P2 || type == P5) { //grayscale
		grayscale_image *image_gray = (grayscale_image *)image;
		free_grayscale_matrix(image_gray->height, image_gray->matrix);
		free(image_gray);
	}
}

pixel **alloc_rgb_matrix(int height, int width)
{
	pixel **matrix = (pixel **)malloc(height * sizeof(pixel *));

	if (!matrix)
		return NULL;

	for (int i = 0; i < height; i++) {
		matrix[i] = (pixel *)malloc(width * sizeof(pixel));

		if (!matrix[i]) {
			free_rgb_matrix(i, matrix); //freeing all allocated rows
			return NULL;
		}
	}

	return matrix;
}

short **alloc_grayscale_matrix(int height, int width)
{
	short **matrix = (short **)malloc(height * sizeof(short *));

	if (!matrix)
		return NULL;

	for (int i = 0; i < height; i++) {
		matrix[i] = (short *)malloc(width * sizeof(short));

		if (!matrix[i]) {
			free_grayscale_matrix(i, matrix); //freeing all allocated rows
			return NULL;
		}
	}

	return matrix;
}

void initialize_grayscale_image(grayscale_image *image, char type[],
								int height, int width, short max_value)
{
	strcpy(image->type, type);
	image->height = height;
	image->width = width;
	image->max_value = max_value;
	image->x1 = 0;
	image->y1 = 0;
	image->x2 = width; //the initial selection is the full image
	image->y2 = height;
	image->matrix = alloc_grayscale_matrix(height, width);
}

void initialize_rgb_image(rgb_image *image, char type[],
						  int height, int width, short max_value)
{
	strcpy(image->type, type);
	image->height = height;
	image->width = width;
	image->max_value = max_value;
	image->x1 = 0;
	image->y1 = 0;
	image->x2 = width; //the initial selection is the full image
	image->y2 = height;
	image->matrix = alloc_rgb_matrix(height, width);
}

void *read_image_text(FILE *input_file, char type[],
					  int height, int width, short max_value)
{
	if (strcmp(type, "P2") == 0) { //P2 - grayscale
		grayscale_image *image = (grayscale_image *)
								 malloc(sizeof(grayscale_image));

		if (!image) {
			fclose(input_file);
			return NULL;
		}

		initialize_grayscale_image(image, type, height, width, max_value);

		if (!image->matrix) {
			fclose(input_file);
			free(image);
			return NULL;
		}

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				if (fscanf(input_file, "%hd",
						   &image->matrix[i][j]) != 1) {
					free_grayscale_matrix(height, image->matrix);
					free(image);
					fclose(input_file);
					return NULL;
				}
			}
		}
		return (void *)image;

	} else { //P3 - rgb
		rgb_image *image = (rgb_image *)malloc(sizeof(rgb_image));

		if (!image) {
			fclose(input_file);
			return NULL;
		}

		initialize_rgb_image(image, type, height, width, max_value);

		if (!image->matrix) {
			fclose(input_file);
			free(image);
			return NULL;
		}

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				if (fscanf(input_file, "%hd%hd%hd",
						   &image->matrix[i][j].r, &image->matrix[i][j].g,
						   &image->matrix[i][j].b) != 3) {
					free_rgb_matrix(height, image->matrix);
					free(image);
					fclose(input_file);
					return NULL;
				}
			}
		}
		return (void *)image;
	}
}

void *read_image_binary(char file_name[], long pos, char type[],
						int height, int width, short max_value)
{
	FILE *input_file = fopen(file_name, "rb");

	if (!input_file)
		return NULL;

	fseek(input_file, pos, SEEK_SET);

	if (strcmp(type, "P5") == 0) { //P5 - grayscale
		grayscale_image *image = (grayscale_image *)
		malloc(sizeof(grayscale_image));

		if (!image) {
			fclose(input_file);
			return NULL;
		}

		initialize_grayscale_image(image, type, height, width, max_value);
		if (!image->matrix) {
			fclose(input_file);
			free(image);
			return NULL;
		}

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				/*reading with size of char, because each pixel value
				is represented as one byte in the binary file*/
				unsigned char pix;
				if (fread(&pix, sizeof(unsigned char), 1, input_file) != 1) {
					free_grayscale_matrix(height, image->matrix);
					free(image);
					fclose(input_file);
					return NULL;
				} //in order to update all bytes of the short variable
				image->matrix[i][j] = (short)pix;
			}
		}
		return (void *)image;

	} else { //P6 - rgb
		rgb_image *image = (rgb_image *)malloc(sizeof(rgb_image));

		if (!image) {
			fclose(input_file);
			return NULL;
		}

		initialize_rgb_image(image, type, height, width, max_value);
		if (!image->matrix) {
			fclose(input_file);
			free(image);
			return NULL;
		}

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				short check = 0;
				/*reading with size of char, because each pixel value
				is represented as one byte in the binary file*/
				unsigned char pix;
				check += fread(&pix, sizeof(unsigned char), 1, input_file);
				image->matrix[i][j].r = (short)pix;
				check += fread(&pix, sizeof(unsigned char), 1, input_file);
				image->matrix[i][j].g = (short)pix;
				check += fread(&pix, sizeof(unsigned char), 1, input_file);
				image->matrix[i][j].b = (short)pix;
				if (check != 3) {
					free_rgb_matrix(height, image->matrix);
					free(image);
					fclose(input_file);
					return NULL;
				}
			}
		}
		return (void *)image;
	}
}

void *load_file(image_type *type, char command[])
{
	*type = INVALID;

	char comm[100], file_name[100];

	if (sscanf(command, "%s%s", comm, file_name) != 2)
		return NULL;

	/* opening the file as a text file, and
	only reading the first line image type/format */
	FILE *input_file = fopen(file_name, "rt");

	if (!input_file) {
		printf("Failed to load %s\n", file_name);
		return NULL;
	}

	char imgtype[3];

	fscanf(input_file, "%s", imgtype);

	int height, width, max_value;

	fscanf(input_file, "%d%d%d", &width, &height, &max_value);

	void *image = NULL;

	if (strcmp(imgtype, "P2") == 0) {
		*type = P2;
		image = read_image_text(input_file, imgtype, height, width, max_value);
	} else if (strcmp(imgtype, "P3") == 0) {
		*type = P3;
		image = read_image_text(input_file, imgtype, height, width, max_value);
	} else if (strcmp(imgtype, "P5") == 0) {
		*type = P5;
		image = read_image_binary(file_name, ftell(input_file) + 1,
								  imgtype, height, width, max_value);
	} else if (strcmp(imgtype, "P6") == 0) {
		*type = P6;
		image = read_image_binary(file_name, ftell(input_file) + 1,
								  imgtype, height, width, max_value);
	}

	if (!image) {
		printf("Failed to load %s\n", file_name);
		type = INVALID;
		return NULL;
	}

	fclose(input_file);

	printf("Loaded %s\n", file_name);

	return image;
}

int print_image_text(void *image, image_type type, char file_name[])
{
	FILE *output_file = fopen(file_name, "wt");

	if (!output_file)
		return 1;

	if (type == P2 || type == P5) { //grayscale
		grayscale_image *image_gray = (grayscale_image *)image;
		fprintf(output_file, "P2\n"); //the saved image will be P2 format
		fprintf(output_file, "%d %d\n", image_gray->width, image_gray->height);
		fprintf(output_file, "%hd\n", image_gray->max_value);

		for (int i = 0; i < image_gray->height; i++) {
			for (int j = 0; j < image_gray->width; j++)
				fprintf(output_file, "%hd ", image_gray->matrix[i][j]);

			fprintf(output_file, "\n");
		}

	} else if (type == P3 || type == P6) { //rgb
		rgb_image *image_color = (rgb_image *)image;
		fprintf(output_file, "P3\n"); //the saved image will be P3 format
		fprintf(output_file, "%d %d\n",
				image_color->width, image_color->height);
		fprintf(output_file, "%hd\n", image_color->max_value);

		for (int i = 0; i < image_color->height; i++) {
			for (int j = 0; j < image_color->width; j++)
				fprintf(output_file, "%hd %hd %hd\t",
						image_color->matrix[i][j].r,
						image_color->matrix[i][j].g,
						image_color->matrix[i][j].b);

			fprintf(output_file, "\n");
		}
	}
	fclose(output_file);
	return 0; //success
}

int print_image_binary(void *image, image_type type, char file_name[])
{
	FILE *output_file = fopen(file_name, "wt");

	if (!output_file)
		return 1;

	if (type == P2 || type == P5) { //grayscale
		grayscale_image *image_gray = (grayscale_image *)image;
		fprintf(output_file, "P5\n"); //the saved image will be P5 format
		fprintf(output_file, "%d %d\n", image_gray->width, image_gray->height);
		fprintf(output_file, "%hd\n", image_gray->max_value);

		fclose(output_file);

		output_file = fopen(file_name, "ab");
		//appending binary starting from the 4th line

		if (!output_file)
			return 1;

		for (int i = 0; i < image_gray->height; i++)
			for (int j = 0; j < image_gray->width; j++)
				fwrite(&image_gray->matrix[i][j], sizeof(char), 1, output_file);

	} else if (type == P3 || type == P6) { //rgb
		rgb_image *image_color = (rgb_image *)image;
		fprintf(output_file, "P6\n"); //the saved image will be P6 format
		fprintf(output_file, "%d %d\n",
				image_color->width, image_color->height);
		fprintf(output_file, "%hd\n", image_color->max_value);

		fclose(output_file);

		output_file = fopen(file_name, "ab");
		//appending binary starting from the 4th line

		if (!output_file)
			return 1;

		for (int i = 0; i < image_color->height; i++)
			for (int j = 0; j < image_color->width; j++) {
				fwrite(&image_color->matrix[i][j].r,
					   sizeof(char), 1, output_file);
				fwrite(&image_color->matrix[i][j].g,
					   sizeof(char), 1, output_file);
				fwrite(&image_color->matrix[i][j].b,
					   sizeof(char), 1, output_file);
			}
	}
	fclose(output_file);
	return 0;
}

void save_image(void *image, image_type type, char command[])
{
	if (!image) {
		printf("No image loaded\n");
		return;
	}

	char comm[100], file_name[100], file_type[10];

	int parameter_num = sscanf(command, "%s%s%s", comm, file_name, file_type);

	if (parameter_num == 2) { //binary
		if (print_image_binary(image, type, file_name) == 1)
			return; //something went wrong
	} else if (parameter_num == 3 && strcmp(file_type, "ascii") == 0) { //text
		if (print_image_text(image, type, file_name) == 1)
			return; //something went wrong
	} else
		return;

	printf("Saved %s\n", file_name);
}
