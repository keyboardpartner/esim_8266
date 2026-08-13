echo off
echo Flash a binary FPGA file with Digilent JTAG-SPI Full Speed via SPI
E:\Xilinx\AdeptUtilities\sfutil -d JtagUsbFs -id
E:\Xilinx\AdeptUtilities\sfutil -d JtagUsbFs -w %1 -fb 
pause