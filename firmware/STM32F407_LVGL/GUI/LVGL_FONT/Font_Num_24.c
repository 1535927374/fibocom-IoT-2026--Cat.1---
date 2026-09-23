/*******************************************************************************
 * Size: 23 px
 * Bpp: 1
 * Opts: --bpp 1 --size 23 --no-compress --stride 1 --align 1 --font ZhiyiSans-Regular.ttf --symbols 1234567890  %℃:：~ --format lvgl -o Font_Num_24.c
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl.h"
#endif



#ifndef FONT_NUM_24
#define FONT_NUM_24 1
#endif

#if FONT_NUM_24

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0025 "%" */
    0x38, 0x2, 0x36, 0x2, 0x31, 0x82, 0x18, 0xc3,
    0xc, 0x61, 0x6, 0x31, 0x3, 0x19, 0x80, 0xd9,
    0x80, 0x38, 0x8e, 0x0, 0x8d, 0x80, 0xcc, 0x60,
    0x46, 0x30, 0x43, 0x18, 0x41, 0x8c, 0x60, 0xc6,
    0x20, 0x36, 0x20, 0xe, 0x0,

    /* U+0030 "0" */
    0xf, 0x3, 0xc, 0x60, 0x66, 0x6, 0xc0, 0x2c,
    0x3, 0xc0, 0x3c, 0x3, 0xc0, 0x3c, 0x3, 0xc0,
    0x3c, 0x3, 0x40, 0x26, 0x6, 0x60, 0x63, 0xc,
    0xf, 0x0,

    /* U+0031 "1" */
    0xd, 0xfe, 0xc3, 0xc, 0x30, 0xc3, 0xc, 0x30,
    0xc3, 0xc, 0x30, 0xc3, 0xc,

    /* U+0032 "2" */
    0x1f, 0x6, 0x39, 0x83, 0x60, 0x34, 0x6, 0x0,
    0xc0, 0x38, 0xe, 0x3, 0x80, 0xe0, 0x38, 0xc,
    0x3, 0x0, 0xc0, 0x18, 0x6, 0x0, 0xff, 0xe0,

    /* U+0033 "3" */
    0x1f, 0xe, 0x39, 0x81, 0xa0, 0x30, 0x6, 0x0,
    0xc0, 0x30, 0xc, 0x1e, 0x0, 0x30, 0x3, 0x0,
    0x30, 0x6, 0x0, 0xf0, 0x3b, 0xe, 0x3f, 0x0,

    /* U+0034 "4" */
    0x0, 0xe0, 0x3, 0x80, 0x1e, 0x0, 0xd8, 0x6,
    0x60, 0x19, 0x80, 0xc6, 0x6, 0x18, 0x18, 0x60,
    0xc1, 0x86, 0x6, 0x18, 0x18, 0xff, 0xfc, 0x1,
    0x80, 0x6, 0x0, 0x18, 0x0, 0x60,

    /* U+0035 "5" */
    0x7f, 0xcc, 0x1, 0x80, 0x20, 0x4, 0x1, 0x80,
    0x30, 0x6, 0xf8, 0xe1, 0xd8, 0x18, 0x1, 0x80,
    0x30, 0x6, 0x0, 0xf0, 0x37, 0xc, 0x3f, 0x0,

    /* U+0036 "6" */
    0xf, 0x86, 0x18, 0x81, 0x30, 0x4, 0x1, 0x80,
    0x33, 0xc6, 0x8e, 0xe0, 0xd8, 0xf, 0x1, 0xe0,
    0x3c, 0x6, 0x80, 0xd8, 0x31, 0x8c, 0x1f, 0x0,

    /* U+0037 "7" */
    0xff, 0xe0, 0xc, 0x3, 0x0, 0x40, 0x18, 0x3,
    0x0, 0xc0, 0x18, 0x6, 0x0, 0xc0, 0x30, 0x6,
    0x0, 0xc0, 0x30, 0x6, 0x0, 0xc0, 0x18, 0x0,

    /* U+0038 "8" */
    0x1f, 0x7, 0x1c, 0xe0, 0xec, 0x6, 0xc0, 0x6c,
    0x6, 0x60, 0xc7, 0x18, 0x1e, 0x3, 0x1c, 0x60,
    0x6c, 0x3, 0xc0, 0x3c, 0x3, 0xc0, 0x77, 0xe,
    0x1f, 0x80,

    /* U+0039 "9" */
    0x1f, 0xe, 0x31, 0x83, 0x60, 0x2c, 0x7, 0x80,
    0xf0, 0x1f, 0x7, 0x71, 0xe3, 0xcc, 0x1, 0x80,
    0x30, 0x4, 0x1, 0xb0, 0x23, 0xc, 0x3e, 0x0,

    /* U+003A ":" */
    0xf0, 0x0, 0xf,

    /* U+007E "~" */
    0x78, 0x11, 0xc4, 0xf, 0x0,

    /* U+2103 "℃" */
    0x70, 0x0, 0x11, 0x0, 0x2, 0x21, 0xf8, 0x44,
    0xe1, 0xc7, 0x30, 0xc, 0xc, 0x1, 0x81, 0x80,
    0x0, 0x30, 0x0, 0xc, 0x0, 0x1, 0x80, 0x0,
    0x30, 0x0, 0x6, 0x0, 0x0, 0xe0, 0x0, 0xc,
    0x0, 0xc1, 0x80, 0x30, 0x18, 0x6, 0x1, 0xc3,
    0x80, 0xf, 0xc0,

    /* U+FF1A "：" */
    0xfc, 0x0, 0x0, 0xfc
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 99, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 313, .box_w = 17, .box_h = 17, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 38, .adv_w = 232, .box_w = 12, .box_h = 17, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 64, .adv_w = 232, .box_w = 6, .box_h = 17, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 77, .adv_w = 232, .box_w = 11, .box_h = 17, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 101, .adv_w = 232, .box_w = 11, .box_h = 17, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 125, .adv_w = 232, .box_w = 14, .box_h = 17, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 155, .adv_w = 232, .box_w = 11, .box_h = 17, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 179, .adv_w = 232, .box_w = 11, .box_h = 17, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 203, .adv_w = 232, .box_w = 11, .box_h = 17, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 227, .adv_w = 232, .box_w = 12, .box_h = 17, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 253, .adv_w = 232, .box_w = 11, .box_h = 17, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 277, .adv_w = 118, .box_w = 2, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 280, .adv_w = 221, .box_w = 11, .box_h = 3, .ofs_x = 1, .ofs_y = 6},
    {.bitmap_index = 285, .adv_w = 368, .box_w = 19, .box_h = 18, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 328, .adv_w = 368, .box_w = 3, .box_h = 10, .ofs_x = 10, .ofs_y = 4}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0x5, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x5e, 0x20e3, 0xfefa
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 65275, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 16, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif

};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t Font_Num_24 = {
#else
lv_font_t Font_Num_24 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 19,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 2,
#endif
//    .static_bitmap = 0,
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if FONT_NUM_24*/
