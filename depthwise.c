#include "depthwise.h"

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

int depthwise_conv_per(short input_width, short input_height, short stride, elem_t input[INPUT_STRIP_COUNT_LIMIT][CHANNEL_PARALLELISM], elem_t weights[KERNEL_STRIP_COUNT_LIMIT][CHANNEL_PARALLELISM], elem_t output[INPUT_STRIP_COUNT_LIMIT][CHANNEL_PARALLELISM])
{
	short output_width = input_width / stride;
	short output_height = input_height / stride;

	int weight_index = 0;

	acc_t partials_0[CHANNEL_PARALLELISM] = {0};
	acc_t partials_1[CHANNEL_PARALLELISM] = {0};
	acc_t out_0[CHANNEL_PARALLELISM] = {0};
	elem_t results_0[CHANNEL_PARALLELISM] = {0};

	assert(stride == 1);

	assert(input_width == 112);
	assert(input_height == 112);


KERNEL_V_LOOP:
	for (short k = 0; k < KERNEL_HEIGHT; k++)
	{
		int input_index = 0;
		int output_index;
		int partial_index = 0;

		int y_start = -1 + k;

		if (y_start < 0)
		{
			y_start += stride;
			partial_index += output_width;
		}

		weight_index = k * KERNEL_WIDTH;
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

				if ((w == 1 && h == 0 && k == 1) || (w == 1 && h == 1 && k == 2))
				{
					printf("debug this");
				}

			DEPTH_LOOP:
				for (short d = 0; d < CHANNEL_PARALLELISM; d++)
				{
					elem_t i_0;
					elem_t w00, w01, w02;

					w00 = weights[weight_index][d];
					w01 = weights[weight_index + 1][d];
					w02 = weights[weight_index + 2][d];

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

	return 0;
}

int pointwise_conv_per(short input_width, short input_height, short kernel_size, short stride, elem_t input[INPUT_STRIP_COUNT_LIMIT][CHANNEL_PARALLELISM], elem_t weights[KERNEL_STRIP_COUNT_LIMIT][CHANNEL_PARALLELISM], elem_t output[INPUT_STRIP_COUNT_LIMIT][CHANNEL_PARALLELISM], int should_read_partials)
{
	short output_width = input_width / stride;
	short output_height = input_height / stride;

	int dout_base = 0;

	acc_t out_0[KERNEL_PARALLELISM];
	elem_t results_0[KERNEL_PARALLELISM];
	elem_t inputs_0[CHANNEL_PARALLELISM];
	elem_t weight_values[KERNEL_PARALLELISM][CHANNEL_PARALLELISM];

	short m, n, y = 0, x, k, c, v, h;

	assert(stride >= 1 && stride < 3);

	assert(kernel_size == 1);

	int y_start = 0;
	int x_start = 0;

	int weight_index = 0;

KERNEL_V_LOOP:
	for (v = 0; v < kernel_size; v++)
	{

	KERNEL_H_LOOP:
		for (short h = 0; h < kernel_size; h++)
		{
			int input_index = dout_base;
			int output_index = dout_base;

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
		Y_LOOP:
			for (y = 0; y < INPUT_WIDTH_LIMIT; y++)
			{
			X_LOOP:
				for (x = 0; x < INPUT_WIDTH_LIMIT; x++)
				{
					short x_loc, y_loc;
					char y_start_temp, x_start_temp;

					char y_invalid;
					if (x == 0 && y == 0) // 用不同的卷积核部分对X、Y方向计算内积。
					{
						x_start_temp = 0;
						y_start_temp = 0;

						x_start = x_start_temp < 0 ? stride + x_start_temp : x_start_temp;

						y_start = y_start_temp < 0 ? stride + y_start_temp : y_start_temp;

					}
					else
					{
					}

					// x为循环变量，x_loc为特征图横坐标。y同。
					x_loc = x + x_start;
					y_loc = y + y_start;
					y_invalid = 0;

					if (x == 0 && y == 0)
					{
						output_index = 0;
					}
					else
					{
						if (!y_invalid)
						{
							output_index++;
						}
					}

					char input_invalid = 0;
					// y方向的边界通过根据不同的k指定不同的y_start解决，也可以用y_invalid，表达式：h < stride + v - 1

					// TODO @lx 适应不同的stride。

					char can_write_output = !y_invalid;
					char can_read_partial = can_write_output || should_read_partials;

				DEPTH_LOOP:
					for (int k = 0; k < KERNEL_PARALLELISM; k++)
					{
						if (can_read_partial)
						{
							results_0[k] = output[output_index][k];
						}
						else
						{
							results_0[k] = 0;
						}

						if (k == 0 && x_loc == 0 && y_loc == 0 && v == 1 && h == 1)
						{
							printf("debug this");
						}

					LOAD_INPUT_VALUES_LOOP:
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

						acc_t part_0, part_1;

						part_0 = inputs_0[0] * weight_values[k][0] + inputs_0[1] * weight_values[k][1] + inputs_0[2] * weight_values[k][2] + inputs_0[3] * weight_values[k][3];
						part_1 = inputs_0[4] * weight_values[k][4] + inputs_0[5] * weight_values[k][5] + inputs_0[6] * weight_values[k][6] + inputs_0[7] * weight_values[k][7];

						out_0[k] = part_0 + part_1 + results_0[k];
						if (can_write_output)
						{
							output[output_index][k] = out_0[k];
						}
					}
					if (!input_invalid)
					{
						input_index += stride;
					}
				}
			}
		}
	}

	return 0;
}

void pointwise_conv(short input_width, short input_height, short input_depth, short output_depth, short kernel_size, short stride, elem_t input[][CHANNEL_PARALLELISM], elem_t weights[][CHANNEL_PARALLELISM], elem_t output[][KERNEL_PARALLELISM])
{
	int output_depth_limit = output_depth / KERNEL_PARALLELISM;
	int input_depth_limit = input_depth / CHANNEL_PARALLELISM;

	const int output_width = input_width / stride;
	const int output_height = input_height / stride;

	int input_step = input_width * input_height;
	int weights_step = kernel_size * kernel_size * KERNEL_PARALLELISM;
	int output_step = output_width * output_width;

	int output_index = 0;

	for (int m = 0; m < OUTPUT_DEPTH_LIMIT; m++)
	{
		int input_index = 0, weights_index = 0;

		if (m >= output_depth_limit)
			break;

		for (int n = 0; n < INPUT_DEPTH_LIMIT; n++)
		{
			if (n >= input_depth_limit)
				break;

			pointwise_conv_per(input_width, input_height, kernel_size, stride, input + input_index, weights + weights_index, output + output_index, n != 0);

			input_index += input_step;
			weights_index += weights_step;
		}

		output_index += output_step;
	}
}

void depthwise_conv(short input_width, short input_height, short input_depth, short stride, elem_t input[][CHANNEL_PARALLELISM], elem_t weights[][CHANNEL_PARALLELISM], elem_t output[][KERNEL_PARALLELISM])
{
	int input_depth_limit = input_depth / CHANNEL_PARALLELISM;

	const int output_width = input_width / stride;
	const int output_height = input_height / stride;

	int input_step = input_width * input_height;
	int weights_step = 9;
	int output_step = output_width * output_width;

	int input_index = 0, weights_index = 0;
	int output_index = 0;

	for (int n = 0; n < INPUT_DEPTH_LIMIT; n++)
	{
		if (n >= input_depth_limit)
			break;

		depthwise_conv_per(input_width, input_height, stride, input + input_index, weights + weights_index, output + output_index);

		input_index += input_step;
		weights_index += weights_step;
		output_index += output_step;
	}
}

void depthwise_top(elem_t input[N], elem_t output[M], elem_t weights_0[W], elem_t weights_1[W])
{
	short input_width = 112, input_height = 112, input_depth = 8;
	short output_width = 112, output_height = 112, output_depth = 8;
	int input_length = input_width * input_height * input_depth;
	int input_length_single = input_width * input_height * CHANNEL_PARALLELISM;

	int output_length = output_width * output_height * output_depth;
	int output_length_single = output_width * output_height * KERNEL_PARALLELISM;

	int layerwise_weights_length = 3 * 3 * CHANNEL_PARALLELISM;

	int pointwise_weights_length = 1 * 1 * CHANNEL_PARALLELISM * KERNEL_PARALLELISM;

	elem_t input_buf[INPUT_STRIP_COUNT_LIMIT][CHANNEL_PARALLELISM];

	elem_t output_buf[OUTPUT_STRIP_COUNT_LIMIT][KERNEL_PARALLELISM];

	elem_t intermediate_buf[OUTPUT_STRIP_COUNT_LIMIT][KERNEL_PARALLELISM];

	elem_t weights_0_buf[3 * 3][CHANNEL_PARALLELISM];

	elem_t weights_buf[1 * 1 * KERNEL_PARALLELISM * 1][CHANNEL_PARALLELISM];

//	depthwise_top_label0:for (short i = 0; i < input_depth / CHANNEL_PARALLELISM; i++)
//	{
		read_data(input, input_buf, input_length_single);
		read_data(weights_0, weights_0_buf, layerwise_weights_length);
		read_data(weights_1, weights_buf, pointwise_weights_length);

		depthwise_conv(input_width, input_height, input_depth, 1, input_buf,
				weights_0_buf,
						   intermediate_buf);


			pointwise_conv(input_width, input_height, input_depth, output_depth, 1, 1, intermediate_buf,
						   weights_buf,
						   output_buf);
//	}

	write_data(output_buf, output, output_length / KERNEL_PARALLELISM);
}

void read_data(elem_t input[N], elem_t buf[INPUT_STRIP_COUNT_LIMIT][CHANNEL_PARALLELISM], int length)
{
	int r, c;

RD_Loop_Row:
	for (r = 0; r < length / CHANNEL_PARALLELISM; r++)
	{
	RD_Loop_Col:
		for (c = 0; c < CHANNEL_PARALLELISM; c++)

#pragma HLS PIPELINE
			buf[r][c] = input[r * CHANNEL_PARALLELISM + c];
	}
}

void write_data(elem_t buf[INPUT_STRIP_COUNT_LIMIT][KERNEL_PARALLELISM], elem_t output[N], int length)
{
	int r, c;

WR_Loop_Row:
	for (r = 0; r < length / KERNEL_PARALLELISM; r++)
	{
	WR_Loop_Col:
		for (c = 0; c < KERNEL_PARALLELISM; c++)

#pragma HLS PIPELINE
			output[r * KERNEL_PARALLELISM + c] = buf[r][c];
	}
}
