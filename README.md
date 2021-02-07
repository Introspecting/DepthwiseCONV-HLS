# DepthwiseCONV-HLS
使用HLS设计一个可分卷积（High Level Synthesis）模块，以在FPGA上对其进行加速。

## 介绍

这是一个Xilinx XUP中用于巩固HLS基础的项目。

可分卷积被应用在许多轻型神经网络中，它能以更少的参数量和计算量（大约可以降低一个量级）来达到相似的精度。在性能受限的嵌入式环境中，这个优点尤为突出。因此，我们想在xczu3eg这样一款面向嵌入式环境的FPGA上实现一个用于加速可分卷积的功能模块。为了巩固还不怎么熟悉的HLS和节约时间，我们将用HLS C作为编程语言。

## 目标

设计一个能够对可分卷积进行加速的HLS模块。它使用AXI接口读取输入数据和权值，并且在计算过程中能对它们进行复用以减少数据的搬移。同时，充分利用FPGA的资源也是设计的一个原则，这里的资源主要是DSP和BRAM。数据类型预期将使用fp16/int16，因为我还不太会量化😓。

## 实验环境

FPGA：xczu3eg-sfvc784-1-i
开发工具：Vivado HLS 2018.3
操作系统：Windows 10

## 进度安排

从1.8到1.26，时间大约3周：
1：通过示例熟悉HLS和AXI接口；
2：实现可分卷积中的分层卷积；
3：实现可分卷积中的1x1卷积。

## 备注

不知道能做到什么程度，可能性能不会太好，但不论什么情况可分卷积模块都必须完成！希望后续能够做的完善一些。