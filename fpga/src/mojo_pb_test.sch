<?xml version="1.0" encoding="UTF-8"?>
<drawing version="7">
    <attr value="spartan6" name="DeviceFamilyName">
        <trait delete="all:0" />
        <trait editname="all:0" />
        <trait edittrait="all:0" />
    </attr>
    <netlist>
        <signal name="SPI_MOSI" />
        <signal name="SPI_SS" />
        <signal name="SPI_SCK" />
        <signal name="SPI_MISO" />
        <signal name="DY1_RCLK" />
        <signal name="DY1_SCLK" />
        <signal name="DY1_SER" />
        <signal name="port_0(7:4)" />
        <signal name="port_0(3:0)" />
        <signal name="port_1(3:0)" />
        <signal name="XLXN_166(17:0)" />
        <signal name="port_0(7:0)" />
        <signal name="XLXN_174(7:0)" />
        <signal name="XLXN_176" />
        <signal name="XLXN_185(7:0)" />
        <signal name="XLXN_186(7:0)" />
        <signal name="XLXN_169(7:0)" />
        <signal name="XLXN_167(11:0)" />
        <signal name="XLXN_187(7:0)" />
        <signal name="XLXN_189" />
        <signal name="LED(7:0)" />
        <signal name="XLXN_170" />
        <signal name="port_1(7:0)" />
        <signal name="spi_rx(31:0)" />
        <signal name="DATA_BYTE(7:0)" />
        <signal name="XLXN_102(31:0)" />
        <signal name="XLXN_196(31:0)" />
        <signal name="CMD_TICK" />
        <signal name="BYTE_TICK" />
        <signal name="SYSCLK" />
        <signal name="SYSCLK50" />
        <signal name="SYSCLK100" />
        <signal name="XLXN_234" />
        <port polarity="Input" name="SPI_MOSI" />
        <port polarity="Input" name="SPI_SS" />
        <port polarity="Input" name="SPI_SCK" />
        <port polarity="BiDirectional" name="SPI_MISO" />
        <port polarity="Output" name="DY1_RCLK" />
        <port polarity="Output" name="DY1_SCLK" />
        <port polarity="Output" name="DY1_SER" />
        <port polarity="Output" name="LED(7:0)" />
        <port polarity="Output" name="DATA_BYTE(7:0)" />
        <port polarity="Output" name="CMD_TICK" />
        <port polarity="Output" name="BYTE_TICK" />
        <port polarity="Input" name="SYSCLK" />
        <blockdef name="fast_spi32">
            <timestamp>2026-8-23T11:32:21</timestamp>
            <line x2="480" y1="160" y2="160" x1="416" />
            <line x2="0" y1="-288" y2="-288" x1="64" />
            <line x2="0" y1="-224" y2="-224" x1="64" />
            <line x2="0" y1="-160" y2="-160" x1="64" />
            <line x2="0" y1="-96" y2="-96" x1="64" />
            <line x2="480" y1="-288" y2="-288" x1="416" />
            <line x2="480" y1="-96" y2="-96" x1="416" />
            <line x2="480" y1="-160" y2="-160" x1="416" />
            <line x2="480" y1="32" y2="32" x1="416" />
            <rect width="352" x="64" y="-320" height="508" />
            <rect width="64" x="0" y="-44" height="24" />
            <line x2="0" y1="-32" y2="-32" x1="64" />
            <rect width="64" x="416" y="-44" height="24" />
            <line x2="480" y1="-32" y2="-32" x1="416" />
            <rect width="64" x="416" y="84" height="24" />
            <line x2="480" y1="96" y2="96" x1="416" />
        </blockdef>
        <blockdef name="demo_dy1_new">
            <timestamp>2026-8-12T14:2:0</timestamp>
            <rect width="64" x="384" y="180" height="24" />
            <line x2="448" y1="192" y2="192" x1="384" />
            <line x2="448" y1="128" y2="128" x1="384" />
            <line x2="0" y1="-224" y2="-224" x1="64" />
            <line x2="448" y1="64" y2="64" x1="384" />
            <line x2="448" y1="-160" y2="-160" x1="384" />
            <line x2="448" y1="-96" y2="-96" x1="384" />
            <line x2="448" y1="-32" y2="-32" x1="384" />
            <rect width="320" x="64" y="-256" height="480" />
            <rect width="64" x="0" y="-172" height="24" />
            <line x2="0" y1="-160" y2="-160" x1="64" />
            <rect width="64" x="0" y="-108" height="24" />
            <line x2="0" y1="-96" y2="-96" x1="64" />
            <rect width="64" x="0" y="-44" height="24" />
            <line x2="0" y1="-32" y2="-32" x1="64" />
        </blockdef>
        <blockdef name="constant">
            <timestamp>2006-1-1T10:10:10</timestamp>
            <rect width="112" x="0" y="0" height="64" />
            <line x2="112" y1="32" y2="32" x1="144" />
        </blockdef>
        <blockdef name="pb_outport">
            <timestamp>2026-8-21T7:39:59</timestamp>
            <rect width="336" x="64" y="-256" height="256" />
            <line x2="0" y1="-224" y2="-224" x1="64" />
            <line x2="0" y1="-160" y2="-160" x1="64" />
            <rect width="64" x="0" y="-108" height="24" />
            <line x2="0" y1="-96" y2="-96" x1="64" />
            <rect width="64" x="0" y="-44" height="24" />
            <line x2="0" y1="-32" y2="-32" x1="64" />
            <rect width="64" x="400" y="-236" height="24" />
            <line x2="464" y1="-224" y2="-224" x1="400" />
            <rect width="64" x="400" y="-172" height="24" />
            <line x2="464" y1="-160" y2="-160" x1="400" />
        </blockdef>
        <blockdef name="pb_inport">
            <timestamp>2026-8-21T13:5:13</timestamp>
            <rect width="336" x="64" y="-320" height="320" />
            <line x2="0" y1="-288" y2="-288" x1="64" />
            <line x2="0" y1="-224" y2="-224" x1="64" />
            <rect width="64" x="0" y="-172" height="24" />
            <line x2="0" y1="-160" y2="-160" x1="64" />
            <rect width="64" x="0" y="-108" height="24" />
            <line x2="0" y1="-96" y2="-96" x1="64" />
            <rect width="64" x="0" y="-44" height="24" />
            <line x2="0" y1="-32" y2="-32" x1="64" />
            <rect width="64" x="400" y="-236" height="24" />
            <line x2="464" y1="-224" y2="-224" x1="400" />
        </blockdef>
        <blockdef name="kcpsm6">
            <timestamp>2026-8-22T16:42:15</timestamp>
            <rect width="336" x="64" y="-512" height="512" />
            <rect width="64" x="0" y="-172" height="24" />
            <line x2="0" y1="-160" y2="-160" x1="64" />
            <line x2="464" y1="-480" y2="-480" x1="400" />
            <line x2="464" y1="-416" y2="-416" x1="400" />
            <line x2="464" y1="-352" y2="-352" x1="400" />
            <line x2="464" y1="-288" y2="-288" x1="400" />
            <line x2="464" y1="-224" y2="-224" x1="400" />
            <rect width="64" x="400" y="-172" height="24" />
            <line x2="464" y1="-160" y2="-160" x1="400" />
            <rect width="64" x="400" y="-108" height="24" />
            <line x2="464" y1="-96" y2="-96" x1="400" />
            <rect width="64" x="400" y="-44" height="24" />
            <line x2="464" y1="-32" y2="-32" x1="400" />
            <line x2="0" y1="-480" y2="-480" x1="64" />
            <line x2="0" y1="-272" y2="-272" x1="64" />
            <line x2="0" y1="-416" y2="-416" x1="64" />
            <line x2="0" y1="-336" y2="-336" x1="64" />
            <rect width="64" x="0" y="-108" height="24" />
            <line x2="0" y1="-96" y2="-96" x1="64" />
        </blockdef>
        <blockdef name="buf">
            <timestamp>2000-1-1T10:10:10</timestamp>
            <line x2="64" y1="-32" y2="-32" x1="0" />
            <line x2="128" y1="-32" y2="-32" x1="224" />
            <line x2="128" y1="0" y2="-32" x1="64" />
            <line x2="64" y1="-32" y2="-64" x1="128" />
            <line x2="64" y1="-64" y2="0" x1="64" />
        </blockdef>
        <blockdef name="bram_wrapper_pb6">
            <timestamp>2026-8-23T11:48:25</timestamp>
            <rect width="64" x="528" y="36" height="24" />
            <line x2="592" y1="48" y2="48" x1="528" />
            <rect width="64" x="528" y="100" height="24" />
            <line x2="592" y1="112" y2="112" x1="528" />
            <rect width="64" x="528" y="164" height="24" />
            <line x2="592" y1="176" y2="176" x1="528" />
            <rect width="64" x="528" y="228" height="24" />
            <line x2="592" y1="240" y2="240" x1="528" />
            <line x2="0" y1="-416" y2="-416" x1="64" />
            <line x2="0" y1="-352" y2="-352" x1="64" />
            <rect width="64" x="0" y="-300" height="24" />
            <line x2="0" y1="-288" y2="-288" x1="64" />
            <rect width="64" x="528" y="-300" height="24" />
            <line x2="592" y1="-288" y2="-288" x1="528" />
            <rect width="64" x="528" y="-28" height="24" />
            <line x2="592" y1="-16" y2="-16" x1="528" />
            <rect width="64" x="0" y="-28" height="24" />
            <line x2="0" y1="-16" y2="-16" x1="64" />
            <rect width="64" x="528" y="-236" height="24" />
            <line x2="592" y1="-224" y2="-224" x1="528" />
            <line x2="592" y1="-128" y2="-128" x1="528" />
            <rect width="64" x="0" y="228" height="24" />
            <line x2="0" y1="240" y2="240" x1="64" />
            <line x2="0" y1="-224" y2="-224" x1="64" />
            <rect width="464" x="64" y="-448" height="716" />
            <rect width="64" x="0" y="-172" height="24" />
            <line x2="0" y1="-160" y2="-160" x1="64" />
        </blockdef>
        <blockdef name="dcm_wrapper">
            <timestamp>2026-8-15T17:15:8</timestamp>
            <rect width="304" x="64" y="-128" height="128" />
            <line x2="0" y1="-96" y2="-96" x1="64" />
            <line x2="432" y1="-96" y2="-96" x1="368" />
            <line x2="432" y1="-32" y2="-32" x1="368" />
        </blockdef>
        <block symbolname="demo_dy1_new" name="XLXI_8">
            <blockpin signalname="SYSCLK50" name="SYSCLK50" />
            <blockpin name="LED_BLINK" />
            <blockpin name="TICK_12HZ" />
            <blockpin signalname="DY1_RCLK" name="RCLK" />
            <blockpin signalname="DY1_SCLK" name="SCLK" />
            <blockpin signalname="DY1_SER" name="SER" />
            <blockpin name="BLINK_COUNT(7:0)" />
            <blockpin signalname="port_1(3:0)" name="DIGIT_L(3:0)" />
            <blockpin signalname="port_0(7:4)" name="DIGIT_M(3:0)" />
            <blockpin signalname="port_0(3:0)" name="DIGIT_R(3:0)" />
        </block>
        <block symbolname="fast_spi32" name="XLXI_1">
            <blockpin signalname="SYSCLK100" name="SYSCLK" />
            <blockpin signalname="SPI_SCK" name="SCK" />
            <blockpin signalname="SPI_SS" name="SS" />
            <blockpin signalname="SPI_MOSI" name="MOSI" />
            <blockpin signalname="XLXN_102(31:0)" name="SPI_TX(31:0)" />
            <blockpin signalname="SPI_MISO" name="MISO" />
            <blockpin name="START_TICK" />
            <blockpin signalname="CMD_TICK" name="CMD_TICK" />
            <blockpin signalname="BYTE_TICK" name="BYTE_TICK" />
            <blockpin signalname="XLXN_234" name="END_TICK" />
            <blockpin signalname="spi_rx(31:0)" name="SPI_RX(31:0)" />
            <blockpin signalname="DATA_BYTE(7:0)" name="BYTE_RX(7:0)" />
        </block>
        <block symbolname="pb_inport" name="XLXI_47">
            <blockpin signalname="SYSCLK100" name="SYSCLK" />
            <blockpin signalname="XLXN_176" name="PORT_RD" />
            <blockpin signalname="XLXN_169(7:0)" name="PORT_ID(7:0)" />
            <blockpin signalname="XLXN_185(7:0)" name="PORT_0(7:0)" />
            <blockpin signalname="XLXN_186(7:0)" name="PORT_1(7:0)" />
            <blockpin signalname="XLXN_174(7:0)" name="PORT_DATA(7:0)" />
        </block>
        <block symbolname="kcpsm6" name="XLXI_48">
            <blockpin signalname="XLXN_166(17:0)" name="instruction(17:0)" />
            <blockpin name="bram_enable" />
            <blockpin signalname="XLXN_170" name="write_strobe" />
            <blockpin name="k_write_strobe" />
            <blockpin signalname="XLXN_176" name="read_strobe" />
            <blockpin name="interrupt_ack" />
            <blockpin signalname="XLXN_167(11:0)" name="address(11:0)" />
            <blockpin signalname="XLXN_187(7:0)" name="out_port(7:0)" />
            <blockpin signalname="XLXN_169(7:0)" name="port_id(7:0)" />
            <blockpin signalname="SYSCLK100" name="clk" />
            <blockpin signalname="XLXN_189" name="reset" />
            <blockpin name="interrupt" />
            <blockpin name="sleep" />
            <blockpin signalname="XLXN_174(7:0)" name="in_port(7:0)" />
        </block>
        <block symbolname="buf" name="XLXI_49(7:0)">
            <blockpin signalname="port_0(7:0)" name="I" />
            <blockpin signalname="LED(7:0)" name="O" />
        </block>
        <block symbolname="pb_outport" name="XLXI_46">
            <blockpin signalname="SYSCLK100" name="SYSCLK" />
            <blockpin signalname="XLXN_170" name="PORT_WR" />
            <blockpin signalname="XLXN_169(7:0)" name="PORT_ID(7:0)" />
            <blockpin signalname="XLXN_187(7:0)" name="PORT_DATA(7:0)" />
            <blockpin signalname="port_0(7:0)" name="PORT_0(7:0)" />
            <blockpin signalname="port_1(7:0)" name="PORT_1(7:0)" />
        </block>
        <block symbolname="bram_wrapper_pb6" name="XLXI_52">
            <blockpin signalname="SYSCLK100" name="SYSCLK" />
            <blockpin signalname="CMD_TICK" name="CMD_TICK" />
            <blockpin signalname="XLXN_234" name="END_TICK" />
            <blockpin signalname="spi_rx(31:0)" name="SPI_RX(31:0)" />
            <blockpin signalname="XLXN_196(31:0)" name="VERSION(31:0)" />
            <blockpin signalname="XLXN_167(11:0)" name="PB_ADDR(11:0)" />
            <blockpin signalname="XLXN_166(17:0)" name="PB_DATA(17:0)" />
            <blockpin signalname="XLXN_189" name="RESET" />
            <blockpin signalname="XLXN_102(31:0)" name="SPI_TX(31:0)" />
            <blockpin name="CFG_PORT(3:0)" />
            <blockpin signalname="XLXN_185(7:0)" name="OUTPORT_0(7:0)" />
            <blockpin signalname="XLXN_186(7:0)" name="OUTPORT_1(7:0)" />
            <blockpin name="OUTPORT_2(7:0)" />
            <blockpin name="OUTPORT_3(7:0)" />
            <blockpin signalname="DATA_BYTE(7:0)" name="CMD_BYTE(7:0)" />
        </block>
        <block symbolname="constant" name="XLXI_29">
            <attr value="22082026" name="CValue">
                <trait delete="all:1 sym:0" />
                <trait editname="all:1 sch:0" />
                <trait valuetype="BitVector 32 Hexadecimal" />
            </attr>
            <blockpin signalname="XLXN_196(31:0)" name="O" />
        </block>
        <block symbolname="dcm_wrapper" name="XLXI_53">
            <blockpin signalname="SYSCLK" name="CLK_50_IN" />
            <blockpin signalname="SYSCLK100" name="CLK_100_OUT" />
            <blockpin signalname="SYSCLK50" name="CLK_50_OUT" />
        </block>
    </netlist>
    <sheet sheetnum="1" width="5440" height="3520">
        <branch name="SPI_MOSI">
            <wire x2="1248" y1="1072" y2="1072" x1="992" />
        </branch>
        <branch name="SPI_SS">
            <wire x2="1248" y1="1008" y2="1008" x1="992" />
        </branch>
        <branch name="SPI_SCK">
            <wire x2="1248" y1="944" y2="944" x1="992" />
        </branch>
        <branch name="SPI_MISO">
            <wire x2="1792" y1="880" y2="880" x1="1728" />
        </branch>
        <branch name="DY1_RCLK">
            <wire x2="3472" y1="2272" y2="2272" x1="3328" />
        </branch>
        <branch name="DY1_SCLK">
            <wire x2="3472" y1="2336" y2="2336" x1="3328" />
        </branch>
        <branch name="DY1_SER">
            <wire x2="3472" y1="2400" y2="2400" x1="3328" />
        </branch>
        <branch name="port_0(7:4)">
            <attrtext style="alignment:SOFT-RIGHT;fontsize:28;fontname:Arial" attrname="Name" x="2768" y="2336" type="branch" />
            <wire x2="2880" y1="2336" y2="2336" x1="2768" />
        </branch>
        <branch name="port_0(3:0)">
            <attrtext style="alignment:SOFT-RIGHT;fontsize:28;fontname:Arial" attrname="Name" x="2768" y="2400" type="branch" />
            <wire x2="2880" y1="2400" y2="2400" x1="2768" />
        </branch>
        <branch name="port_1(3:0)">
            <attrtext style="alignment:SOFT-RIGHT;fontsize:28;fontname:Arial" attrname="Name" x="2768" y="2272" type="branch" />
            <wire x2="2880" y1="2272" y2="2272" x1="2768" />
        </branch>
        <instance x="2880" y="2432" name="XLXI_8" orien="R0">
        </instance>
        <instance x="1248" y="1168" name="XLXI_1" orien="R0">
        </instance>
        <branch name="XLXN_166(17:0)">
            <wire x2="2912" y1="1408" y2="1408" x1="2688" />
        </branch>
        <branch name="port_0(7:0)">
            <attrtext style="alignment:SOFT-LEFT;fontsize:28;fontname:Arial" attrname="Name" x="4176" y="1280" type="branch" />
            <wire x2="4112" y1="1280" y2="1280" x1="4064" />
            <wire x2="4176" y1="1280" y2="1280" x1="4112" />
            <wire x2="4144" y1="1136" y2="1136" x1="4112" />
            <wire x2="4112" y1="1136" y2="1280" x1="4112" />
        </branch>
        <branch name="XLXN_174(7:0)">
            <wire x2="2912" y1="1472" y2="1472" x1="2864" />
            <wire x2="2864" y1="1472" y2="1632" x1="2864" />
            <wire x2="4144" y1="1632" y2="1632" x1="2864" />
            <wire x2="4144" y1="1632" y2="1824" x1="4144" />
            <wire x2="4144" y1="1824" y2="1824" x1="4064" />
        </branch>
        <instance x="3600" y="2048" name="XLXI_47" orien="R0">
        </instance>
        <branch name="XLXN_176">
            <wire x2="3408" y1="1280" y2="1280" x1="3376" />
            <wire x2="3408" y1="1280" y2="1824" x1="3408" />
            <wire x2="3600" y1="1824" y2="1824" x1="3408" />
        </branch>
        <branch name="XLXN_185(7:0)">
            <wire x2="2816" y1="1472" y2="1472" x1="2688" />
            <wire x2="2816" y1="1472" y2="1952" x1="2816" />
            <wire x2="3600" y1="1952" y2="1952" x1="2816" />
        </branch>
        <branch name="XLXN_186(7:0)">
            <wire x2="2768" y1="1536" y2="1536" x1="2688" />
            <wire x2="2768" y1="1536" y2="2016" x1="2768" />
            <wire x2="3600" y1="2016" y2="2016" x1="2768" />
        </branch>
        <instance x="2912" y="1568" name="XLXI_48" orien="R0">
        </instance>
        <branch name="XLXN_169(7:0)">
            <wire x2="3472" y1="1536" y2="1536" x1="3376" />
            <wire x2="3472" y1="1536" y2="1888" x1="3472" />
            <wire x2="3600" y1="1888" y2="1888" x1="3472" />
            <wire x2="3600" y1="1408" y2="1408" x1="3472" />
            <wire x2="3472" y1="1408" y2="1536" x1="3472" />
        </branch>
        <branch name="XLXN_167(11:0)">
            <wire x2="2096" y1="1408" y2="1408" x1="2064" />
            <wire x2="2064" y1="1408" y2="1760" x1="2064" />
            <wire x2="3440" y1="1760" y2="1760" x1="2064" />
            <wire x2="3440" y1="1408" y2="1408" x1="3376" />
            <wire x2="3440" y1="1408" y2="1760" x1="3440" />
        </branch>
        <branch name="XLXN_187(7:0)">
            <wire x2="3600" y1="1472" y2="1472" x1="3376" />
        </branch>
        <branch name="XLXN_189">
            <wire x2="2912" y1="1296" y2="1296" x1="2688" />
        </branch>
        <instance x="4144" y="1168" name="XLXI_49(7:0)" orien="R0" />
        <branch name="LED(7:0)">
            <wire x2="4400" y1="1136" y2="1136" x1="4368" />
        </branch>
        <instance x="3600" y="1504" name="XLXI_46" orien="R0">
        </instance>
        <branch name="XLXN_170">
            <wire x2="3472" y1="1152" y2="1152" x1="3376" />
            <wire x2="3472" y1="1152" y2="1344" x1="3472" />
            <wire x2="3600" y1="1344" y2="1344" x1="3472" />
        </branch>
        <branch name="port_1(7:0)">
            <attrtext style="alignment:SOFT-LEFT;fontsize:28;fontname:Arial" attrname="Name" x="4176" y="1344" type="branch" />
            <wire x2="4176" y1="1344" y2="1344" x1="4064" />
        </branch>
        <instance x="2096" y="1424" name="XLXI_52" orien="R0">
        </instance>
        <branch name="spi_rx(31:0)">
            <attrtext style="alignment:SOFT-BCENTER;fontsize:28;fontname:Arial" attrname="Name" x="1968" y="1136" type="branch" />
            <wire x2="1968" y1="1136" y2="1136" x1="1728" />
            <wire x2="2096" y1="1136" y2="1136" x1="1968" />
        </branch>
        <instance x="784" y="1632" name="XLXI_29" orien="R0">
        </instance>
        <branch name="XLXN_102(31:0)">
            <wire x2="2768" y1="768" y2="768" x1="1136" />
            <wire x2="2768" y1="768" y2="1136" x1="2768" />
            <wire x2="1136" y1="768" y2="1136" x1="1136" />
            <wire x2="1248" y1="1136" y2="1136" x1="1136" />
            <wire x2="2768" y1="1136" y2="1136" x1="2688" />
        </branch>
        <branch name="XLXN_196(31:0)">
            <wire x2="2096" y1="1664" y2="1664" x1="928" />
        </branch>
        <branch name="CMD_TICK">
            <wire x2="1776" y1="1200" y2="1200" x1="1728" />
            <wire x2="2096" y1="1200" y2="1200" x1="1776" />
            <wire x2="1776" y1="1200" y2="2000" x1="1776" />
            <wire x2="1936" y1="2000" y2="2000" x1="1776" />
        </branch>
        <iomarker fontsize="28" x="1792" y="880" name="SPI_MISO" orien="R0" />
        <iomarker fontsize="28" x="3472" y="2272" name="DY1_RCLK" orien="R0" />
        <iomarker fontsize="28" x="3472" y="2336" name="DY1_SCLK" orien="R0" />
        <iomarker fontsize="28" x="3472" y="2400" name="DY1_SER" orien="R0" />
        <iomarker fontsize="28" x="992" y="944" name="SPI_SCK" orien="R180" />
        <iomarker fontsize="28" x="992" y="1008" name="SPI_SS" orien="R180" />
        <iomarker fontsize="28" x="992" y="1072" name="SPI_MOSI" orien="R180" />
        <iomarker fontsize="28" x="4400" y="1136" name="LED(7:0)" orien="R0" />
        <iomarker fontsize="28" x="1936" y="1936" name="BYTE_TICK" orien="R0" />
        <iomarker fontsize="28" x="1936" y="2000" name="CMD_TICK" orien="R0" />
        <iomarker fontsize="28" x="1936" y="1856" name="DATA_BYTE(7:0)" orien="R0" />
        <branch name="SYSCLK">
            <wire x2="960" y1="560" y2="560" x1="880" />
        </branch>
        <iomarker fontsize="28" x="880" y="560" name="SYSCLK" orien="R180" />
        <branch name="SYSCLK100">
            <wire x2="1184" y1="736" y2="880" x1="1184" />
            <wire x2="1248" y1="880" y2="880" x1="1184" />
            <wire x2="1440" y1="736" y2="736" x1="1184" />
            <wire x2="1440" y1="560" y2="560" x1="1392" />
            <wire x2="1440" y1="560" y2="736" x1="1440" />
            <wire x2="2032" y1="560" y2="560" x1="1440" />
            <wire x2="2864" y1="560" y2="560" x1="2032" />
            <wire x2="2864" y1="560" y2="1088" x1="2864" />
            <wire x2="2912" y1="1088" y2="1088" x1="2864" />
            <wire x2="3504" y1="560" y2="560" x1="2864" />
            <wire x2="3504" y1="560" y2="1280" x1="3504" />
            <wire x2="3600" y1="1280" y2="1280" x1="3504" />
            <wire x2="3504" y1="1280" y2="1760" x1="3504" />
            <wire x2="3600" y1="1760" y2="1760" x1="3504" />
            <wire x2="2032" y1="560" y2="1008" x1="2032" />
            <wire x2="2096" y1="1008" y2="1008" x1="2032" />
        </branch>
        <instance x="960" y="656" name="XLXI_53" orien="R0">
        </instance>
        <branch name="SYSCLK50">
            <wire x2="2736" y1="624" y2="624" x1="1392" />
            <wire x2="2736" y1="624" y2="2208" x1="2736" />
            <wire x2="2880" y1="2208" y2="2208" x1="2736" />
        </branch>
        <branch name="BYTE_TICK">
            <wire x2="1824" y1="1328" y2="1328" x1="1728" />
            <wire x2="1824" y1="1328" y2="1936" x1="1824" />
            <wire x2="1936" y1="1936" y2="1936" x1="1824" />
        </branch>
        <branch name="DATA_BYTE(7:0)">
            <wire x2="1872" y1="1264" y2="1264" x1="1728" />
            <wire x2="1872" y1="1264" y2="1856" x1="1872" />
            <wire x2="1936" y1="1856" y2="1856" x1="1872" />
            <wire x2="2096" y1="1264" y2="1264" x1="1872" />
        </branch>
        <branch name="XLXN_234">
            <wire x2="2096" y1="1072" y2="1072" x1="1728" />
        </branch>
    </sheet>
</drawing>