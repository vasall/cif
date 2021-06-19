#define CIF_WRITE
#include "cif.h"

int main(void) {
	char data[] = {'A', 'A', 'A'};
	cif_image image;
	cif_file *file;

	image.name = "Test";
	image.image_type = CIF_IMAGE_TYPE_ALBEDO_COMPRESSED;
	image.image_format = CIF_IMAGE_FORMAT_R32G32B32A32;
	image.width = 800;
	image.height = 600;
	image.mipmap_level = 0;
	image.size = 3;
	image.data = data;

	file = cif_create("test.cif");
	if(!file)
		return 1;

	cif_write_image(file, image);
	cif_close(file);
	return 0;
}
