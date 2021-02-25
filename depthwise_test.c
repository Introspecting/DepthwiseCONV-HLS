#include "depthwise.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

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

    elem_t(*input)[input_height][input_width][CHANNEL_PARALLELISM] = input_ptr;
    elem_t(*weights)[3][3][CHANNEL_PARALLELISM] = weights_ptr;
    elem_t(*output)[output_height][output_width][CHANNEL_PARALLELISM] = output_ptr;
    elem_t(*golden_output)[output_height][output_width][CHANNEL_PARALLELISM] = golden_output_ptr;

    memset(weights, 0, weights_length * sizeof(elem_t));

    for (int i = 0; i < input_depth / CHANNEL_PARALLELISM; i++)
    {
        for (int j = 0; j < CHANNEL_PARALLELISM; j++)
        {
            weights[i][0][0][j] = 1;
            weights[i][0][1][j] = 2;
            weights[i][0][2][j] = 3;

            weights[i][1][0][j] = 2;
            weights[i][1][1][j] = 2;
            weights[i][1][2][j] = 2;

            weights[i][2][0][j] = 4;
            weights[i][2][1][j] = 5;
            weights[i][2][2][j] = 6;
        }
    }

    memset(input, 1, input_length * sizeof(elem_t));

    memset(output, 0, output_length * sizeof(elem_t));

    memset(golden_output, 0, output_length * sizeof(elem_t));

    depthwise_conv(input_width, input_height, input_depth, stride, input, weights, output);

    int dout, h, w, d;

    for (dout = 0; dout < input_depth / CHANNEL_PARALLELISM; dout += 1)
    {
        for (h = 0; h < output_height; h += 1)
        {
            for (w = 0; w < output_width; w += 1)
            {
                for (d = 0; d < CHANNEL_PARALLELISM; d++)
                {
                    // input_x和input_y是元素对应的卷积窗口左上角坐标。
                    int input_x = w * stride - PADDING_LEFT;
                    int input_y = h * stride - PADDING_TOP;
                    elem_t top_left = (input_x < 0 || input_y < 0) ? 0 : input[dout][input_y][input_x][d];
                    elem_t top_center = (input_y < 0) ? 0 : input[dout][input_y][input_x + 1][d];
                    elem_t top_right = (input_x + 2 > input_width - 1 || input_y < 0) ? 0 : input[dout][input_y][input_x + 2][d];
                    elem_t center_left = (input_x < 0) ? 0 : input[dout][input_y + 1][input_x][d];
                    elem_t center = input[dout][input_y + 1][input_x + 1][d];
                    elem_t center_right = (input_x + 2 > input_width - 1) ? 0 : input[dout][input_y + 1][input_x + 1 + 1][d];
                    elem_t bottom_left = (input_x < 0 || input_y + 2 > input_height - 1) ? 0 : input[dout][input_y + 1 + 1][input_x][d];
                    elem_t bottom_center = (input_y + 2 > input_height - 1) ? 0 : input[dout][input_y + 1 + 1][input_x + 1][d];
                    elem_t bottom_right = (input_x + 2 > input_width - 1 || input_y + 2 > input_height - 1) ? 0 : input[dout][input_y + 1 + 1][input_x + 1 + 1][d];

                    golden_output[dout][h][w][d] = weights[dout][0][0][d] * top_left + weights[dout][0][1][d] * top_center + weights[dout][0][2][d] * top_right + weights[dout][1][0][d] * center_left + weights[dout][1][1][d] * center + weights[dout][1][2][d] * center_right + weights[dout][2][0][d] * bottom_left + weights[dout][2][1][d] * bottom_center + weights[dout][2][2][d] * bottom_right;
                }
            }
        }
    }

    return compare_array(output_width, output_height, output_depth, output, golden_output, __FUNCTION__);
}

int test_pointwise_conv(int input_width, int input_height, int input_depth, int output_depth, int kernel_size, int stride)
{
    int mismatch_count = 0;
    int match_count = 0;

    const int output_width = input_width / stride;
    const int output_height = input_height / stride;

    int input_length = input_width * input_height * input_depth;
    int output_length = output_width * output_height * output_depth;

    int output_depth_limit = output_depth / KERNEL_PARALLELISM;
    int input_depth_limit = input_depth / CHANNEL_PARALLELISM;

    elem_t *input_ptr = malloc(input_length * sizeof(elem_t));
    elem_t *weights_ptr;
    elem_t *output_ptr = malloc(output_length * sizeof(elem_t));
    elem_t *golden_output_ptr = malloc(output_length * sizeof(elem_t));

    memset(input_ptr, 1, input_length * sizeof(elem_t));

    initialize_weights(&weights_ptr, output_depth, input_depth, kernel_size);

    memset(output_ptr, 0, output_length * sizeof(elem_t));

    memset(golden_output_ptr, 0, output_length * sizeof(elem_t));

    elem_t(*input)[input_height][input_width][CHANNEL_PARALLELISM] = input_ptr;
    elem_t(*weights)[kernel_size][kernel_size][KERNEL_PARALLELISM][CHANNEL_PARALLELISM] = weights_ptr;
    elem_t(*output)[output_height][output_width][KERNEL_PARALLELISM] = output_ptr;
    elem_t(*golden_output)[output_height][output_width][KERNEL_PARALLELISM] = golden_output_ptr;

    pointwise_conv(input_width, input_height, input_depth, output_depth, kernel_size, stride, input, weights, output);

    int m, n, y, x, k, c, v, h;

    for (y = 0; y < output_height; y += 1)
    {
        for (x = 0; x < output_width; x += 1)
        {
            for (m = 0; m < output_depth; m++)
            {
                int partial = 0;
                int output_m, output_k;

                output_m = m / KERNEL_PARALLELISM;
                output_k = m % KERNEL_PARALLELISM;

                for (n = 0; n < input_depth; n += 1)
                {
                    for (v = 0; v < kernel_size; v += 1)
                    {
                        for (h = 0; h < kernel_size; h += 1)
                        {
                            int input_c, input_n;

                            input_n = n / CHANNEL_PARALLELISM;
                            input_c = n % CHANNEL_PARALLELISM;

                            // Padding 不一定是PADDING_LEFT。
                            int input_x = x * stride - kernel_size / 2 + h;
                            int input_y = y * stride - kernel_size / 2 + v;

                            elem_t input_value = 0;
                            // input_x < 0 || input_y + 2 > input_height - 1
                            if (input_x < 0 || input_x > input_width - 1 || input_y < 0 || input_y > input_height - 1)
                            {
                                input_value = 0;
                            }
                            else
                            {
                                input_value = input[input_n][input_y][input_x][input_c];
                            }

                            partial += input_value * weights[output_m * output_depth_limit + input_n][v][h][output_k][input_c];
                        }
                    }
                }

                golden_output[output_m][y][x][output_k] = partial;
            }
        }
    }

    return compare_array(output_width, output_height, output_depth, output, golden_output, __FUNCTION__);
}

