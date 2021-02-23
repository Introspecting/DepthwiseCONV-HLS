#include "depthwise.h"
#include "ap_utils.h"
#include <stdint.h>
#include <assert.h>

int depthwise_conv(short input_width, short input_height, short input_depth, short stride, elem_t input[INPUT_STRIP_COUNT_LIMIT][8], elem_t weights[KERNEL_STRIP_COUNT_LIMIT][8], elem_t output[INPUT_STRIP_COUNT_LIMIT][8])
{
	short output_width = input_width / stride;
	short output_height = input_height / stride;
	short output_depth = input_depth;

	int input_index = 0;
	int weight_index = 0;
	int output_index = 0;

	acc_t partials_0[8] = {0};
	acc_t partials_1[8] = {0};
	acc_t out_0[8] = {0};
	elem_t results_0[8] = {0};

	assert(stride == 1 || stride == 2);

	int dout_base;

DEPTH_OUTER_LOOP:
	for (short dout = 0; dout < (input_depth / 8); dout += 1)
	{
		dout_base = dout * input_width * input_height;

	KERNEL_V_LOOP:
		for (short k = 0; k < KERNEL_HEIGHT; k++)
		{
			int stripe_part = dout_base;
			int output_stripe_part = dout_base;

			// 输出x位置比输入滞后一个位置。
			if (k == 0)
			{
				output_stripe_part += input_width - 1;
			}
			else if (k == 1)
			{
				output_stripe_part += -1;
			}
			else if (k == 2)
			{
				output_stripe_part += -input_width - 1;
			}

			weight_index = dout * KERNEL_WIDTH * KERNEL_HEIGHT + k * KERNEL_WIDTH;

		HEIGHT_LOOP:
			for (short h = 0; h < input_height; h++)
			{

			WIDTH_LOOP:
				for (short w = -KERNEL_WIDTH / 2; w < input_width + KERNEL_WIDTH / 2; w += 1)
				{
					char input_invalid = w == -1 || w == input_width;

				DEPTH_LOOP:
					for (short d = 0; d < 8; d++)
					{
						elem_t i_0;
						elem_t w00, w01, w02;

						char x_start, y_start, x_end, y_end;
						char block_start, channel_start;

						x_start = w == 0;
						y_start = h == 0;
						x_end = w == input_width - 1;
						y_end = h == input_height - 1;

						input_index = stripe_part;

						output_index = output_stripe_part; // @lx 输出位置，含stride。

						// 同样要读部分和。
						w00 = weights[weight_index][d];
						w01 = weights[weight_index + 1][d];
						w02 = weights[weight_index + 2][d];

						if (w == -1 && h == 111 && k == 2)
						{
							printf("debug this");
						}

						char out_valid = !(w == 0 || w == -1) && !(y_start && k == 2) && !(y_end && k == 0);
						char y_valid = !(y_start && k == 2) && !(y_end && k == 0);

						if (input_invalid)
						{
							i_0 = 0;
						}
						else
						{
							i_0 = input[input_index][d];
						}
						if (y_valid && k != 0 && w != input_width)
						{
							if (w == -1)
							{
								results_0[d] = output[output_index + 1][d];
							}
							else
							{
								results_0[d] = output[output_index + 2][d];
							}
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
						if (out_valid)
						{
							output[output_index][d] = out_0[d];
						}
					}
					if (!input_invalid)
					{
						stripe_part += 1;
						output_stripe_part += 1;
					}
				}
			}
		}
	}

	return 0;
}
