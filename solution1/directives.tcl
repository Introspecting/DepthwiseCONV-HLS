############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2019 Xilinx, Inc. All Rights Reserved.
############################################################
set_directive_array_partition -type cyclic -factor 8 -dim 2 "depthwise_conv" input
set_directive_array_partition -type cyclic -factor 3 -dim 1 "depthwise_conv" weights
set_directive_array_partition -type cyclic -factor 8 -dim 2 "depthwise_conv" output
set_directive_pipeline -II 1 "depthwise_conv/DEPTH_LOOP"
set_directive_pipeline "depthwise_conv/WIDTH_LOOP"
set_directive_array_partition -type complete -dim 2 "depthwise_conv" weights
set_directive_array_partition -type complete -dim 1 "depthwise_conv/HEIGHT_LOOP" partials_0
set_directive_array_partition -type complete -dim 1 "depthwise_conv/HEIGHT_LOOP" partials_1
set_directive_array_partition -type complete -dim 1 "depthwise_conv/HEIGHT_LOOP" out_0
