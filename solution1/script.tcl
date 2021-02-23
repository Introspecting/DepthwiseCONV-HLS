############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2019 Xilinx, Inc. All Rights Reserved.
############################################################
open_project DepthwiseCONV-HLS
set_top depthwise_conv
add_files DepthwiseCONV-HLS/depthwise.c
add_files -tb DepthwiseCONV-HLS/depthwise_test.c
open_solution "solution1"
set_part {xczu3eg-sfvc784-1-i} -tool vivado
create_clock -period 5 -name default
source "./DepthwiseCONV-HLS/solution1/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -format ip_catalog
