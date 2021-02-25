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

#define PADDING_TOP 1
#define PADDING_LEFT 1

#define CHANNEL_PARALLELISM 8
#define KERNEL_PARALLELISM 8

extern int depthwise_conv(short input_width, short input_height, short input_depth, short stride, elem_t input[INPUT_STRIP_COUNT_LIMIT][8], elem_t weights[KERNEL_STRIP_COUNT_LIMIT][8], elem_t output[INPUT_STRIP_COUNT_LIMIT][8]);

extern int pointwise_conv(short input_width, short input_height, short input_depth, short output_depth, short kernel_size, short stride, elem_t input[][CHANNEL_PARALLELISM], elem_t weights[][CHANNEL_PARALLELISM], elem_t output[][CHANNEL_PARALLELISM]);
#endif
