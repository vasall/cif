#define CIF_WRITE
#include "cif.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <zlib.h>

#define CIF_MAGIC {0x73, 0x43, 0x49, 0x46, 0x01}

typedef struct cif_file {
	char *data;
	size_t size;
	FILE *file;
} cif_file;

cif_file *cif_load(const char *path) {
	FILE *file;
	char magic_ref[] = CIF_MAGIC;
	char magic[5];
	cif_file *cif;
	
	file = fopen(path, "rb");

	if(!file)
		goto err;

	if(fread(magic, 1, 5, file) != 5)
		goto err_file;

	if(memcmp(magic, magic_ref, 5))
		goto err_file;

	cif = malloc(sizeof(cif_file));
	if(!cif)
		goto err_file;

	cif->file = NULL;

	fseek(file, 0, SEEK_END);
	cif->size = ftell(file)-5;
	fseek(file, 5, SEEK_SET);

	cif->data = malloc(cif->size);
	if(!cif->data)
		goto err_cif;

	if(fread(cif->data, 1, cif->size, file) != cif->size)
		goto err_data;

	fclose(file);

	return cif;

err_data:
	free(cif->data);
err_cif:
	free(cif);
err_file:
	fclose(file);
err:
	return NULL;
}

cif_image *cif_get_images(cif_file *cif, size_t *image_count) {
	char *current;
	cif_image *images;

	if(!cif->size)
		return NULL;

	images = NULL;
	*image_count = 0;
	current = cif->data;

	while((size_t)(current-cif->data) != cif->size) {
		if(images) {
			images = realloc(images, sizeof(cif_image) * (*image_count+1));
		} else {
			images = malloc(sizeof(cif_image));
		}
		if(!images)
			return NULL;

		images[*image_count].name = current;
		current+=256;
		images[*image_count].image_type = *current;
		current++;
		images[*image_count].image_format = *current;
		current++;
		images[*image_count].width = (unsigned int)*current;
		current+=sizeof(unsigned int);
		images[*image_count].height = (unsigned int)*current;
		current+=sizeof(unsigned int);
		images[*image_count].mipmap_level = (unsigned int)*current;
		current+=sizeof(unsigned int);
		images[*image_count].size = (size_t)*current;
		current+=sizeof(size_t);
		images[*image_count].data = current;
		current+= images[*image_count].size;

		*image_count+=1;
	}
	return images;
}

int cif_clean(cif_file *cif) {

	if(cif->file)
		fclose(cif->file);
	free(cif->data);
	free(cif);
	return 0;
}

cif_file *cif_create(const char *path) {
	cif_file *cif;
	char magic[] = CIF_MAGIC;

	cif = malloc(sizeof(cif_file));
	if(!cif)
		goto err;

	cif->size = 0;
	cif->data = NULL;

	cif->file = fopen(path, "wb");
	if(!cif->file)
		goto err_cif;

	fwrite(magic, 1, 5, cif->file);

	return cif;

err_cif:
	free(cif);
err:
	return NULL;
}

int cif_write_image(cif_file *cif, cif_image image) {
	char name[256];
	char tmp;

	if(!cif->file)
		return -1;

	memset(name, 0, 256);
	strncpy(name, image.name, 256);

	fwrite(name, 1, 256, cif->file);
	tmp = image.image_type;
	fwrite(&tmp, 1, 1, cif->file);
	tmp = image.image_format;
	fwrite(&tmp, 1, 1, cif->file);
	fwrite(&image.width, 1, sizeof(unsigned int), cif->file);
	fwrite(&image.height, 1, sizeof(unsigned int), cif->file);
	fwrite(&image.mipmap_level, 1, sizeof(unsigned int), cif->file);

	if(image.image_type >= 127) {
		
	} else {
		fwrite(&image.size, 1, sizeof(size_t), cif->file);
		fwrite(image.data, 1, image.size, cif->file);
	}

	return 0;
}