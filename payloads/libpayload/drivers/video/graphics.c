#include <libpayload.h>
#include <sysinfo.h>

struct resolution {
	uint32_t width;
	uint32_t height;
};

/*
 * 'campus' is the drawing area located in the center of the screen.
 * It extends to the top and bottom edges of the screen and leaves unused
 * (non-drawing) areas on the left and the right if the aspect ratios of the
 * display and the campus do not match.
 */

/* This determines the size of the actual drawing area. */
static struct resolution campus;

/* number of pixels from the left edge of the screen to the drawing area. */
static uint32_t campus_offset;

/* aspect ratio of the campus */
static const struct resolution campus_aspect_ratio = {
	.width = 1280,
	.height = 800,
};

static struct resolution display;
static struct cb_framebuffer *fbinfo;
static uint8_t *fbaddr;
static char initialized = 0;

#define LOG(x...)	printf("CBGFX: " x)

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
	uint8_t *pixel = fbaddr + (x + y * xres) * bpp / 8;
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

	/* calculate campus size, assuming the screen is landscape */
	campus.height = display.height;
	campus.width = display.height *
			campus_aspect_ratio.width / campus_aspect_ratio.height;
	campus_offset = (display.width - campus.width) / 2;

	initialized = 1;
	LOG("campus initialized: width=%d, height=%d, offset=%d\n",
	    campus.width, campus.height, campus_offset);

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

	x = campus.width * x_rel / 100 + campus_offset;
	y = campus.height * y_rel / 100;
	width = campus.width * width_rel / 100;
	height = campus.height * height_rel / 100;
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
