#include "depthwise.h"
#include "ap_utils.h"
#include <stdint.h>
#include <assert.h>

int depthwise_conv(short input_width, short input_height, short input_depth, short stride, elem_t input[INPUT_STRIP_COUNT_LIMIT][CHANNEL_PARALLELISM], elem_t weights[KERNEL_STRIP_COUNT_LIMIT][CHANNEL_PARALLELISM], elem_t output[INPUT_STRIP_COUNT_LIMIT][CHANNEL_PARALLELISM])
{
	short output_width = input_width / stride;
	short output_height = input_height / stride;
	short output_depth = input_depth;

	int weight_index = 0;

	acc_t partials_0[CHANNEL_PARALLELISM] = {0};
	acc_t partials_1[CHANNEL_PARALLELISM] = {0};
	acc_t out_0[CHANNEL_PARALLELISM] = {0};
	elem_t results_0[CHANNEL_PARALLELISM] = {0};

	assert(stride == 1 || stride == 2);

	int dout_base;

DEPTH_OUTER_LOOP:
	for (short dout = 0; dout < (input_depth / CHANNEL_PARALLELISM); dout += 1)
	{
		dout_base = dout * output_width * output_height;

	KERNEL_V_LOOP:
		for (short k = 0; k < KERNEL_HEIGHT; k++)
		{
			int input_index = dout_base;
			int output_index;
			int partial_index = dout_base;

			int y_start = -1 + k;

			if (y_start < 0)
			{
				y_start += stride;
				partial_index += output_width;
			}

			weight_index = dout * KERNEL_WIDTH * KERNEL_HEIGHT + k * KERNEL_WIDTH;
			output_index = partial_index;

			char strided_output_valid = 0;
			char strided_partial_valid = 0;
			char output_count = 0;
			char partial_count = 0;

		HEIGHT_LOOP:
			for (short h = y_start; h < input_height; h += stride)
			{

			WIDTH_LOOP:
				for (short w = -KERNEL_WIDTH / 2; w < input_width + KERNEL_WIDTH / 2; w += 1)
				{
					char input_invalid = w == -1 || w == input_width;
					// y方向的边界通过根据不同的k指定不同的y_start解决，也可以用y_invalid，表达式：h < stride + k - 1
					char y_invalid = (h == 0 && k == 2) || (h == input_height - 1 && k == 0);

					if (!(w == 0 || w == -1))
					{
						strided_output_valid = output_count == 0;

						output_count++;
						if (output_count == stride)
						{
							output_count = 0;
						}
					}
					else
					{
						strided_output_valid = 0;
					}

					if (!(w >= input_width - 1))
					{
						strided_partial_valid = partial_count == 0;

						partial_count++;
						if (partial_count == stride)
						{
							partial_count = 0;
						}
					}
					else
					{
						strided_partial_valid = 0;
					}

					char can_write_output = !y_invalid && strided_output_valid;
					// 这里因为是分层卷积，所以k == 0
					// 时不用读取原来的部分和，但是普通卷积是需要的，因为需要加上之前通道的计算结果。
					char can_read_partial = !y_invalid && strided_partial_valid && k != 0;

				DEPTH_LOOP:
					for (short d = 0; d < CHANNEL_PARALLELISM; d++)
					{
						elem_t i_0;
						elem_t w00, w01, w02;

						w00 = weights[weight_index][d];
						w01 = weights[weight_index + 1][d];
						w02 = weights[weight_index + 2][d];

						if (w == 1 && h == 110 && dout == 0)
						{
							// debug this
							if (k == 0)
							{
								printf("222");
							}
							else if (k == 1)
							{
								printf("333");
							}
							else if (k == 3)
							{
								printf("444");
							}
						}

						if (input_invalid)
						{
							i_0 = 0;
						}
						else
						{
							i_0 = input[input_index][d];
						}

						if (can_read_partial)
						{
							results_0[d] = output[partial_index][d];
						}
						else
						{
							results_0[d] = 0;
						}

						// partials_1[d]存储着最接近结果的部分和，在行尾时，partials_1[d]即结果。
						if (stride == 1)
						{
							// 顺序很重要！！
							out_0[d] = i_0 * w02 + partials_1[d];

							partials_1[d] = partials_0[d] + i_0 * w01;

							partials_0[d] = i_0 * w00 + results_0[d];
						}
						else if (stride == 2)
						{
							if (w % 2 == 0)
							{
								partials_1[d] = partials_0[d] + i_0 * w01;
							}
							else
							{
								out_0[d] = i_0 * w02 + partials_1[d];
								partials_0[d] = i_0 * w00 + results_0[d];
							}
						}
						if (can_write_output)
						{
							output[output_index][d] = out_0[d];
						}
					}
					if (!input_invalid)
					{
						input_index++;
					}

					if (strided_partial_valid)
					{
						partial_index++;
					}

					if (strided_output_valid)
					{
						output_index++;
					}
				}
			}
		}
	}

	return 0;
}

