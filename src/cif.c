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
	void *extra_alloc[100];
	size_t extra_alloc_num;
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
	cif->extra_alloc_num = 0;

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
	int err;

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

		if(images[*image_count].image_format >= 127) {
			z_stream stream;
			unsigned char *data = NULL;
			size_t size;

			size = images[*image_count].size;

			stream.zalloc = Z_NULL;
			stream.zfree = Z_NULL;
			stream.opaque = Z_NULL;
			stream.avail_in = images[*image_count].size;
			stream.next_in = images[*image_count].data;
			stream.total_in = 0;
			stream.total_out = 0;

			err = inflateInit(&stream);

			if(err != Z_OK) {
				fprintf(stderr, "Compression error %s\n", zError(err));
				free(images);
				return NULL;
			}

			while(stream.avail_in) {
				size *= 2;
				data = realloc(data, size);
				if(!data) {
					free(images);
					return NULL;
				}
				stream.avail_out = size-stream.total_out;
				stream.next_out = data + stream.total_in;
				err = inflate(&stream, Z_NO_FLUSH);
				if(err < 0) {
					free(data);
					fprintf(stderr, "Compression error %s\n", zError(err));
					free(images);
					return NULL;
				}
			}

			cif->extra_alloc[cif->extra_alloc_num] = data;
			cif->extra_alloc_num += 1;

			images[*image_count].size = stream.total_out;
			images[*image_count].data = data;

			inflateEnd(&stream);
			
		}

		*image_count+=1;
	}
	return images;
}

int cif_clean(cif_file *cif) {
	size_t i;

	if(cif->file)
		fclose(cif->file);
	for(i = 0; i < cif->extra_alloc_num; i++) {
		free(cif->extra_alloc[i]);
	}
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
		z_stream stream;
		unsigned char *data;
		int err;

		data = malloc(image.size);
		if(!data)
			return -1;

		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;
		stream.avail_in = image.size;
		stream.next_in = image.data;
		stream.avail_out = image.size;
		stream.next_out = data;

		err = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
		if(err != Z_OK) {
			fprintf(stderr, "Compression error %s\n", zError(err));
			return -1;
		}
		err = deflate(&stream, Z_FINISH);
		if(err < 0) {
			fprintf(stderr, "Compression error %s\n", zError(err));
			return -1;
		}

		fwrite(&stream.total_out, 1, sizeof(size_t), cif->file);
		fwrite(data, 1, stream.total_out, cif->file);

		deflateEnd(&stream);

		free(data);
	} else {
		fwrite(&image.size, 1, sizeof(size_t), cif->file);
		fwrite(image.data, 1, image.size, cif->file);
	}

	return 0;
}