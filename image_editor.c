//Marasescu Stefan-Gabriel, 311CA
#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <math.h>

#include "image_loader.h"

#include "image_transforms.h"

int main(void)
{
	void *image = NULL;

	image_type type = INVALID;

	while (1) {
		char command[255];

		fgets(command, sizeof(command), stdin);

		if (strncmp(command, "LOAD", 4) == 0) {
			free_image(image, type);
			image = load_file(&type, command);
		} else if (strncmp(command, "SELECT ALL", 10) == 0)
			select_all(image, type);
		else if (strncmp(command, "SELECT", 6) == 0)
			select(image, type, command);
		else if (strncmp(command, "HISTOGRAM", 9) == 0)
			histogram(image, type, command);
		else if (strncmp(command, "EQUALIZE", 8) == 0)
			equalize_image(image, type);
		else if (strncmp(command, "ROTATE", 6) == 0)
			rotate_selection(image, type, command);
		else if (strncmp(command, "CROP", 4) == 0) {
			if (type == P2 || type == P5)
				image = crop_grayscale_image(image);
			else if (type == P3 || type == P6)
				image = crop_rgb_image(image);
			else if (!image)
				printf("No image loaded\n");
		} else if (strncmp(command, "APPLY", 5) == 0)
			apply(image, type, command);
		else if (strncmp(command, "SAVE", 4) == 0)
			save_image(image, type, command);
		else if (strncmp(command, "EXIT", 4) == 0) {
			if (!image)
				printf("No image loaded");
			break;

		} else
			printf("Invalid command\n");

	}

	free_image(image, type);

	return 0;
}
