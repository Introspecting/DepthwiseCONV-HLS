############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2019 Xilinx, Inc. All Rights Reserved.
############################################################
set_directive_array_partition -type cyclic -factor 8 -dim 2 "depthwise_conv_per" input
set_directive_array_partition -type cyclic -factor 3 -dim 1 "depthwise_conv_per" weights
set_directive_array_partition -type cyclic -factor 8 -dim 2 "depthwise_conv_per" output
set_directive_pipeline "depthwise_conv_per/WIDTH_LOOP"
set_directive_array_partition -type complete -dim 2 "depthwise_conv_per" weights
set_directive_dependence -variable output -class array -type intra -dependent false "depthwise_conv_per"
set_directive_array_partition -type complete -dim 1 "depthwise_conv_per" partials_0
set_directive_array_partition -type complete -dim 1 "depthwise_conv_per" partials_1
set_directive_array_partition -type complete -dim 1 "depthwise_conv_per" out_0
set_directive_array_partition -type complete -dim 1 "depthwise_conv_per" results_0
set_directive_dependence -variable output -class array -type inter -dependent false "depthwise_conv_per"
set_directive_array_partition -type complete -dim 2 "pointwise_conv_per" input
set_directive_array_partition -type complete -dim 2 "pointwise_conv_per" weights
set_directive_array_partition -type complete -dim 2 "pointwise_conv_per" output
set_directive_array_partition -type complete -dim 1 "pointwise_conv_per" out_0
set_directive_array_partition -type complete -dim 1 "pointwise_conv_per" results_0
set_directive_array_partition -type complete -dim 1 "pointwise_conv_per" inputs_0
set_directive_array_partition -type complete -dim 2 "pointwise_conv_per" weight_values
set_directive_pipeline -II 1 "pointwise_conv_per/HEIGHT_LOOP"
set_directive_array_partition -type complete -dim 1 "pointwise_conv_per" weight_values
set_directive_array_partition -type complete -dim 1 "test_loop" out_0
set_directive_array_partition -type complete -dim 1 "test_loop" results_0
set_directive_array_partition -type complete -dim 1 "test_loop" inputs_0
set_directive_array_partition -type complete -dim 1 "test_loop" weight_values
set_directive_pipeline "test_loop/XLOOP"
set_directive_array_partition -type complete -dim 2 "test_loop" input
set_directive_array_partition -type complete -dim 2 "test_loop" weights
set_directive_array_partition -type complete -dim 2 "test_loop" output
set_directive_pipeline "test_loop/LOAD_WEIGHT_VALUES_LOOP"
set_directive_array_partition -type complete -dim 2 "test_loop" weight_values
set_directive_loop_flatten "test_loop/LOAD_WEIGHT_VALUES_LOOP"
set_directive_pipeline "pointwise_conv_per/X_LOOP"
set_directive_dependence -variable output -class array -type inter -dependent false "test_loop"
set_directive_dependence -variable output -class array -type inter -direction RAW -dependent false "pointwise_conv_per"
set_directive_pipeline "pointwise_conv_per/LOAD_WEIGHT_VALUES_LOOP"
set_directive_loop_tripcount -min 112 -max 112 "depthwise_conv_per/WIDTH_LOOP"
set_directive_loop_tripcount -min 112 -max 112 "depthwise_conv_per/HEIGHT_LOOP"
set_directive_loop_tripcount -min 112 -max 112 "depthwise_conv_per/KERNEL_V_LOOP"
set_directive_dataflow "depthwise_top"
set_directive_dataflow "depthwise_top/depthwise_top_label0"
set_directive_allocation -type function "read_data" 2
set_directive_dataflow "read_data"
