# MOJO Picoblaze Demo

The XILINX Webpack ISE project *MojoFPGA_Picoblaze.xise* contains a Picoblaze CPU core which lets the onboard-LEDs blink, demonstrating the upload of Picoblaze Instruction ROM instead of using "hard-wired" BRAM contents defined in VHDL synthesis. 

After uploading the *mojo_pb_test.bit* configuration, stream the *picoblaze_test.dat* file to the Mojo FPGA using the Upload web page provided by the ESP8266.

For convenience, we provide a Drag&Drop batch file for [Ken Chapmman's 6-series picoblaze assembler](https://www.amd.com/en/products/adaptive-socs-and-fpgas/intellectual-property/picoblaze.html) for Windows PCs.
