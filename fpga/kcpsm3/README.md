# MOJO Picoblaze Demo

The XILINX Webpack ISE project *MojoFPGA_Picoblaze.xise* contains a Picoblaze CPU core which lets the onboard-LEDs blink, demonstrating the upload of Picoblaze Instruction ROM instead of using a "hard-wired" BRAM contents defined in VHDL synthesis. This might be used for the GODIL as well (needs some project changes).

For convenience, we provide a compiled version of [Jan Viktorin's picoblaze assembler](https://github.com/jviki/kcpsm3) for Windows PCs and a Drag&Drop batch file. Note that Ken Chapman's original KCPSM3 no longer works on 64-Bit Windows.

After uploading the *mojo_pb_test.bit* configuration, stream the *picoblaze_test.dat* file to the Mojo FPGA using the Upload web page provided by the ESP8266.

