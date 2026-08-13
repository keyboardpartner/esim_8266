# Tools for EPROM Simulator Uploader

### GODIL Tools

<img src="/docs/godil.jpg" width="320">

For programming the GODIL's internal SPI flash, you need a JTAG adaptor from Digilent like JTAG-SPI USB Full-Speed RevC or never. While the FPGA can be loaded with *Digilent Adept* app (volatile, good for development), you need the CLI-based Digilent *sfutil.exe* to permanently store the FPGA configuration in it's SPI flash. Provided here are two drag & drop capable batch files to read the flash mem's ID code and to program it via *sfutil.exe*.

Useful links:

[Digilent Adept Overview](https://digilent.com/shop/software/digilent-adept/)

[Digilent Adept Download](https://lp.digilent.com/complete-adept-utilities-download)

[Digilent SFUTIL SPI flash utility (Windows)](https://digilent.com/reference/software/serial-flash-utility/start)

