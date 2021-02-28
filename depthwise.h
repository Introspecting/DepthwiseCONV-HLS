#ifndef __DEPTHWISE_HEADER__
#define __DEPTHWISE_HEADER__

typedef short elem_t;
typedef short acc_t;

#define INPUT_WIDTH_LIMIT 112
#define INPUT_HEIGHT_LIMIT 112
#define INPUT_DEPTH_LIMIT 64
#define OUTPUT_DEPTH_LIMIT 64

#define INPUT_STRIP_COUNT_LIMIT (INPUT_WIDTH_LIMIT * INPUT_HEIGHT_LIMIT)

#define N (INPUT_STRIP_COUNT_LIMIT * CHANNEL_PARALLELISM)

#define OUTPUT_STRIP_COUNT_LIMIT (INPUT_WIDTH_LIMIT * INPUT_HEIGHT_LIMIT)

#define M (OUTPUT_STRIP_COUNT_LIMIT * KERNEL_PARALLELISM)

#define KERNEL_STRIP_COUNT_LIMIT (KERNEL_WIDTH * KERNEL_HEIGHT)

#define W (KERNEL_STRIP_COUNT_LIMIT * CHANNEL_PARALLELISM * KERNEL_PARALLELISM)

#define KERNEL_WIDTH 3
#define KERNEL_HEIGHT 3

#define PADDING_TOP 1
#define PADDING_LEFT 1

#define CHANNEL_PARALLELISM 8
#define KERNEL_PARALLELISM 8

extern void depthwise_conv(short input_width, short input_height, short input_depth, short stride, elem_t input[INPUT_STRIP_COUNT_LIMIT][8], elem_t weights[KERNEL_STRIP_COUNT_LIMIT][8], elem_t output[INPUT_STRIP_COUNT_LIMIT][8]);

extern void pointwise_conv(short input_width, short input_height, short input_depth, short output_depth, short kernel_size, short stride, elem_t input[][CHANNEL_PARALLELISM], elem_t weights[][CHANNEL_PARALLELISM], elem_t output[][CHANNEL_PARALLELISM]);

void read_data(elem_t input[], elem_t buf[][CHANNEL_PARALLELISM], int length);

void write_data(elem_t buf[][KERNEL_PARALLELISM], elem_t output[], int length);

#endif
