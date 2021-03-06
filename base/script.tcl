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
open_solution "base"
set_part {xczu3eg-sfvc784-1-i}
create_clock -period 4 -name default
source "./DepthwiseCONV-HLS/base/directives.tcl"
csim_design
csynth_design
cosim_design
export_design -format ip_catalog
