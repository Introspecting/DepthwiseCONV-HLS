############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2019 Xilinx, Inc. All Rights Reserved.
############################################################
open_project DepthwiseCONV-HLS
set_top depthwise_top
add_files DepthwiseCONV-HLS/test_unroll.c
add_files DepthwiseCONV-HLS/depthwise.c
add_files -tb DepthwiseCONV-HLS/depthwise_test.c -cflags "-Wno-unknown-pragmas" -csimflags "-Wno-unknown-pragmas"
open_solution "optimized"
set_part {xczu3eg-sfvc784-1-i} -tool vivado
create_clock -period 4 -name default
config_compile -name_max_length 20 -no_signed_zeros=0 -pipeline_loops 0 -unsafe_math_optimizations=0
config_sdx -target none
config_export -format ip_catalog -rtl verilog -vivado_optimization_level 2 -vivado_phys_opt place -vivado_report_level 0
set_clock_uncertainty 12.5%
source "./DepthwiseCONV-HLS/optimized/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -flow impl -rtl verilog -format ip_catalog
