#include "depthwise.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int test_112_112_32_stride_2(const int input_width, const int input_height, const int input_depth, const int stride)
{

    int mismatch_count = 0;
    int match_count = 0;

    const int output_width = input_width / stride;
    const int output_height = input_height / stride;
    const int output_depth = input_depth;

    int input_length = input_width * input_height * input_depth;
    int output_length = output_width * output_height * output_depth;
    int weights_length = input_depth * 9;

    elem_t *input_ptr = malloc(input_length * sizeof(elem_t));
    elem_t *weights_ptr = malloc(weights_length * sizeof(elem_t));
    elem_t *output_ptr = malloc(output_length * sizeof(elem_t));
    elem_t *golden_output_ptr = malloc(output_length * sizeof(elem_t));

    elem_t(*input)[input_height][input_width][8] = input_ptr;
    elem_t(*weights)[3][3][8] = weights_ptr;
    elem_t(*output)[output_height][output_width][8] = output_ptr;
    elem_t(*golden_output)[output_height][output_width][8] = golden_output_ptr;

    memset(weights, 0, weights_length * sizeof(elem_t));

    for (int i = 0; i < input_depth / 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            weights[i][1][0][j] = 2;
            weights[i][1][1][j] = 2;
            weights[i][1][2][j] = 2;
        }
    }

    memset(input, 1, input_length * sizeof(elem_t));

    memset(output, 0, output_length * sizeof(elem_t));

    memset(golden_output, 0, output_length * sizeof(elem_t));

    depthwise_conv(input_width, input_height, input_depth, stride, input, weights, output);

    int dout, h, w, d;

    for (dout = 0; dout < input_depth / 8; dout += 1)
    {
        for (h = 0; h < output_height; h += 1)
        {
            for (w = 0; w < output_width; w += 1)
            {
                for (d = 0; d < 8; d++)
                {
                    elem_t top_left = (w == 0 || h == 0) ? 0 : input[dout][h * stride - 1][w * stride - 1][d];
                    elem_t top_center = (h == 0) ? 0 : input[dout][h * stride - 1][w * stride][d];
                    elem_t top_right = (w == output_width - 1 || h == 0) ? 0 : input[dout][h * stride - 1][w * stride + 1][d];
                    elem_t center_left = (w == 0) ? 0 : input[dout][h * stride][w * stride - 1][d];
                    elem_t center = input[dout][h * stride][w * stride][d];
                    elem_t center_right = (w == output_width - 1) ? 0 : input[dout][h * stride][w * stride + 1][d];
                    elem_t bottom_left = (w == 0 || h == output_height - 1) ? 0 : input[dout][h * stride + 1][w * stride - 1][d];
                    elem_t bottom_center = (h == output_height - 1) ? 0 : input[dout][h * stride + 1][w * stride][d];
                    elem_t bottom_right = (w == output_width - 1 || h == output_height - 1) ? 0 : input[dout][h * stride + 1][w * stride + 1][d];

                    golden_output[dout][h][w][d] = weights[dout][0][0][d] * top_left + weights[dout][0][1][d] * top_center + weights[dout][0][2][d] * top_right + weights[dout][1][0][d] * center_left + weights[dout][1][1][d] * center + weights[dout][1][2][d] * center_right + weights[dout][2][0][d] * bottom_left + weights[dout][2][1][d] * bottom_center + weights[dout][2][2][d] * bottom_right;
                }
            }
        }
    }

    for (dout = 0; dout < output_depth / 8; dout += 1)
    {
        for (h = 0; h < output_height; h++)
        {
            for (w = 0; w < output_width; w += 1)
            {
                for (d = 0; d < 8; d++)
                {
                    if (output[dout][h][w][d] != golden_output[dout][h][w][d])
                    {
                        if (mismatch_count == 0)
                        {
                            printf("\nFirst counterpart(Output:%d; Golden:%d) position:[%d][%d][%d][%d]\n", output[dout][h][w][d], golden_output[dout][h][w][d], dout, h, w, d);
                        }
                        mismatch_count++;
                    }
                    else
                    {
                        if (match_count == 0)
                        {
                            printf("\nFirst part(Output:%d; Golden:%d) position:[%d][%d][%d][%d]\n", output[dout][h][w][d], golden_output[dout][h][w][d], dout, h, w, d);
                        }
                        match_count++;
                    }
                }
            }
        }
    }

    if (mismatch_count == 0)
    {
        printf("\n********** Test %s Passed **********\n", __FUNCTION__);
    }
    else
    {

        printf("\n********** Test %s Failed **********\n", __FUNCTION__);
        printf("There are %d numbers mismatched.", mismatch_count);
    }
}

int main(void)
{
    test_112_112_32_stride_2(112, 112, 32, 2);

    return 0;
}
