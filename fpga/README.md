# EPROM Simulator Uploader - FPGA Sources

Copy contents to own Xilinx ISE FPGA project folder and open fpga_main.xise there. Add **one** UCF file (pin assignment) to your project (either GODIL or MOJO, not both). Also add memory IP core (*sram_32k.xco* for GODIL and *sram64k.xco* for MOJO) from *ipcore_dir* directory.

Data exchange happens through a 32-bit SPI register in FPGA. See *bram_wrapper.vhd* for SPI command set.

### Note to myself: Sources must be copied to this folder manually due to complexity of Xilinx ISE projects. Do not run ISE in this folder as it creates a mess.

