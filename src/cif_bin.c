#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CIF_WRITE
#include "cif.h"

static char *get_image_type_str(enum cif_image_type type) {
	switch (type) {
		case CIF_IMAGE_TYPE_ALBEDO:
			return "CIF_IMAGE_TYPE_ALBEDO";
		case CIF_IMAGE_TYPE_ROUGHNESS:
			return "CIF_IMAGE_TYPE_ROUGHNESS";
		case CIF_IMAGE_TYPE_METALLIC:
			return "CIF_IMAGE_TYPE_METALLIC";
		case CIF_IMAGE_TYPE_NORMAL:
			return "CIF_IMAGE_TYPE_NORMAL";
		case CIF_IMAGE_TYPE_AO:
			return "CIF_IMAGE_TYPE_AO";
		case CIF_IMAGE_TYPE_CUBE_PX:
			return "CIF_IMAGE_TYPE_CUBE_PX";
		case CIF_IMAGE_TYPE_CUBE_NX:
			return "CIF_IMAGE_TYPE_CUBE_NX";
		case CIF_IMAGE_TYPE_CUBE_PY:
			return "CIF_IMAGE_TYPE_CUBE_PY";
		case CIF_IMAGE_TYPE_CUBE_NY:
			return "CIF_IMAGE_TYPE_CUBE_NY";
		case CIF_IMAGE_TYPE_CUBE_PZ:
			return "CIF_IMAGE_TYPE_CUBE_PZ";
		case CIF_IMAGE_TYPE_CUBE_NZ:
			return "CIF_IMAGE_TYPE_CUBE_NZ";
		case CIF_IMAGE_TYPE_ENVIRONMENT:
			return "CIF_IMAGE_TYPE_ENVIRONMENT";
		case CIF_IMAGE_TYPE_ALBEDO_COMPRESSED:
			return "CIF_IMAGE_TYPE_ALBEDO_COMPRESSED";
		case CIF_IMAGE_TYPE_ROUGHNESS_COMPRESSED:
			return "CIF_IMAGE_TYPE_ROUGHNESS_COMPRESSED";
		case CIF_IMAGE_TYPE_METALLIC_COMPRESSED:
			return "CIF_IMAGE_TYPE_METALLIC_COMPRESSED";
		case CIF_IMAGE_TYPE_NORMAL_COMPRESSED:
			return "CIF_IMAGE_TYPE_NORMAL_COMPRESSED";
		case CIF_IMAGE_TYPE_AO_COMPRESSED:
			return "CIF_IMAGE_TYPE_AO_COMPRESSED";
		case CIF_IMAGE_TYPE_CUBE_PX_COMPRESSED:
			return "CIF_IMAGE_TYPE_CUBE_PX_COMPRESSED";
		case CIF_IMAGE_TYPE_CUBE_NX_COMPRESSED:
			return "CIF_IMAGE_TYPE_CUBE_NX_COMPRESSED";
		case CIF_IMAGE_TYPE_CUBE_PY_COMPRESSED:
			return "CIF_IMAGE_TYPE_CUBE_PY_COMPRESSED";
		case CIF_IMAGE_TYPE_CUBE_NY_COMPRESSED:
			return "CIF_IMAGE_TYPE_CUBE_NY_COMPRESSED";
		case CIF_IMAGE_TYPE_CUBE_PZ_COMPRESSED:
			return "CIF_IMAGE_TYPE_CUBE_PZ_COMPRESSED";
		case CIF_IMAGE_TYPE_CUBE_NZ_COMPRESSED:
			return "CIF_IMAGE_TYPE_CUBE_NZ_COMPRESSED";
		case CIF_IMAGE_TYPE_ENVIRONMENT_COMPRESSED:
			return "CIF_IMAGE_TYPE_ENVIRONMENT_COMPRESSED";
	}
	return "";
}

static char *get_image_format_str(enum cif_image_format format) {
	switch (format) {
		case CIF_IMAGE_FORMAT_R8G8B8:
			return "CIF_IMAGE_FORMAT_R8G8B8";
		case CIF_IMAGE_FORMAT_R8G8B8A8:
			return "CIF_IMAGE_FORMAT_R8G8B8A8";
		case CIF_IMAGE_FORMAT_R32G32B32:
			return "CIF_IMAGE_FORMAT_R32G32B32";
		case CIF_IMAGE_FORMAT_R32G32B32A32:
			return "CIF_IMAGE_FORMAT_R32G32B32A32";
	}
	return "";
}

static void print_interactive_help() {
	return;
}

static void list_images(size_t image_count, cif_image *images) {
	unsigned int i;

	for(i = 0; i < image_count; i++) {
		printf("%s\n", images[i].name);
		printf("\t%s\n", get_image_type_str(images[i].image_type));
		printf("\t%s\n", get_image_format_str(images[i].image_format));
		printf("\t%ux%upx\n", images[i].width, images[i].height);
		printf("\tMipmap level: %u\n", images[i].mipmap_level);
		printf("\tSize: %ld\n\n", images[i].size);
	}
}

static void add_image(const char *file) {
	(void) file;
	return;
}

static void print_help(const char *prog_name) {
	printf("Usage: %s [options] <file>\n\n", prog_name);
	printf("If no options are provided, the program will start the interactive mode\n");
	printf("Options are:\n");
	printf("\t-h Show this help\n");
	printf("\t-l List images in file\n");
}

int main(int argc, char *argv[])
{
	int opt;
	int list = 0;
	cif_file *cif;
	size_t image_count;
	cif_image *images;
	char cmd[100];

	while((opt = getopt(argc, argv, "hl")) != -1) {
		switch(opt) {
			case 'h':
				print_help(argv[0]);
				exit(EXIT_SUCCESS);
				break;
			case 'l':
				list = 1;
				break;
			default:
				print_help(argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if(optind >= argc) {
		fprintf(stderr, "Expected file\n\n");
		print_help(argv[0]);
		exit(EXIT_FAILURE);
	}

	cif = cif_load(argv[optind]);
	if(!cif) {
		cif = cif_create(argv[optind]);
		if(!cif) {
			fprintf(stderr, "Couldn't create file %s\n", argv[optind]);
			exit(EXIT_FAILURE);
		}
	}

	images = cif_get_images(cif, &image_count);

	if(list) {
		list_images(image_count, images);
		exit(EXIT_SUCCESS);
	}

	/* Go into interactive mode */
	printf(">");
	fflush(stdout);
	while(fgets(cmd, 100, stdin) != NULL) {
		if(!strcmp(cmd, "quit\n")) {
			break;
		} else if(!strcmp(cmd, "help\n")) {
			print_interactive_help();
		} else if(!strcmp(cmd, "list\n")) {
			list_images(image_count, images);
		} else if(!strncmp(cmd, "add ", 4)) {
			add_image(cmd+4);
		} else if(cmd[0] != '\n') {
			printf("Unrecognized command. Type 'help' for list of available commands\n");
		}

		printf(">");
		fflush(stdout);
	}

	cif_clean(cif);
	return 0;
}
