#include "lvgl.h"

const uint16_t purple_centre_map[] __attribute__((aligned(4), section(".rodata")))= {
    0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0x522f, 0x522f, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d,
    0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d,
    0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d, 0x2b3d,
    0x2b3d, 0x522f, 0x522f, 0xfd9f, 0xfd9f, 0x32b8, 0x32b8, 0xecff, 0xfd9f, 0xfd9f, 0x6ab3, 0x5b9e,
    0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x7c1f, 0x943f, 0x943f, 0x943f, 0x943f, 0x943f,
    0x943f, 0x943f, 0x943f, 0x943f, 0x943f, 0x943f, 0x943f, 0x943f, 0x943f, 0x943f, 0x7c1f, 0x5bbf,
    0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5b9e, 0x6ab3, 0xfd9f, 0xfd9f, 0xecff, 0x32b8, 0x32b8,
    0x635b, 0xfd9f, 0xfd9f, 0x6ab3, 0x5b9e, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x7c1f,
    0x943f, 0x943f, 0x943f, 0x943f, 0x943f, 0x943f, 0x943f, 0x943f, 0x943f, 0x943f, 0x943f, 0x943f,
    0x943f, 0x943f, 0x943f, 0x7c1f, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5b9e, 0x6ab3,
    0xfd9f, 0xfd9f, 0x635b, 0x32b7, 0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0xbb74, 0xe51f, 0xed1f, 0xed1f,
    0xed1f, 0xed1f, 0xed1f, 0xed1f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f,
    0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xed1f, 0xed1f, 0xed1f,
    0xed1f, 0xed1f, 0xed1f, 0xe51f, 0xbb74, 0xfd9f, 0xfd9f, 0x32b8, 0x3297, 0x32b8, 0x32b8, 0xfd9f,
    0xfd9f, 0xbb74, 0xe51f, 0xed1f, 0xed1f, 0xed1f, 0xed1f, 0xed1f, 0xed1f, 0xfd9f, 0xfd9f, 0xfd9f,
    0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f, 0xfd9f,
    0xfd9f, 0xfd9f, 0xed1f, 0xed1f, 0xed1f, 0xed1f, 0xed1f, 0xed1f, 0xe51f, 0xbb74, 0xfd9f, 0xfd9f,
    0x32b8, 0x3297, 0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0x5a72, 0x63df, 0x63df, 0x5bbf, 0x5bbf, 0x5bbf,
    0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf,
    0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf,
    0x63df, 0x63df, 0x5a72, 0xfd9f, 0xfd9f, 0x32b8, 0x3296, 0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0x5a72,
    0x63df, 0x63df, 0x5bbf, 0x539e, 0x539e, 0x5bbf, 0x5bbf, 0x5bbf, 0xc59f, 0xc59f, 0xc59f, 0xc59f,
    0xc59f, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0x5bbf,
    0x5bbf, 0x5bbf, 0x539e, 0x539e, 0x5bbf, 0x63df, 0x63df, 0x5a72, 0xfd9f, 0xfd9f, 0x32b8, 0x3296,
    0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0x5a72, 0x5bbe, 0x5bbf, 0x539e, 0x1886, 0x1886, 0x539e, 0x63bf,
    0x5bbf, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0xc59f,
    0xc59f, 0xc59f, 0xc59f, 0xc59f, 0x5bbf, 0x63bf, 0x539e, 0x1886, 0x1886, 0x539e, 0x5bbf, 0x5bbe,
    0x5a72, 0xfd9f, 0xfd9f, 0x32b8, 0x3296, 0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0x5a72, 0x5bbe, 0x539e,
    0x1886, 0x3b35, 0x3b35, 0x1886, 0x439e, 0x5bbf, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0x439e,
    0x1886, 0x1886, 0x1886, 0x539e, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0xc59f, 0x5bbf, 0x439e, 0x1886,
    0x3b35, 0x3b35, 0x1886, 0x539e, 0x5bbe, 0x5a72, 0xfd9f, 0xfd9f, 0x32b8, 0x3297, 0x32b8, 0x32b8,
    0xfd9f, 0xfd9f, 0x5a72, 0x5bbe, 0x1886, 0x3b35, 0x5c1b, 0x5c1b, 0x3b35, 0x1886, 0x5bbf, 0xc57f,
    0xc57f, 0xc57f, 0xc57f, 0x439e, 0x1886, 0x3b35, 0x3b35, 0x3b35, 0x1886, 0x539e, 0xc57f, 0xc57f,
    0xc57f, 0xc57f, 0x5bbf, 0x1886, 0x3b35, 0x5c1b, 0x5c1b, 0x3b35, 0x1886, 0x5bbe, 0x5a72, 0xfd9f,
    0xfd9f, 0x32b8, 0x3297, 0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0x5a72, 0x5bbe, 0x20a6, 0x3b35, 0x5c1b,
    0x5c1b, 0x3b35, 0x20a6, 0x5bbf, 0xbd7f, 0xbd7f, 0xbd7f, 0x439e, 0x20a6, 0x3b35, 0x43d8, 0x43d8,
    0x43d8, 0x3b35, 0x20a6, 0x539e, 0xbd7f, 0xbd7f, 0xbd7f, 0x5bbf, 0x20a6, 0x3b35, 0x5c1b, 0x5c1b,
    0x3b35, 0x20a6, 0x5bbe, 0x5a72, 0xfd9f, 0xfd9f, 0x32b8, 0x3297, 0x32b8, 0x32b8, 0xfd9f, 0xfd9f,
    0x5a72, 0x5bbe, 0x20c6, 0x3b35, 0x5c1b, 0x5c1b, 0x3b35, 0x20c6, 0x5bbf, 0xb55f, 0xb55f, 0xb55f,
    0x20c6, 0x3b35, 0x43d8, 0x643b, 0x643b, 0x643b, 0x43d8, 0x3b35, 0x20c6, 0xb55f, 0xb55f, 0xb55f,
    0x5bbf, 0x20c6, 0x3b35, 0x5c1b, 0x5c1b, 0x3b35, 0x20c6, 0x5bbe, 0x5a72, 0xfd9f, 0xfd9f, 0x32b8,
    0x3297, 0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0x5a72, 0x5bbe, 0x20c7, 0x3b55, 0x3b55, 0x3b55, 0x3b55,
    0x20c7, 0x5bbf, 0xb51f, 0xb51f, 0xb51f, 0x20c7, 0x541b, 0x541b, 0x6c3b, 0x6c3b, 0x6c3b, 0x541b,
    0x541b, 0x20c7, 0xb51f, 0xb51f, 0xb51f, 0x5bbf, 0x20c7, 0x3b55, 0x3b55, 0x3b55, 0x3b55, 0x20c7,
    0x5bbe, 0x5a72, 0xfd9f, 0xfd9f, 0x32b8, 0x32b7, 0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0x5a72, 0x5bbe,
    0x28e7, 0x28e7, 0x28e7, 0x28e7, 0x28e7, 0x28e7, 0x5bbf, 0xad1f, 0xad1f, 0xad1f, 0x28e7, 0x643b,
    0x643b, 0x6c3c, 0x6c3c, 0x6c3c, 0x643b, 0x643b, 0x28e7, 0xad1f, 0xad1f, 0xad1f, 0x5bbf, 0x28e7,
    0x28e7, 0x28e7, 0x28e7, 0x28e7, 0x28e7, 0x5bbe, 0x5a72, 0xfd9f, 0xfd9f, 0x32b8, 0x32b7, 0x32b8,
    0x32b8, 0xfd9f, 0xfd9f, 0x5a72, 0x5bbe, 0x28e7, 0x3b35, 0x5c1b, 0x541b, 0x3b35, 0x28e7, 0x5bbf,
    0xa4ff, 0xa4ff, 0xa4ff, 0x28e7, 0x3b14, 0x7c5c, 0x7c5c, 0x7c5c, 0x7c5c, 0x7c5c, 0x3b14, 0x28e7,
    0xa4ff, 0xa4ff, 0xa4ff, 0x5bbf, 0x28e7, 0x3b35, 0x541b, 0x5c1b, 0x3b35, 0x28e7, 0x5bbe, 0x5a72,
    0xfd9f, 0xfd9f, 0x32b8, 0x32b7, 0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0x5a72, 0x5bbe, 0x3108, 0x3b35,
    0x5c1b, 0x5c1b, 0x3b35, 0x3108, 0x5bbf, 0xa4df, 0xa4df, 0xa4df, 0x5bbf, 0x3108, 0x5419, 0x7c5c,
    0x7c5c, 0x7c5c, 0x5419, 0x3108, 0x5bbf, 0xa4df, 0xa4df, 0xa4df, 0x5bbf, 0x3108, 0x3b35, 0x5c1b,
    0x5c1b, 0x3b35, 0x3108, 0x5bbe, 0x5a72, 0xfd9f, 0xfd9f, 0x32b8, 0x32b7, 0x32b8, 0x32b8, 0xfd9f,
    0xfd9f, 0x5a72, 0x5bbe, 0x3128, 0x3b35, 0x5c1b, 0x5c1b, 0x3b35, 0x3128, 0x5bbf, 0x9cbf, 0x9cbf,
    0x9cbf, 0x9cbf, 0x5bbf, 0x3128, 0x745c, 0x745c, 0x745c, 0x3128, 0x5bbf, 0x9cbf, 0x9cbf, 0x9cbf,
    0x9cbf, 0x5bbf, 0x3128, 0x3b35, 0x5c1b, 0x5c1b, 0x3b35, 0x3128, 0x5bbe, 0x5a72, 0xfd9f, 0xfd9f,
    0x32b8, 0x32b7, 0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0x5a72, 0x5bbe, 0x3128, 0x3b35, 0x5c1b, 0x5c1b,
    0x3b35, 0x3128, 0x5bbf, 0x949f, 0x949f, 0x949f, 0x949f, 0x949f, 0x63df, 0x3128, 0x3128, 0x3128,
    0x63df, 0x949f, 0x949f, 0x949f, 0x949f, 0x949f, 0x5bbf, 0x3128, 0x3b35, 0x5c1b, 0x5c1b, 0x3b35,
    0x3128, 0x5bbe, 0x5a72, 0xfd9f, 0xfd9f, 0x32b8, 0x32b7, 0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0x5a72,
    0x5bbe, 0x3928, 0x3b55, 0x3b55, 0x3b55, 0x3b55, 0x3928, 0x5bbf, 0x947f, 0x947f, 0x947f, 0x947f,
    0x947f, 0x947f, 0x947f, 0x947f, 0x947f, 0x947f, 0x947f, 0x947f, 0x947f, 0x947f, 0x947f, 0x5bbf,
    0x3928, 0x3b55, 0x3b55, 0x3b55, 0x3b55, 0x3928, 0x5bbe, 0x5a72, 0xfd9f, 0xfd9f, 0x32b8, 0x32b7,
    0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0x5a72, 0x5bbe, 0x394a, 0x32f3, 0x3b55, 0x3b55, 0x32f3, 0x394a,
    0x5bbf, 0x845f, 0x845f, 0x845f, 0x845f, 0x845f, 0x845f, 0x845f, 0x845f, 0x845f, 0x845f, 0x845f,
    0x845f, 0x845f, 0x845f, 0x845f, 0x5bbf, 0x394a, 0x32f3, 0x3b55, 0x3b55, 0x32f3, 0x394a, 0x5bbe,
    0x5a72, 0xfd9f, 0xfd9f, 0x32b8, 0x32b7, 0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0x5a72, 0x5bbe, 0x416a,
    0x32f3, 0x3b55, 0x3b55, 0x32f3, 0x416a, 0x5bbf, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f,
    0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x5bbf, 0x416a, 0x32f3,
    0x3b55, 0x3b55, 0x32f3, 0x416a, 0x5bbe, 0x5a72, 0xfd9f, 0xfd9f, 0x32b8, 0x32b7, 0x32b8, 0x32b8,
    0xfd9f, 0xfd9f, 0x5a72, 0x5bbe, 0x416a, 0x32f3, 0x3b55, 0x3b55, 0x32f3, 0x416a, 0x5bbf, 0x843f,
    0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f,
    0x843f, 0x843f, 0x5bbf, 0x416a, 0x32f3, 0x3b55, 0x3b55, 0x32f3, 0x416a, 0x5bbe, 0x5a72, 0xfd9f,
    0xfd9f, 0x32b8, 0x32b7, 0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0x5a72, 0x5bbe, 0x418a, 0x32f3, 0x3b55,
    0x3b55, 0x32f3, 0x418a, 0x5bbf, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f,
    0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x5bbf, 0x418a, 0x32f3, 0x3b55, 0x3b55,
    0x32f3, 0x418a, 0x5bbe, 0x5a72, 0xfd9f, 0xfd9f, 0x32b8, 0x32b7, 0x32b8, 0x32b8, 0xfd9f, 0xfd9f,
    0x5a72, 0x5bbe, 0x518b, 0x32f3, 0x3b55, 0x3b55, 0x32f3, 0x518b, 0x5bbf, 0x843f, 0x843f, 0x843f,
    0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f,
    0x5bbf, 0x518b, 0x32f3, 0x3b55, 0x3b55, 0x32f3, 0x518b, 0x5bbe, 0x5a72, 0xfd9f, 0xfd9f, 0x32b8,
    0x32b7, 0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0x5a72, 0x5bbe, 0x51ab, 0x51ab, 0x51ab, 0x51ab, 0x51ab,
    0x51ab, 0x5bbf, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f,
    0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x5bbf, 0x51ab, 0x51ab, 0x51ab, 0x51ab, 0x51ab, 0x51ab,
    0x5bbe, 0x5a72, 0xfd9f, 0xfd9f, 0x32b8, 0x32b8, 0x32b8, 0x635b, 0xfd9f, 0xfd9f, 0x5a72, 0x5bbe,
    0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f,
    0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x5bbf, 0x5bbf,
    0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbe, 0x5a72, 0xfd9f, 0xfd9f, 0x635b, 0x32b8, 0x32b8,
    0xecff, 0xfd9f, 0xfd9f, 0x5a72, 0x5bbe, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf,
    0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f, 0x843f,
    0x843f, 0x843f, 0x843f, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbf, 0x5bbe, 0x5a72,
    0xfd9f, 0xfd9f, 0xecff, 0x32b8, 0x32b8, 0xfd9f, 0xfd9f, 0x7ad3, 0x7ad3, 0x9c5f, 0x9c5f, 0x9c5f,
    0x9c5f, 0x9c5f, 0x9c5f, 0x9c5f, 0xed1f, 0xfd5f, 0xfd5f, 0xfd5f, 0xfd5f, 0xfd5f, 0xfd5f, 0xfd5f,
    0xfd5f, 0xfd5f, 0xfd5f, 0xfd5f, 0xfd5f, 0xfd5f, 0xfd5f, 0xfd5f, 0xed1f, 0x9c5f, 0x9c5f, 0x9c5f,
    0x9c5f, 0x9c5f, 0x9c5f, 0x9c5f, 0x7ad3, 0x7ad3, 0xfd9f, 0xfd9f,
};

const lv_image_dsc_t purple_centre_img __attribute__((section(".rodata"))) = {
    .header = {
        .cf = LV_COLOR_FORMAT_RGB565,
        .w = 41,
        .h = 28,
    },
    .data_size = sizeof(purple_centre_map),
    .data = (const uint8_t *)purple_centre_map
};
