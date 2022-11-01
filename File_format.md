# Clusterfuck Image Format
CIF is a file format, which can contain one or more different images with
different formats including mip mapping. It is designed for use in game engines

The file should have the file extension `.cif`
## Header
Every `.cif` file has to start with the magic
```hex
0x73 0x43 0x49 0x46 
```
Appending that the version of the file format
```hex
0x01
```

After that, the file consists of an array of images with a special header
```c
struct image_header {
	char name[256];
	char image_type;
	char image_format;
	uint32_t width;
	uint32_t height;
	uint32_t mipmap_level;
	size_t size;
};
```
Immediately afterwards, the image data follows
- `name` is the name of the image
- `image_type` is the type of the image. See [Image Types](#image-types) for more details
- `image_format` is the format of the image. See [Image Formats](#image-formats) for more details
- `width` is the width of the image
- `height` is the height of the image
- `mipmap_level` is the mipmap level of the image. The original IMage should have level 0
- `size` is the size of the following image data
## Image Types
```
IMAGE_TYPE_ALBEDO = 0
IMAGE_TYPE_ROUGHNESS = 1
IMAGE_TYPE_METALLIC = 2
IMAGE_TYPE_NORMAL = 3
IMAGE_TYPE_AO = 4
IMAGE_TYPE_CUBE_PX = 5
IMAGE_TYPE_CUBE_NX = 6
IMAGE_TYPE_CUBE_PY = 7
IMAGE_TYPE_CUBE_NY = 8
IMAGE_TYPE_CUBE_PZ = 9
IMAGE_TYPE_CUBE_NZ = 10
IMAGE_TYPE_ENVIRONMENT = 11

IMAGE_TYPE_ALBEDO_COMPRESSED = 127
IMAGE_TYPE_ROUGHNESS_COMPRESSED = 128
IMAGE_TYPE_METALLIC_COMPRESSED = 129
IMAGE_TYPE_NORMAL_COMPRESSED = 130
IMAGE_TYPE_AO_COMPRESSED = 131
IMAGE_TYPE_CUBE_PX_COMPRESSED = 132
IMAGE_TYPE_CUBE_NX_COMPRESSED = 133
IMAGE_TYPE_CUBE_PY_COMPRESSED = 134
IMAGE_TYPE_CUBE_NY_COMPRESSED = 135
IMAGE_TYPE_CUBE_PZ_COMPRESSED = 136
IMAGE_TYPE_CUBE_NZ_COMPRESSED = 137
IMAGE_TYPE_ENVIRONMENT_COMPRESSED = 138
```
The difference between format and format_COMPRESSED is that format_COMPRESSED is zlib compressed
## Image Formats
```
IMAGE_FORMAT_R8G8B8 = 0
IMAGE_FORMAT_R8G8B8A8 = 1
IMAGE_FORMAT_R32G32B32 = 2
IMAGE_FORMAT_R32G32B32A32 = 3
```
## License
The file format is free of known copyright restrictions (Public Domain)
