#ifndef __DEPTHWISE_HEADER__
#define __DEPTHWISE_HEADER__

typedef short elem_t;
typedef short acc_t;

#define INPUT_WIDTH_LIMIT 112
#define INPUT_HEIGHT_LIMIT 112
#define INPUT_DEPTH_LIMIT 4

#define INPUT_STRIP_COUNT_LIMIT (INPUT_WIDTH_LIMIT * INPUT_HEIGHT_LIMIT * INPUT_DEPTH_LIMIT)

#define KERNEL_STRIP_COUNT_LIMIT (KERNEL_WIDTH * KERNEL_HEIGHT * INPUT_DEPTH_LIMIT)

#define KERNEL_WIDTH 3
#define KERNEL_HEIGHT 3

extern int depthwise_conv(short input_width, short input_height, short input_depth, short stride, elem_t input[INPUT_STRIP_COUNT_LIMIT][8], elem_t weights[KERNEL_STRIP_COUNT_LIMIT][8], elem_t output[INPUT_STRIP_COUNT_LIMIT][8]);

#endif
