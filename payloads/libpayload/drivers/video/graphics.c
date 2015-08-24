#include <libpayload.h>
#include <sysinfo.h>
#include "bitmap.h"

#define LOG(x...)	printf("CBGFX: " x)
/*
 * This is the range used internally to scale bitmap images. (e.g. 128 = 50%,
 * 512 = 200%). We choose 256 so that division and multiplication becomes bit
 * shift operation.
 */
#define BITMAP_SCALE_BASE	256

struct resolution {
	uint32_t width;
	uint32_t height;
};

/*
 * 'canvas' is the drawing area located in the center of the screen. It's a
 * square area, stretching vertically to the edges of the screen, leaving
 * non-drawing areas on the left and right. The screen is assumed to be
 * landscape.
 */

/* This determines the size of the actual drawing area. */
static struct resolution canvas;

/* number of pixels from the left edge of the screen to the drawing area. */
static uint32_t canvas_offset;

static struct resolution display;
static struct cb_framebuffer *fbinfo;
static uint8_t *fbaddr;
static char initialized = 0;

static inline uint32_t calculate_color(uint32_t red, uint32_t green,
				       uint32_t blue)
{
	uint32_t color = 0;
	color |= (red >> (8 - fbinfo->red_mask_size))
		<< fbinfo->red_mask_pos;
	color |= (green >> (8 - fbinfo->green_mask_size))
		<< fbinfo->green_mask_pos;
	color |= (blue >> (8 - fbinfo->blue_mask_size))
		<< fbinfo->blue_mask_pos;
	return color;
}

static inline void set_pixel(uint32_t x, uint32_t y, uint32_t color)
{
	const uint32_t xres = fbinfo->x_resolution;
	const int bpp = fbinfo->bits_per_pixel;
	int i;
	uint8_t *pixel = fbaddr + (x + canvas_offset + y * xres) * bpp / 8;
	for (i = 0; i < bpp / 8; i++)
		pixel[i] = (color >> (i * 8));
}

/*
 * Initializes the library. It's automatically called by APIs and becomes
 * no-op once the library is successfully initialized.
 */
static int cbgfx_init(void)
{
	if (initialized)
		return 0;

	fbinfo = lib_sysinfo.framebuffer;
	if (!fbinfo)
		return -1;
	display.width = fbinfo->x_resolution;
	display.height = fbinfo->y_resolution;

	fbaddr = (uint8_t *)(uintptr_t)(fbinfo->physical_address);
	if (!fbaddr)
		return -1;

	/* calculate canvas size, assuming the screen is landscape */
	canvas.height = display.height;
	canvas.width = display.height;
	canvas_offset = (display.width - canvas.width) / 2;

	initialized = 1;
	LOG("canvas initialized: width=%d, height=%d, offset=%d\n",
	    canvas.width, canvas.height, canvas_offset);

	return 0;
}

int draw_box(uint32_t x_rel, uint32_t y_rel,
	     uint32_t width_rel, uint32_t height_rel,
	     uint32_t red, uint32_t green, uint32_t blue)
{
	uint32_t x, y, width, height, color;
	int i, j;

	if (cbgfx_init())
		return -1;

	x = canvas.width * x_rel / 100;
	y = canvas.height * y_rel / 100;
	width = canvas.width * width_rel / 100;
	height = canvas.height * height_rel / 100;
	color = calculate_color(red, green, blue);
	for (j = y; j < y + height; j++)
		for (i = x; i < x + width; i++)
			set_pixel(i, j, color);

	return 0;
}

int clear_screen(uint32_t red, uint32_t green, uint32_t blue)
{
	if (cbgfx_init())
		return -1;

	return draw_box(0, 0, 100, 100, red, green, blue);
}

