#include "depthwise.h"

#include <assert.h>

int test_loop(short stride, short output_width, elem_t input[INPUT_STRIP_COUNT_LIMIT][CHANNEL_PARALLELISM], elem_t weights[KERNEL_STRIP_COUNT_LIMIT][CHANNEL_PARALLELISM], elem_t output[INPUT_STRIP_COUNT_LIMIT][CHANNEL_PARALLELISM])
{
	int output_index = 0;
	int input_index = 0;

	char can_read_partial = 1;
	int can_write_output = 1;
	char input_invalid = 0;
	int dout_base = 0;
	short kernel_size = 3;

	acc_t out_0[KERNEL_PARALLELISM];
	elem_t results_0[KERNEL_PARALLELISM];
	elem_t inputs_0[CHANNEL_PARALLELISM];
	elem_t weight_values[KERNEL_PARALLELISM][CHANNEL_PARALLELISM];
	short input_width = 200;

	short m, n, y = 0, x, k, c, v, h;

	assert(stride < 2 && stride >= 0);

//	weight_index = v * kernel_size * KERNEL_PARALLELISM + h * KERNEL_PARALLELISM; // @lx

			LOAD_WEIGHT_VALUES_LOOP:
				for (k = 0; k < KERNEL_PARALLELISM; k++)
				{
					for (c = 0; c < CHANNEL_PARALLELISM; c++)
					{
						weight_values[k][c] = weights[0 + k][c];
					}
				}


KERNEL_V_LOOP:
	for (v = 0; v < 20; v++) // @lx kernel_size
	{

	KERNEL_H_LOOP:
		for (short h = 0; h < 20; h++) // kernel_size
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
for (y = 0; y < 200; y++)
		XLOOP:
			for (x = 0; x < INPUT_WIDTH_LIMIT; x += 1)
			{
				if (x >= input_width)
					break;
				
				// TODO @lx 适应不同的stride。
				char input_invalid = (x == 0 && h == 2) || (x == input_width - 1 && h == 0 && kernel_size == 3);
				// y方向的边界通过根据不同的k指定不同的y_start解决，也可以用y_invalid，表达式：h < stride + v - 1
				char y_invalid = (y == 0 && v == 2) || (y == 100 - 1 && v == 0 && kernel_size == 3);

				char can_write_output = 1;
				char can_read_partial = can_write_output || 0;
			DPETH_LOOP:
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

				if (1)
				{
					output_index++;
				}
			}
		}
	}
}
