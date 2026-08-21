<?xml version="1.0" encoding="UTF-8"?>
<drawing version="7">
    <attr value="spartan6" name="DeviceFamilyName">
        <trait delete="all:0" />
        <trait editname="all:0" />
        <trait edittrait="all:0" />
    </attr>
    <netlist>
        <signal name="data_rx(31:0)" />
        <signal name="XLXN_90" />
        <signal name="SPI_MOSI" />
        <signal name="SPI_SS" />
        <signal name="SPI_SCK" />
        <signal name="SPI_MISO" />
        <signal name="XLXN_102(31:0)" />
        <signal name="XLXN_105(31:0)" />
        <signal name="DY1_RCLK" />
        <signal name="DY1_SCLK" />
        <signal name="DY1_SER" />
        <signal name="port_0(7:4)" />
        <signal name="port_0(3:0)" />
        <signal name="port_1(3:0)" />
        <signal name="SYSCLK" />
        <signal name="XLXN_166(17:0)" />
        <signal name="XLXN_167(11:0)" />
        <signal name="XLXN_169(7:0)" />
        <signal name="XLXN_170" />
        <signal name="port_0(7:0)" />
        <signal name="port_1(7:0)" />
        <signal name="XLXN_174(7:0)" />
        <signal name="XLXN_176" />
        <signal name="XLXN_185(7:0)" />
        <signal name="XLXN_186(7:0)" />
        <signal name="XLXN_187(7:0)" />
        <signal name="XLXN_188" />
        <signal name="XLXN_189" />
        <signal name="LED(7:0)" />
        <signal name="XLXN_191(7:0)" />
        <signal name="XLXN_193(7:0)" />
        <signal name="XLXN_194(7:0)" />
        <port polarity="Input" name="SPI_MOSI" />
        <port polarity="Input" name="SPI_SS" />
        <port polarity="Input" name="SPI_SCK" />
        <port polarity="BiDirectional" name="SPI_MISO" />
        <port polarity="Output" name="DY1_RCLK" />
        <port polarity="Output" name="DY1_SCLK" />
        <port polarity="Output" name="DY1_SER" />
        <port polarity="Input" name="SYSCLK" />
        <port polarity="Output" name="LED(7:0)" />
        <blockdef name="fast_spi32">
            <timestamp>2026-8-19T7:30:32</timestamp>
            <rect width="352" x="64" y="-320" height="320" />
            <line x2="0" y1="-288" y2="-288" x1="64" />
            <line x2="0" y1="-224" y2="-224" x1="64" />
            <line x2="0" y1="-160" y2="-160" x1="64" />
            <line x2="0" y1="-96" y2="-96" x1="64" />
            <rect width="64" x="0" y="-44" height="24" />
            <line x2="0" y1="-32" y2="-32" x1="64" />
            <line x2="480" y1="-288" y2="-288" x1="416" />
            <rect width="64" x="416" y="-44" height="24" />
            <line x2="480" y1="-32" y2="-32" x1="416" />
            <line x2="480" y1="-96" y2="-96" x1="416" />
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
            <timestamp>2026-8-21T14:37:47</timestamp>
            <rect width="336" x="64" y="-512" height="512" />
            <rect width="64" x="0" y="-172" height="24" />
            <line x2="0" y1="-160" y2="-160" x1="64" />
            <rect width="64" x="0" y="-92" height="24" />
            <line x2="0" y1="-80" y2="-80" x1="64" />
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
            <timestamp>2026-8-21T15:26:5</timestamp>
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
            <rect width="64" x="0" y="-236" height="24" />
            <line x2="0" y1="-224" y2="-224" x1="64" />
            <rect width="64" x="528" y="-28" height="24" />
            <line x2="592" y1="-16" y2="-16" x1="528" />
            <rect width="64" x="0" y="-28" height="24" />
            <line x2="0" y1="-16" y2="-16" x1="64" />
            <rect width="64" x="528" y="-236" height="24" />
            <line x2="592" y1="-224" y2="-224" x1="528" />
            <line x2="592" y1="-128" y2="-128" x1="528" />
            <rect width="464" x="64" y="-448" height="720" />
        </blockdef>
        <block symbolname="fast_spi32" name="XLXI_1">
            <blockpin signalname="SYSCLK" name="SYSCLK" />
            <blockpin signalname="SPI_SCK" name="SCK" />
            <blockpin signalname="SPI_SS" name="SS" />
            <blockpin signalname="SPI_MOSI" name="MOSI" />
            <blockpin signalname="XLXN_102(31:0)" name="DATA_TX(31:0)" />
            <blockpin signalname="SPI_MISO" name="MISO" />
            <blockpin signalname="XLXN_90" name="END_TICK" />
            <blockpin signalname="data_rx(31:0)" name="DATA_RX(31:0)" />
        </block>
        <block symbolname="constant" name="XLXI_29">
            <attr value="21082026" name="CValue">
                <trait delete="all:1 sym:0" />
                <trait editname="all:1 sch:0" />
                <trait valuetype="BitVector 32 Hexadecimal" />
            </attr>
            <blockpin signalname="XLXN_105(31:0)" name="O" />
        </block>
        <block symbolname="demo_dy1_new" name="XLXI_8">
            <blockpin signalname="SYSCLK" name="SYSCLK50" />
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
        <block symbolname="pb_outport" name="XLXI_46">
            <blockpin signalname="SYSCLK" name="SYSCLK" />
            <blockpin signalname="XLXN_170" name="PORT_WR" />
            <blockpin signalname="XLXN_169(7:0)" name="PORT_ID(7:0)" />
            <blockpin signalname="XLXN_187(7:0)" name="PORT_DATA(7:0)" />
            <blockpin signalname="port_0(7:0)" name="PORT_0(7:0)" />
            <blockpin signalname="port_1(7:0)" name="PORT_1(7:0)" />
        </block>
        <block symbolname="pb_inport" name="XLXI_47">
            <blockpin signalname="SYSCLK" name="SYSCLK" />
            <blockpin signalname="XLXN_176" name="PORT_RD" />
            <blockpin signalname="XLXN_169(7:0)" name="PORT_ID(7:0)" />
            <blockpin signalname="XLXN_185(7:0)" name="PORT_0(7:0)" />
            <blockpin signalname="XLXN_186(7:0)" name="PORT_1(7:0)" />
            <blockpin signalname="XLXN_174(7:0)" name="PORT_DATA(7:0)" />
        </block>
        <block symbolname="kcpsm6" name="XLXI_48">
            <blockpin signalname="XLXN_166(17:0)" name="instruction(17:0)" />
            <blockpin signalname="XLXN_174(7:0)" name="in_port(7:0)" />
            <blockpin name="bram_enable" />
            <blockpin signalname="XLXN_170" name="write_strobe" />
            <blockpin name="k_write_strobe" />
            <blockpin signalname="XLXN_176" name="read_strobe" />
            <blockpin name="interrupt_ack" />
            <blockpin signalname="XLXN_167(11:0)" name="address(11:0)" />
            <blockpin signalname="XLXN_187(7:0)" name="out_port(7:0)" />
            <blockpin signalname="XLXN_169(7:0)" name="port_id(7:0)" />
            <blockpin signalname="SYSCLK" name="clk" />
            <blockpin signalname="XLXN_189" name="reset" />
            <blockpin name="interrupt" />
            <blockpin name="sleep" />
        </block>
        <block symbolname="buf" name="XLXI_49(7:0)">
            <blockpin signalname="port_0(7:0)" name="I" />
            <blockpin signalname="LED(7:0)" name="O" />
        </block>
        <block symbolname="bram_wrapper_pb6" name="XLXI_52">
            <blockpin signalname="SYSCLK" name="SYSCLK" />
            <blockpin signalname="XLXN_90" name="END_TICK" />
            <blockpin signalname="XLXN_105(31:0)" name="VERSION(31:0)" />
            <blockpin signalname="data_rx(31:0)" name="SPI_RX(31:0)" />
            <blockpin signalname="XLXN_167(11:0)" name="PB_ADDR(11:0)" />
            <blockpin signalname="XLXN_166(17:0)" name="PB_DATA(17:0)" />
            <blockpin signalname="XLXN_189" name="RESET" />
            <blockpin signalname="XLXN_102(31:0)" name="SPI_TX(31:0)" />
            <blockpin name="CFG_PORT(3:0)" />
            <blockpin signalname="XLXN_185(7:0)" name="OUTPORT_0(7:0)" />
            <blockpin signalname="XLXN_186(7:0)" name="OUTPORT_1(7:0)" />
            <blockpin name="OUTPORT_2(7:0)" />
            <blockpin name="OUTPORT_3(7:0)" />
        </block>
    </netlist>
    <sheet sheetnum="1" width="5440" height="3520">
        <branch name="data_rx(31:0)">
            <attrtext style="alignment:SOFT-BCENTER;fontsize:28;fontname:Arial" attrname="Name" x="1552" y="688" type="branch" />
            <wire x2="1552" y1="688" y2="688" x1="1392" />
            <wire x2="1760" y1="688" y2="688" x1="1552" />
        </branch>
        <branch name="XLXN_90">
            <wire x2="1760" y1="624" y2="624" x1="1392" />
        </branch>
        <branch name="SPI_MOSI">
            <wire x2="912" y1="624" y2="624" x1="656" />
        </branch>
        <branch name="SPI_SS">
            <wire x2="912" y1="560" y2="560" x1="656" />
        </branch>
        <branch name="SPI_SCK">
            <wire x2="912" y1="496" y2="496" x1="656" />
        </branch>
        <branch name="SPI_MISO">
            <wire x2="1456" y1="432" y2="432" x1="1392" />
        </branch>
        <branch name="XLXN_102(31:0)">
            <wire x2="720" y1="320" y2="688" x1="720" />
            <wire x2="912" y1="688" y2="688" x1="720" />
            <wire x2="2432" y1="320" y2="320" x1="720" />
            <wire x2="2432" y1="320" y2="688" x1="2432" />
            <wire x2="2432" y1="688" y2="688" x1="2352" />
        </branch>
        <branch name="XLXN_105(31:0)">
            <wire x2="1632" y1="864" y2="864" x1="1568" />
            <wire x2="1632" y1="752" y2="864" x1="1632" />
            <wire x2="1760" y1="752" y2="752" x1="1632" />
        </branch>
        <branch name="DY1_RCLK">
            <wire x2="3136" y1="1824" y2="1824" x1="2992" />
        </branch>
        <branch name="DY1_SCLK">
            <wire x2="3136" y1="1888" y2="1888" x1="2992" />
        </branch>
        <branch name="DY1_SER">
            <wire x2="3136" y1="1952" y2="1952" x1="2992" />
        </branch>
        <branch name="port_0(7:4)">
            <attrtext style="alignment:SOFT-RIGHT;fontsize:28;fontname:Arial" attrname="Name" x="2432" y="1888" type="branch" />
            <wire x2="2544" y1="1888" y2="1888" x1="2432" />
        </branch>
        <branch name="port_0(3:0)">
            <attrtext style="alignment:SOFT-RIGHT;fontsize:28;fontname:Arial" attrname="Name" x="2432" y="1952" type="branch" />
            <wire x2="2544" y1="1952" y2="1952" x1="2432" />
        </branch>
        <branch name="port_1(3:0)">
            <attrtext style="alignment:SOFT-RIGHT;fontsize:28;fontname:Arial" attrname="Name" x="2432" y="1824" type="branch" />
            <wire x2="2544" y1="1824" y2="1824" x1="2432" />
        </branch>
        <instance x="2544" y="1984" name="XLXI_8" orien="R0">
        </instance>
        <instance x="912" y="720" name="XLXI_1" orien="R0">
        </instance>
        <iomarker fontsize="28" x="1456" y="432" name="SPI_MISO" orien="R0" />
        <iomarker fontsize="28" x="3136" y="1824" name="DY1_RCLK" orien="R0" />
        <iomarker fontsize="28" x="3136" y="1888" name="DY1_SCLK" orien="R0" />
        <iomarker fontsize="28" x="3136" y="1952" name="DY1_SER" orien="R0" />
        <branch name="XLXN_166(17:0)">
            <wire x2="2576" y1="960" y2="960" x1="2352" />
        </branch>
        <branch name="port_0(7:0)">
            <attrtext style="alignment:SOFT-LEFT;fontsize:28;fontname:Arial" attrname="Name" x="3840" y="832" type="branch" />
            <wire x2="3776" y1="832" y2="832" x1="3728" />
            <wire x2="3840" y1="832" y2="832" x1="3776" />
            <wire x2="3808" y1="688" y2="688" x1="3776" />
            <wire x2="3776" y1="688" y2="832" x1="3776" />
        </branch>
        <iomarker fontsize="28" x="656" y="496" name="SPI_SCK" orien="R180" />
        <iomarker fontsize="28" x="656" y="560" name="SPI_SS" orien="R180" />
        <iomarker fontsize="28" x="656" y="624" name="SPI_MOSI" orien="R180" />
        <iomarker fontsize="28" x="640" y="128" name="SYSCLK" orien="R180" />
        <branch name="XLXN_174(7:0)">
            <wire x2="2528" y1="1040" y2="1184" x1="2528" />
            <wire x2="3808" y1="1184" y2="1184" x1="2528" />
            <wire x2="3808" y1="1184" y2="1376" x1="3808" />
            <wire x2="2576" y1="1040" y2="1040" x1="2528" />
            <wire x2="3808" y1="1376" y2="1376" x1="3728" />
        </branch>
        <instance x="3264" y="1600" name="XLXI_47" orien="R0">
        </instance>
        <branch name="XLXN_176">
            <wire x2="3072" y1="832" y2="832" x1="3040" />
            <wire x2="3072" y1="832" y2="1376" x1="3072" />
            <wire x2="3264" y1="1376" y2="1376" x1="3072" />
        </branch>
        <instance x="1424" y="832" name="XLXI_29" orien="R0">
        </instance>
        <branch name="XLXN_185(7:0)">
            <wire x2="2480" y1="1024" y2="1024" x1="2352" />
            <wire x2="2480" y1="1024" y2="1504" x1="2480" />
            <wire x2="3264" y1="1504" y2="1504" x1="2480" />
        </branch>
        <branch name="XLXN_186(7:0)">
            <wire x2="2432" y1="1088" y2="1088" x1="2352" />
            <wire x2="2432" y1="1088" y2="1568" x1="2432" />
            <wire x2="3264" y1="1568" y2="1568" x1="2432" />
        </branch>
        <branch name="SYSCLK">
            <wire x2="880" y1="128" y2="128" x1="640" />
            <wire x2="880" y1="128" y2="432" x1="880" />
            <wire x2="912" y1="432" y2="432" x1="880" />
            <wire x2="880" y1="432" y2="1760" x1="880" />
            <wire x2="2544" y1="1760" y2="1760" x1="880" />
            <wire x2="1696" y1="128" y2="128" x1="880" />
            <wire x2="1696" y1="128" y2="560" x1="1696" />
            <wire x2="1760" y1="560" y2="560" x1="1696" />
            <wire x2="2512" y1="128" y2="128" x1="1696" />
            <wire x2="3200" y1="128" y2="128" x1="2512" />
            <wire x2="3200" y1="128" y2="832" x1="3200" />
            <wire x2="3264" y1="832" y2="832" x1="3200" />
            <wire x2="3200" y1="832" y2="1312" x1="3200" />
            <wire x2="3264" y1="1312" y2="1312" x1="3200" />
            <wire x2="2512" y1="128" y2="640" x1="2512" />
            <wire x2="2576" y1="640" y2="640" x1="2512" />
        </branch>
        <instance x="2576" y="1120" name="XLXI_48" orien="R0">
        </instance>
        <branch name="XLXN_169(7:0)">
            <wire x2="3136" y1="1088" y2="1088" x1="3040" />
            <wire x2="3136" y1="1088" y2="1440" x1="3136" />
            <wire x2="3264" y1="1440" y2="1440" x1="3136" />
            <wire x2="3264" y1="960" y2="960" x1="3136" />
            <wire x2="3136" y1="960" y2="1088" x1="3136" />
        </branch>
        <branch name="XLXN_167(11:0)">
            <wire x2="1760" y1="960" y2="960" x1="1728" />
            <wire x2="1728" y1="960" y2="1312" x1="1728" />
            <wire x2="3104" y1="1312" y2="1312" x1="1728" />
            <wire x2="3104" y1="960" y2="960" x1="3040" />
            <wire x2="3104" y1="960" y2="1312" x1="3104" />
        </branch>
        <branch name="XLXN_187(7:0)">
            <wire x2="3264" y1="1024" y2="1024" x1="3040" />
        </branch>
        <branch name="XLXN_189">
            <wire x2="2576" y1="848" y2="848" x1="2352" />
        </branch>
        <instance x="3808" y="720" name="XLXI_49(7:0)" orien="R0" />
        <branch name="LED(7:0)">
            <wire x2="4064" y1="688" y2="688" x1="4032" />
        </branch>
        <iomarker fontsize="28" x="4064" y="688" name="LED(7:0)" orien="R0" />
        <instance x="3264" y="1056" name="XLXI_46" orien="R0">
        </instance>
        <branch name="XLXN_170">
            <wire x2="3136" y1="704" y2="704" x1="3040" />
            <wire x2="3136" y1="704" y2="896" x1="3136" />
            <wire x2="3264" y1="896" y2="896" x1="3136" />
        </branch>
        <branch name="port_1(7:0)">
            <attrtext style="alignment:SOFT-LEFT;fontsize:28;fontname:Arial" attrname="Name" x="3840" y="896" type="branch" />
            <wire x2="3840" y1="896" y2="896" x1="3728" />
        </branch>
        <instance x="1760" y="976" name="XLXI_52" orien="R0">
        </instance>
    </sheet>
</drawing>