static int draw_bitmap_v3(uint32_t x, uint32_t y, uint32_t scale,
			  struct bitmap_header_v3 *header,
			  struct bitmap_palette_element_v3 *palette,
			  uint8_t *pixel_array)
{
	const int bpp = header->bits_per_pixel;

	if (header->compression) {
		LOG("Compressed bitmaps are not supported\n");
		return -1;
	}
	if (bpp >= 16) {
		LOG("Non-palette bitmaps are not supported\n");
		return -1;
	}
	if (bpp != 8) {
		LOG("Unsupported bits per pixel (%d)\n", bpp);
		return -1;
	}

	/* calculate absolute height and width of the image */
	const int32_t width = header->width * scale / BITMAP_SCALE_BASE;
	int32_t height = header->height * scale / BITMAP_SCALE_BASE;
	const int extra = header->width % 4;
	const int padding = extra ? (4 - extra) : 0;
	/*
	 * header->height can be positive or negative.
	 *
	 * If it's negative, pixel data is stored from top to bottom. We render
	 * image from the lowest row to the highest row.
	 *
	 * If it's positive, pixel data is stored from bottom to top. We render
	 * image from the highest row to the lowest row.
	 */
	int32_t dir;
	if (header->height < 0) {
		height = -height;
		dir = 1;
	} else {
		y += height - 1;
		dir = -1;
	}
	const int32_t y_stride = header->width * bpp / 8 + padding;
	/*
	 * (x0, y0): counter for source (bitmap data)
	 * (x1, y1): counter for destination (canvas)
	 *
	 * We scan over the canvas using (x1, y1) and find the corresponding
	 * pixel data at (x0, y0), which is adjusted by scale.
	 */
	int32_t y1;
	for (y1 = 0; y1 < height; y1++) {
		int32_t y0 = y1 * BITMAP_SCALE_BASE / scale;
		uint8_t *data = pixel_array + y0 * y_stride;
		int32_t x1;
		for (x1 = 0; x1 < width; x1++) {
			int32_t x0 = x1 * BITMAP_SCALE_BASE / scale;
			if (y0 * y_stride + x0 > header->size)
				/*
				 * Because we're handling integers rounded by
				 * divisions, we might get here legitimately
				 * when rendering the last row of a sane image.
				 */
				return 0;
			uint8_t index = data[x0];
			if (index >= header->colors_used) {
				LOG("Color index exceeds palette boundary\n");
				return -1;
			}
			uint32_t color = calculate_color(palette[index].red,
							 palette[index].green,
							 palette[index].blue);
			set_pixel(x + x1, y + dir * y1, color);
		}
	}

	return 0;
}

int draw_bitmap(uint8_t x_rel, uint8_t y_rel,
		uint32_t scale_rel, uint8_t *bitmap, uint32_t size)
{
	struct bitmap_file_header *file_header =
			(struct bitmap_file_header *)bitmap;
	uint32_t header_size;
	uint32_t x, y, scale;
	struct bitmap_header_v3 *header;
	struct bitmap_palette_element_v3 *palette;
	uint8_t *pixel_array;

	if (cbgfx_init())
		return -1;

	if (file_header->signature[0] != 'B' ||
	    file_header->signature[1] != 'M') {
		LOG("Bitmap signature mismatch\n");
		return -1;
	}

	header_size = le32toh(*(uint32_t *)(file_header + 1));
	/* use header size as a version indicator. only v3 is supported now */
	if (header_size != sizeof(*header)) {
		LOG("Unsupported bitmap format\n");
		return -1;
	}

	/* convert relative coordinate (x_rel, y_rel) to absolute (x, y) */
	x = canvas.width * x_rel / 100;
	y = canvas.height * y_rel / 100;
	header = (struct bitmap_header_v3 *)(&file_header[1]);
	if ((uint8_t *)header + sizeof(*header) > bitmap + size) {
		LOG("Bitmap header exceeds buffer boundary\n");
		return -1;
	}

	/* convert a canvas scale to a self scale (relative to image size) */
	scale = scale_rel * canvas.width * BITMAP_SCALE_BASE /
			(100 * header->width);
	/*
	 * Check whether the right bottom corner is exceeding canvas boundaries
	 * or not. This check is comprehensive; meaning it checks x_rel, y_rel,
	 * and scale_rel are between 0 and 100 as well.
	 */
	if (x + header->width * scale / BITMAP_SCALE_BASE > canvas.width ||
	    y + header->height * scale / BITMAP_SCALE_BASE > canvas.height) {
		LOG("Bitmap image exceeds canvas boundary\n");
		return -1;
	}

	palette = (struct bitmap_palette_element_v3 *)&header[1];
	if ((uint8_t *)palette + header->colors_used >
			bitmap + file_header->bitmap_offset) {
		LOG("Bitmap palette data exceeds palette boundary\n");
		return -1;
	}

	pixel_array = (uint8_t *)bitmap + file_header->bitmap_offset;
	if (pixel_array + header->size > bitmap + size) {
		LOG("Bitmap pixel array exceeds buffer boundary\n");
		return -1;
	}

	return draw_bitmap_v3(x, y, scale, header, palette, pixel_array);
}
