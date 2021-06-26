#define CIF_WRITE
#include "cif.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NO_ZLIB
#include <zlib.h>
#endif /* NO_ZLIB */

#define CIF_MAGIC {0x73, 0x43, 0x49, 0x46, 0x01}
#define CHUNK 16384

typedef struct cif_file {
	char *data;
	size_t size;
	FILE *file;
	void *extra_alloc[100];
	size_t extra_alloc_num;
} cif_file;

#ifndef NO_ZLIB
static int cif_uncompress(size_t in_size, char *in_data, size_t *out_size,
						char **out_data) {
	z_stream stream;
	int ret;
	unsigned char *output_data = NULL;
	size_t output_size = 0;

	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = in_size;
	stream.next_in = (unsigned char *) in_data;

	ret = inflateInit(&stream);
	if(ret != Z_OK)
		return ret;

	do {
		output_size+=CHUNK;

		output_data = realloc(output_data, output_size);
		if(!output_data) {
			inflateEnd(&stream);
			return Z_MEM_ERROR;
		}

		stream.avail_out += CHUNK;
		stream.next_out = output_data + stream.total_out;

		ret = inflate(&stream, Z_NO_FLUSH);
		if(ret < 0) {
			inflateEnd(&stream);
			free(output_data);
			return ret;
		}
	} while(ret != Z_STREAM_END);

	*out_size = stream.total_out; 
	*out_data = (char *) output_data;

	inflateEnd(&stream);

	return Z_OK;
}

static int cif_compress(size_t in_size, char *in_data, size_t *out_size,
						char **out_data) {
	z_stream stream;
	int ret;
	unsigned char *output_data;

	*out_size = 0;
	*out_data = NULL;

	output_data = malloc(in_size);
	if(!output_data)
		return Z_MEM_ERROR;

	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = in_size;
	stream.next_in = (unsigned char *) in_data;
	stream.avail_out = in_size;
	stream.next_out = output_data;

	ret = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
	if(ret != Z_OK) {
		free(output_data);
		return ret;
	}
	
	ret = deflate(&stream, Z_FINISH);

	*out_size = stream.total_out;
	*out_data = (char *) output_data;

	deflateEnd(&stream);

	if(ret != Z_STREAM_END) {
		free(output_data);
		*out_size = 0;
		*out_data = NULL;
		return ret;
	}

	return Z_OK;
}
#endif /* NO_ZLIB */

cif_file *cif_open(const char *path) {
	char new = 0;
	FILE *file;
	char magic_ref[] = CIF_MAGIC;
	char magic[5];
	cif_file *cif;

	if(access(path, F_OK) < 0) {
		new = 1;
	}
	
	file = fopen(path, "a+b");

	if(new) {
		fwrite(magic_ref, 1, 5, file);
		fflush(file);
		fseek(file, 0, SEEK_SET);
	}

	if(!file)
		goto err;

	if(fread(magic, 1, 5, file) != 5)
		goto err_file;

	if(memcmp(magic, magic_ref, 5))
		goto err_file;

	cif = malloc(sizeof(cif_file));
	if(!cif)
		goto err_file;

	cif->file = file;
	cif->extra_alloc_num = 0;

	fseek(file, 0, SEEK_END);
	cif->size = ftell(file)-5;
	fseek(file, 5, SEEK_SET);

	cif->data = malloc(cif->size);
	if(!cif->data)
		goto err_cif;

	if(fread(cif->data, 1, cif->size, file) != cif->size)
		goto err_data;

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
		if(!images) {
			printf("wtf?\n");
			return NULL;
		}

		images[*image_count].name = current;
		current+=256;
		images[*image_count].image_type = *current;
		current+=1;
		images[*image_count].image_format = *current;
		current+=1;
		memcpy(&images[*image_count].width, current, sizeof(uint32_t));
		current+=sizeof(uint32_t);
		memcpy(&images[*image_count].height, current, sizeof(uint32_t));
		current+=sizeof(uint32_t);
		memcpy(&images[*image_count].mipmap_level, current, sizeof(uint32_t));
		current+=sizeof(uint32_t);
		memcpy(&images[*image_count].size, current, sizeof(size_t));
		current+=sizeof(size_t);
		images[*image_count].data = current;
		current+=images[*image_count].size;

#ifndef NO_ZLIB
		if(images[*image_count].image_type >= 127) {
			size_t new_size;
			char *new_data;
			if(cif_uncompress(images[*image_count].size, images[*image_count].data,
								&new_size, &new_data) < 0) {
				fprintf(stderr, "Uncompression error\n");
				free(images);
				return NULL;
			}

			cif->extra_alloc[cif->extra_alloc_num] = new_data;
			cif->extra_alloc_num += 1;

			images[*image_count].size = new_size;
			images[*image_count].data = new_data;
			images[*image_count].image_type -= 127;
		}
#endif /* NO_ZLIB */

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
	fwrite(&image.width, 1, sizeof(uint32_t), cif->file);
	fwrite(&image.height, 1, sizeof(uint32_t), cif->file);
	fwrite(&image.mipmap_level, 1, sizeof(uint32_t), cif->file);

#ifndef NO_ZLIB
	if(image.image_type >= 127) {
		size_t new_size;
		char *new_data;

		if(cif_compress(image.size, image.data, &new_size, &new_data) < 0) {
			fprintf(stderr, "Compression error\n");
			return -1;
		}

		image.size = new_size;
		image.data = new_data;
	}
#endif /* NO_ZLIB */

	fwrite(&image.size, 1, sizeof(size_t), cif->file);
	fwrite(image.data, 1, image.size, cif->file);

#ifndef NO_ZLIB
	if(image.image_format >= 127) {
		free(image.data);
	}
#endif /* NO_ZLIB */

	return 0;
}