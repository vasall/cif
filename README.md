# Clusterwerk Image Format
CIF is a file format, which can contain one or more different images with
different formats including mip mapping. It is designed for use in game engines

For information on the file format, see [File_format.md](File_format.md)

## Content
This repository contains:
- The [file format specification](File_format.md)
- libcif, a library for reading and writing cif files
- cif, a program for creating and modifying cif files

## Dependencies
libcif has the dependency `zlib`. You can disable it by declaring `NO_ZLIB=1` when building

cif has the additional dependency `libpng`
## Building
There are four build targets: `bin`, `shared`, `static` and `all`
```sh
make [NO_ZLIB=1] <all|bin|static|shared>
```
## cif
```sh
Usage: cif <operation> [options] <cif-file> [image...]

Operations are:
        -h Show this help
        -l List images in cif file
        -a Add image[s] to cif file
        -d Delete image[s] from cif file

Options are:
        -c Compress newly added images
        -v Show verbose output
```
## API
See [cif.h](src/cif.h)
## Example
```c
#include <stdio.h>
#include <stdlib.h>

#include "cif.h"

int main(void)
{
	cif_file *cif;
	size_t image_count;
	cif_image *images;
	unsigned int i;

	cif = cif_open("test.cif");
	if(!cif)
		return EXIT_FAILURE;

	images = cif_get_images(cif, &image_count);
	if(!images) {
		cif_clean(cif);
		return EXIT_FAILURE;
	}

	for(i = 0; i < image_count; i++) {
		printf("%s: %ux%upx\n", images[i].name, images[i].width, images[i].height);
	}

	cif_clean(cif);

	return EXIT_SUCCESS;
}
```
## License
While the format has no copyright restrictions (see [File_format.md](File_format.md)) the software is licensed under the terms of the zlib license. See [LICENSE](LICENSE) for more details