int initialize_weights(elem_t **weights_ptr_pointer, int output_depth, int input_depth, int kernel_size)
{
    assert(weights_ptr_pointer != 0);

    int weights_length = output_depth * input_depth * kernel_size * kernel_size;
    int output_depth_limit = output_depth / KERNEL_PARALLELISM;

    int input_depth_limit = input_depth / CHANNEL_PARALLELISM;

    *weights_ptr_pointer = malloc(weights_length * sizeof(elem_t));

    elem_t(*weights)[kernel_size][kernel_size][KERNEL_PARALLELISM][CHANNEL_PARALLELISM] = *weights_ptr_pointer;

    memset(weights, 0, weights_length * sizeof(elem_t));

    for (int m = 0; m < output_depth_limit; m++)
    {
        for (int n = 0; n < input_depth_limit; n++)
        {
            for (int k = 0; k < KERNEL_PARALLELISM; k++)
                for (int c = 0; c < CHANNEL_PARALLELISM; c++)
                {
                    if (kernel_size == 3)
                    {
                        weights[m * input_depth_limit + n][0][0][k][c] = 6;
                        weights[m * input_depth_limit + n][0][1][k][c] = 3;
                        weights[m * input_depth_limit + n][0][2][k][c] = 0;

                        weights[m * input_depth_limit + n][1][0][k][c] = 1;
                        weights[m * input_depth_limit + n][1][1][k][c] = 2;
                        weights[m * input_depth_limit + n][1][2][k][c] = 0;

                        weights[m * input_depth_limit + n][2][0][k][c] = 0;
                        weights[m * input_depth_limit + n][2][1][k][c] = 0;
                        weights[m * input_depth_limit + n][2][2][k][c] = 0;
                    }
                    else
                    {
                        weights[m * input_depth_limit + n][0][0][k][c] = 6;
                    }
                }
        }
    }

    return 0;
}

int compare_array(int output_width, int output_height, int output_depth, const elem_t *output_ptr, const elem_t *golden_output_ptr, const char *info)
{
    int mismatch_count = 0;
    int match_count = 0;
    int m, y, x, k, v, h;

    elem_t(*output)[output_height][output_width][KERNEL_PARALLELISM] = output_ptr;
    elem_t(*golden_output)[output_height][output_width][KERNEL_PARALLELISM] = golden_output_ptr;

    for (m = 0; m < output_depth / KERNEL_PARALLELISM; m += 1)
    {
        for (y = 0; y < output_height; y += 1)
        {
            for (x = 0; x < output_width; x += 1)
            {
                for (k = 0; k < KERNEL_PARALLELISM; k++)
                {
                    if (output[m][y][x][k] != golden_output[m][y][x][k])
                    {
                        if (mismatch_count == 0)
                        {
                            printf("\nFirst counterpart(Output:%d; Golden:%d) position:[%d][%d][%d][%d]\n", output[m][y][x][k], golden_output[m][y][x][k], m, y, x, k);
                        }
                        mismatch_count++;
                    }
                    else
                    {
                        if (match_count == 0)
                        {
                            printf("\nFirst part(Output:%d; Golden:%d) position:[%d][%d][%d][%d]\n", output[m][y][x][k], golden_output[m][y][x][k], m, y, x, k);
                        }
                        match_count++;
                    }
                }
            }
        }
    }

    if (mismatch_count == 0)
    {
        printf("\n********** Test %s Passed **********\n", info);
    }
    else
    {
        printf("There are %d numbers mismatched.", mismatch_count);
        printf("\n********** Test %s Failed **********\n", info);
    }

    return mismatch_count;
}

int main(void)
{
    int mismatch_count = 0;

    mismatch_count += test_112_112_32_stride_2(112, 112, 32, 1);

    mismatch_count += test_112_112_32_stride_2(112, 112, 32, 2);

    mismatch_count += test_pointwise_conv(112, 112, 32, 32, 3, 1);

    mismatch_count += test_pointwise_conv(112, 112, 32, 32, 1, 1);

    return -mismatch_count;
}