int pointwise_conv_per(short input_width, short input_height, short kernel_size, short stride, elem_t input[][CHANNEL_PARALLELISM], elem_t weights[][CHANNEL_PARALLELISM], elem_t output[][CHANNEL_PARALLELISM], int should_read_partials)
{
	short output_width = input_width / stride;
	short output_height = input_height / stride;

	int dout_base = 0;

	acc_t out_0[KERNEL_PARALLELISM] = {0};
	elem_t results_0[KERNEL_PARALLELISM] = {0};
	elem_t inputs_0[CHANNEL_PARALLELISM] = {0};
	elem_t weight_values[KERNEL_PARALLELISM][CHANNEL_PARALLELISM] = {0};

	short m, n, y, x, k, c, v, h;

KERNEL_V_LOOP:
	for (v = 0; v < kernel_size; v++)
	{

	KERNEL_H_LOOP:
		for (short h = 0; h < kernel_size; h++)
		{
			int input_index = dout_base;
			int output_index = dout_base;
			int weight_index;

			int y_start = v - kernel_size / 2;

			if (y_start < 0)
			{
				y_start += stride;
				output_index += output_width;
			}
			int x_start = h - kernel_size / 2;
			if (x_start < 0)
			{
				x_start += stride;
				output_index += 1;
			}
			weight_index = v * kernel_size * KERNEL_PARALLELISM + h * KERNEL_PARALLELISM; // @lx

		LOAD_WEIGHT_VALUES_LOOP:
			for (k = 0; k < KERNEL_PARALLELISM; k++)
			{
				for (c = 0; c < CHANNEL_PARALLELISM; c++)
				{
					weight_values[k][c] = weights[weight_index + k][c];
				}
			}

			// TODO @lx flatten
		HEIGHT_LOOP:
			for (y = y_start; y < input_height; y += stride)
			{

			WIDTH_LOOP:
				for (x = x_start; x < input_width; x += stride)
				{
					// TODO @lx 适应不同的stride。
					char input_invalid = (x == 0 && h == 2) || (x == input_width - 1 && h == 0 && kernel_size == 3);
					// y方向的边界通过根据不同的k指定不同的y_start解决，也可以用y_invalid，表达式：h < stride + v - 1
					char y_invalid = (y == 0 && v == 2) || (y == input_height - 1 && v == 0 && kernel_size == 3);

					char can_write_output = !y_invalid;
					char can_read_partial = can_write_output || should_read_partials;

				DEPTH_LOOP:
					for (k = 0; k < KERNEL_PARALLELISM; k++)
					{
						if (can_read_partial)
						{
							results_0[k] = output[output_index][k];
						}
						else
						{
							results_0[k] = 0;
						}

						if (k == 0 && x == 3 && y == 3 && v == 1 && h == 1)
						{
							printf("debug this");
						}

						for (c = 0; c < CHANNEL_PARALLELISM; c++)
						{
							if (input_invalid)
							{
								inputs_0[c] = 0;
							}
							else
							{
								inputs_0[c] = input[input_index][c];
							}
						}

						for (c = 0; c < CHANNEL_PARALLELISM; c++)
						{
							elem_t i_0;
							elem_t w00;

							i_0 = inputs_0[c];
							w00 = weight_values[k][c];

							if (c == 0)
							{
								out_0[k] = i_0 * w00 + results_0[k];
							}
							else
							{
								out_0[k] += i_0 * w00;
							}
						}
						if (can_write_output)
						{
							output[output_index][k] = out_0[k];
						}
					}
					if (!input_invalid)
					{
						input_index += stride;
					}

					if (!y_invalid)
					{
						output_index++;
					}
				}
			}
		}
	}
}

int pointwise_conv(short input_width, short input_height, short input_depth, short output_depth, short kernel_size, short stride, elem_t input[][CHANNEL_PARALLELISM], elem_t weights[][CHANNEL_PARALLELISM], elem_t output[][KERNEL_PARALLELISM])
{
	int output_depth_limit = output_depth / KERNEL_PARALLELISM;
	int input_depth_limit = input_depth / CHANNEL_PARALLELISM;

	const int output_width = input_width / stride;
	const int output_height = input_height / stride;

	int input_step = input_width * input_height;
	int weights_step = kernel_size * kernel_size * KERNEL_PARALLELISM;
	int output_step = output_width * output_width;

	int output_index = 0;

	for (int m = 0; m < output_depth_limit; m++)
	{
		int input_index = 0, weights_index = 0;
		for (int n = 0; n < input_depth_limit; n++)
		{
			pointwise_conv_per(input_width, input_height, kernel_size, stride, input + input_index, weights + weights_index, output + output_index, n != 0);

			input_index += input_step;
			weights_index += weights_step;
		}

		output_index += output_step;
	}
}
