/*
zlib License

(C) 2021 clusterwerk

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef CIF_H
#define CIF_H

#include <stddef.h>
#include <stdint.h>

typedef struct cif_file cif_file;

enum cif_image_type {
	CIF_IMAGE_TYPE_ALBEDO = 0,
	CIF_IMAGE_TYPE_ROUGHNESS = 1,
	CIF_IMAGE_TYPE_METALLIC = 2,
	CIF_IMAGE_TYPE_NORMAL = 3,
	CIF_IMAGE_TYPE_AO = 4,
	CIF_IMAGE_TYPE_CUBE_PX = 5,
	CIF_IMAGE_TYPE_CUBE_NX = 6,
	CIF_IMAGE_TYPE_CUBE_PY = 7,
	CIF_IMAGE_TYPE_CUBE_NY = 8,
	CIF_IMAGE_TYPE_CUBE_PZ = 9,
	CIF_IMAGE_TYPE_CUBE_NZ = 10,
	CIF_IMAGE_TYPE_ENVIRONMENT = 11,

	CIF_IMAGE_TYPE_ALBEDO_COMPRESSED = 127,
	CIF_IMAGE_TYPE_ROUGHNESS_COMPRESSED = 128,
	CIF_IMAGE_TYPE_METALLIC_COMPRESSED = 129,
	CIF_IMAGE_TYPE_NORMAL_COMPRESSED = 130,
	CIF_IMAGE_TYPE_AO_COMPRESSED = 131,
	CIF_IMAGE_TYPE_CUBE_PX_COMPRESSED = 132,
	CIF_IMAGE_TYPE_CUBE_NX_COMPRESSED = 133,
	CIF_IMAGE_TYPE_CUBE_PY_COMPRESSED = 134,
	CIF_IMAGE_TYPE_CUBE_NY_COMPRESSED = 135,
	CIF_IMAGE_TYPE_CUBE_PZ_COMPRESSED = 136,
	CIF_IMAGE_TYPE_CUBE_NZ_COMPRESSED = 137,
	CIF_IMAGE_TYPE_ENVIRONMENT_COMPRESSED = 138
};

enum cif_image_format {
	CIF_IMAGE_FORMAT_R8G8B8 = 0,
	CIF_IMAGE_FORMAT_R8G8B8A8 = 1,
	CIF_IMAGE_FORMAT_R32G32B32 = 2,
	CIF_IMAGE_FORMAT_R32G32B32A32 = 3
};

typedef struct cif_image {
	char *name;
	enum cif_image_type image_type;
	enum cif_image_format image_format;
	uint32_t width;
	uint32_t height;
	uint32_t mipmap_level;
	size_t size;
	char *data;
} cif_image;

/**
 * Open or create a cif file and load its content into memory
 * 
 * @path: The path to the cif file
 * 
 * Returns the cif_file handle or NULL if an error occured
 */
cif_file *cif_open(const char *path);

/**
 * Get the images of the cif file
 * 
 * @cif: The cif_file handle returned by cif_open()
 * @image_count: The amount of images. Will be filled by the function
 * 
 * Returns an allocated array of cif_image structs or NULL if an error occured
 */
cif_image *cif_get_images(cif_file *cif, size_t *image_count);

/**
 * Free up any allocated space and close the cif file
 * 
 * @cif: The cif_file handle
 * 
 * Returns 0 on success or -1 if an error occured
 */
int cif_clean(cif_file *cif);

/**
 * Write a new image to the cif file
 * 
 * @cif: The cif_file handle
 * @image: The cif_image struct of the new image
 * 
 * Returns 0 on success or -1 if an error occured
 */
int cif_write_image(cif_file *cif, cif_image image);

#endif /* CIF_H */