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
	for (short dout = 0; dout < (input_depth >> 3); dout += 1)
	{
		dout_base = dout * input_width * input_height;

	KERNEL_V_LOOP:
		for (short k = 0; k < KERNEL_HEIGHT; k++)
		{
			int stripe_part = dout_base;

			int output_partial_part = dout_base;

			// 输出x位置比输入滞后一个位置。
			if (k == 0)
			{
				output_partial_part += output_width;
			}
			else if (k == 1)
			{
				output_partial_part += 0;
			}
			else if (k == 2)
			{
				output_partial_part += -output_width;
			}
			weight_index = dout * KERNEL_WIDTH * KERNEL_HEIGHT + k * KERNEL_WIDTH;
			int output_stripe_part = output_partial_part;
		HEIGHT_LOOP:
			for (short h = 0; h < input_height; h++)
			{

			WIDTH_LOOP:
				for (short w = -KERNEL_WIDTH / 2; w < input_width + KERNEL_WIDTH / 2; w += 1)
				{
					char input_invalid = w == -1 || w == input_width;
					char y_invalid = (h == 0 && k == 2) || (h == input_height - 1 && k == 0);
					char output_invalid = (w == 0 || w == -1) || y_invalid;
					char partial_invalid = (w >= input_width - 1) || y_invalid;

				DEPTH_LOOP:
					for (short d = 0; d < 8; d++)
					{
						elem_t i_0;
						elem_t w00, w01, w02;

						input_index = stripe_part;

						output_index = output_stripe_part; // @lx 输出位置，含stride。

						// 同样要读部分和。
						w00 = weights[weight_index][d];
						w01 = weights[weight_index + 1][d];
						w02 = weights[weight_index + 2][d];

						if (w == -1 && h == 1 && k == 1)
						{
							printf("debug this");
						}

						if (input_invalid)
						{
							i_0 = 0;
						}
						else
						{
							i_0 = input[input_index][d];
						}

						if (partial_invalid || k == 0)
						{
							results_0[d] = 0;
						}
						else
						{
							results_0[d] = output[output_partial_part][d];
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
						if (!output_invalid)
						{
							output[output_index][d] = out_0[d];
						}
					}
					if (!input_invalid)
					{
						stripe_part++;
					}

					if (!(w >= input_width - 1))
					{
						output_partial_part++;
					}

					if (!(w == 0 || w == -1))
					{
						output_stripe_part++;
					}
				}
			}
		}
	}

	return 0;
}
