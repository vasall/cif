#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <png.h>

#define CIF_WRITE
#include "cif.h"

#define CHECK_OP(op) if(op != OP_NULL) {\
	fprintf(stderr, "You can do only one operation per command\n");\
	print_help(argv[0]);\
	exit(EXIT_FAILURE); }

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

static void add_images(cif_file *cif, int argc, char *argv[]) {
	unsigned int i;
	size_t image_count;

	if(argc <= optind+1) {
		fprintf(stderr, "No image files specified\n");
		exit(EXIT_FAILURE);
	}

	image_count = argc - optind - 1;

	printf("Please select the image type. Possible values are:\n");
	printf("Albedo (normal): 0\n");
	printf("Roughness map: 1\n");
	printf("Metallic map: 2\n");
	printf("Normal map: 3\n");
	printf("AO: 4\n");
	printf("Cube map positive x: 5\n");
	printf("Cube map negative x: 6\n");
	printf("Cube map positive y: 7\n");
	printf("Cube map negative y: 8\n");
	printf("Cube map positive z: 9\n");
	printf("Cube map negative z: 10\n");
	printf("Environment map: 11\n");

	for(i = 0; i < image_count; i++) {
		png_image png;
		char *data;
		cif_image image;
		char *extension;
		char *image_name;
		unsigned int index = argc - optind + 1 + i;
		char type[3];
		int ty;

		memset(&png, 0, sizeof(png_image));
		png.version = PNG_IMAGE_VERSION;
		png.opaque = NULL;

		if(png_image_begin_read_from_file(&png, argv[index]) <= 0) {
			fprintf(stderr, "%s\n", png.message);
			png_image_free(&png);
			exit(EXIT_FAILURE);
		}
		png.format = PNG_FORMAT_RGBA;
		data = malloc(PNG_IMAGE_SIZE(png));
		if(!data) {
			png_image_free(&png);
			fprintf(stderr, "Memory allocation failed\n");
			exit(EXIT_FAILURE);
		}
		if(png_image_finish_read(&png, NULL, data, 0, NULL) <= 0) {
			png_image_free(&png);
			free(data);
			fprintf(stderr, "%s\n", png.message);
			exit(EXIT_FAILURE);
		}
		png_image_free(&png);

		image_name = strdup(argv[index]);
		extension = strrchr(image_name, '.');
		extension[0] = '\0';

		image.name = basename(image_name);
		image.image_type = CIF_IMAGE_TYPE_ALBEDO;
		image.image_format = CIF_IMAGE_FORMAT_R8G8B8A8;
		image.width = png.width;
		image.height = png.height;
		image.mipmap_level = 0;
		image.size = PNG_IMAGE_SIZE(png);
		image.data = data;

		printf("Please select the image type for %s:", image.name);
		fflush(stdout);
		fgets(type, 3, stdin);
		type[2] = '\0';
		ty = atoi(type);

		image.image_type = ty;

		if(cif_write_image(cif, image) < 0) {
			free(data);
			free(image_name);
			fprintf(stderr, "Couldn't write image %s\n", image.name);
			exit(EXIT_FAILURE);
		}
		
		free(data);
		free(image_name);
	}
}

static int replace(const char *old_file, const char *new_file) {
	struct stat old_stat, new_stat;
	int old_fd, new_fd;

	if(stat(old_file, &old_stat) < 0)
		return -1;
	if(stat(new_file, &new_stat) < 0)
		return -1;

	if(remove(old_file) < 0) {
		perror("");
		return -1;
	}

	old_fd = open(new_file, O_RDONLY);
	if(old_fd < 0)
		return -1;
	new_fd = open(old_file, O_CREAT | O_WRONLY, old_stat.st_mode);
	if(new_fd < 0)
		return -1;

	if(sendfile(new_fd, old_fd, NULL, new_stat.st_size) < 0)
		return -1;

	close(old_fd);
	close(new_fd);

	if(remove(new_file) < 0) {
		perror("");
		return -1;
	}

	return 0;
}

static void del_images(char *cif_path, int argc, char *argv[], size_t image_count, cif_image *images) {
	unsigned int i;
	cif_file *cif = cif_open("/tmp/tmp.cif");

	if(!cif) {
		fprintf(stderr, "What?\n");
		exit(EXIT_FAILURE);
	}

	for(i = 0; i < image_count; i++) {
		int j;
		int removable = 0;
		
		for(j = 0; j < (argc - optind - 1); j++) {
			int index = argc - optind + 1 + j;
			if(!strcmp(images[i].name, argv[index])) {
				removable = 1;
				break;
			}
		}

		if(!removable) {
			cif_write_image(cif, images[i]);
		}
	}

	cif_clean(cif);

	replace(cif_path, "/tmp/tmp.cif");
}

static void print_help(const char *prog_name) {
	printf("Usage: %s <operation> [options] <cif-file> [image...]\n\n", prog_name);
	printf("Operations are:\n");
	printf("\t-h Show this help\n");
	printf("\t-l List images in cif file\n");
	printf("\t-a Add image[s] to cif file\n");
	printf("\t-d Delete image[s] from cif file\n\n");
	printf("Options are:\n");
	printf("\t-c Compress newly added images\n");
	printf("\t-m Generate mip maps for new images\n");
	printf("\t-v Show verbose output\n");
}

enum operation {
	OP_NULL,
	OP_LIST,
	OP_ADD,
	OP_DEL
};

int main(int argc, char *argv[]) {
	int opt;
	enum operation op = OP_NULL;
	char verbose = 0;
	char compress = 0;
	char mipmap = 0;
	cif_file *cif;
	size_t image_count;
	cif_image *images;

	while((opt = getopt(argc, argv, "hladcmv")) != -1) {
		switch (opt) {
			case 'h':
				print_help(argv[0]);
				exit(EXIT_SUCCESS);
				break;
			case 'l':
				CHECK_OP(op);
				op = OP_LIST;
				break;
			case 'a':
				CHECK_OP(op);
				op = OP_ADD;
				break;
			case 'd':
				CHECK_OP(op);
				op = OP_DEL;
				break;
			case 'c':
				compress = 1;
				break;
			case 'm':
				mipmap = 1;
				break;
			case 'v':
				verbose = 1;
				break;
			case '?':
				print_help(argv[0]);
				exit(EXIT_FAILURE);
				break;
		}
	}

	if(optind >= argc) {
		fprintf(stderr, "Expected cif file after options\n");
		print_help(argv[0]);
		exit(EXIT_FAILURE);
	}

	cif = cif_open(argv[optind]);
	if(!cif) {
		fprintf(stderr, "Couldn't create file %s\n", argv[optind]);
		exit(EXIT_FAILURE);
	}

	images = cif_get_images(cif, &image_count);
	if(!images && image_count) {
		fprintf(stderr, "Why aren't there images?\n");
		exit(EXIT_FAILURE);
	}

	switch (op) {
		case OP_NULL:
			fprintf(stderr, "Please specify an operation\n");
			print_help(argv[0]);
			exit(EXIT_FAILURE);
			break;

		case OP_LIST:
			list_images(image_count, images);
			break;

		case OP_ADD:
			add_images(cif, argc, argv);
			break;

		case OP_DEL:
			cif_clean(cif);
			del_images(argv[optind], argc, argv, image_count, images);
			break;
	}

	free(images);
	cif_clean(cif);
	return 0;
}
