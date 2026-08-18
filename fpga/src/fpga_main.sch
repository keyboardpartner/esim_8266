<?xml version="1.0" encoding="UTF-8"?>
<drawing version="7">
    <attr value="spartan6" name="DeviceFamilyName">
        <trait delete="all:0" />
        <trait editname="all:0" />
        <trait edittrait="all:0" />
    </attr>
    <netlist>
        <signal name="SYSCLK" />
        <signal name="D(7:0)" />
        <signal name="data_rx(31:0)" />
        <signal name="XLXN_90" />
        <signal name="SPI_MOSI" />
        <signal name="SPI_SS" />
        <signal name="SPI_SCK" />
        <signal name="SPI_MISO" />
        <signal name="XLXN_102(31:0)" />
        <signal name="XLXN_105(31:0)" />
        <signal name="XLXN_107" />
        <signal name="XLXN_108(3:0)" />
        <signal name="XLXN_109(3:0)" />
        <signal name="XLXN_110(3:0)" />
        <signal name="XLXN_111" />
        <signal name="DY1_RCLK" />
        <signal name="DY1_SCLK" />
        <signal name="DY1_SER" />
        <signal name="addr_intern(11:8)" />
        <signal name="addr_intern(7:4)" />
        <signal name="addr_intern(15:12)" />
        <signal name="XLXN_119" />
        <signal name="XLXN_120" />
        <signal name="XLXN_121" />
        <signal name="XLXN_122" />
        <signal name="addr_intern(15:0)" />
        <signal name="PIN_1" />
        <signal name="PIN_2" />
        <signal name="PIN_20" />
        <signal name="PIN_22" />
        <signal name="PIN_23" />
        <signal name="PIN_26" />
        <signal name="PIN_27" />
        <signal name="A(10:0)" />
        <signal name="type_sel(3:0)">
        </signal>
        <signal name="LED(7)" />
        <signal name="LED(3:0)" />
        <signal name="LED(6:4)" />
        <signal name="LED(7:0)" />
        <port polarity="Input" name="SYSCLK" />
        <port polarity="BiDirectional" name="D(7:0)" />
        <port polarity="Input" name="SPI_MOSI" />
        <port polarity="Input" name="SPI_SS" />
        <port polarity="Input" name="SPI_SCK" />
        <port polarity="Output" name="SPI_MISO" />
        <port polarity="Output" name="DY1_RCLK" />
        <port polarity="Output" name="DY1_SCLK" />
        <port polarity="Output" name="DY1_SER" />
        <port polarity="Input" name="PIN_1" />
        <port polarity="Input" name="PIN_2" />
        <port polarity="Input" name="PIN_20" />
        <port polarity="Input" name="PIN_22" />
        <port polarity="Input" name="PIN_23" />
        <port polarity="Input" name="PIN_26" />
        <port polarity="Input" name="PIN_27" />
        <port polarity="Input" name="A(10:0)" />
        <port polarity="Output" name="LED(7)" />
        <port polarity="Output" name="LED(3:0)" />
        <port polarity="Output" name="LED(6:4)" />
        <blockdef name="fast_spi32">
            <timestamp>2026-8-4T7:25:56</timestamp>
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
        <blockdef name="bram_wrapper">
            <timestamp>2026-8-14T16:15:19</timestamp>
            <rect width="64" x="528" y="148" height="24" />
            <line x2="592" y1="160" y2="160" x1="528" />
            <line x2="0" y1="-416" y2="-416" x1="64" />
            <line x2="0" y1="-352" y2="-352" x1="64" />
            <rect width="64" x="528" y="-172" height="24" />
            <line x2="592" y1="-160" y2="-160" x1="528" />
            <rect width="64" x="0" y="-300" height="24" />
            <line x2="0" y1="-288" y2="-288" x1="64" />
            <rect width="64" x="528" y="-300" height="24" />
            <line x2="592" y1="-288" y2="-288" x1="528" />
            <rect width="64" x="0" y="-236" height="24" />
            <line x2="0" y1="-224" y2="-224" x1="64" />
            <rect width="64" x="0" y="84" height="24" />
            <line x2="0" y1="96" y2="96" x1="64" />
            <line x2="0" y1="-96" y2="-96" x1="64" />
            <line x2="0" y1="-32" y2="-32" x1="64" />
            <line x2="0" y1="32" y2="32" x1="64" />
            <rect width="464" x="64" y="-448" height="640" />
        </blockdef>
        <blockdef name="constant">
            <timestamp>2006-1-1T10:10:10</timestamp>
            <rect width="112" x="0" y="0" height="64" />
            <line x2="112" y1="32" y2="32" x1="144" />
        </blockdef>
        <blockdef name="type_select">
            <timestamp>2026-8-14T16:8:42</timestamp>
            <rect width="368" x="64" y="-576" height="576" />
            <line x2="0" y1="-544" y2="-544" x1="64" />
            <line x2="0" y1="-480" y2="-480" x1="64" />
            <line x2="0" y1="-416" y2="-416" x1="64" />
            <line x2="0" y1="-352" y2="-352" x1="64" />
            <line x2="0" y1="-288" y2="-288" x1="64" />
            <line x2="0" y1="-224" y2="-224" x1="64" />
            <line x2="0" y1="-160" y2="-160" x1="64" />
            <rect width="64" x="0" y="-108" height="24" />
            <line x2="0" y1="-96" y2="-96" x1="64" />
            <rect width="64" x="0" y="-44" height="24" />
            <line x2="0" y1="-32" y2="-32" x1="64" />
            <line x2="496" y1="-544" y2="-544" x1="432" />
            <line x2="496" y1="-480" y2="-480" x1="432" />
            <line x2="496" y1="-416" y2="-416" x1="432" />
            <rect width="64" x="432" y="-364" height="24" />
            <line x2="496" y1="-352" y2="-352" x1="432" />
        </blockdef>
        <blockdef name="buf">
            <timestamp>2000-1-1T10:10:10</timestamp>
            <line x2="64" y1="-32" y2="-32" x1="0" />
            <line x2="128" y1="-32" y2="-32" x1="224" />
            <line x2="128" y1="0" y2="-32" x1="64" />
            <line x2="64" y1="-32" y2="-64" x1="128" />
            <line x2="64" y1="-64" y2="0" x1="64" />
        </blockdef>
        <blockdef name="gnd">
            <timestamp>2000-1-1T10:10:10</timestamp>
            <line x2="64" y1="-64" y2="-96" x1="64" />
            <line x2="52" y1="-48" y2="-48" x1="76" />
            <line x2="60" y1="-32" y2="-32" x1="68" />
            <line x2="40" y1="-64" y2="-64" x1="88" />
            <line x2="64" y1="-64" y2="-80" x1="64" />
            <line x2="64" y1="-128" y2="-96" x1="64" />
        </blockdef>
        <block symbolname="fast_spi32" name="XLXI_1">
            <blockpin signalname="SYSCLK" name="SYSCLK" />
            <blockpin signalname="SPI_SCK" name="SCK" />
            <blockpin signalname="SPI_SS" name="SS" />
            <blockpin signalname="SPI_MOSI" name="MOSI" />
            <blockpin signalname="XLXN_102(31:0)" name="DATA_TX(31:0)" />
            <blockpin signalname="SPI_MISO" name="MISO" />
            <blockpin signalname="data_rx(31:0)" name="DATA_RX(31:0)" />
            <blockpin signalname="XLXN_90" name="END_TICK" />
        </block>
        <block symbolname="bram_wrapper" name="XLXI_24">
            <blockpin signalname="SYSCLK" name="SYSCLK" />
            <blockpin signalname="XLXN_90" name="END_TICK" />
            <blockpin signalname="XLXN_120" name="RAM_CE" />
            <blockpin signalname="XLXN_121" name="RAM_OE" />
            <blockpin signalname="XLXN_122" name="RAM_WR" />
            <blockpin signalname="XLXN_105(31:0)" name="VERSION(31:0)" />
            <blockpin signalname="data_rx(31:0)" name="SPI_RX(31:0)" />
            <blockpin signalname="addr_intern(15:0)" name="RAM_ADDR(15:0)" />
            <blockpin signalname="D(7:0)" name="RAM_DATA(7:0)" />
            <blockpin signalname="XLXN_102(31:0)" name="SPI_TX(31:0)" />
            <blockpin signalname="type_sel(3:0)" name="TYPE_SEL(3:0)" />
        </block>
        <block symbolname="constant" name="XLXI_29">
            <attr value="15082026" name="CValue">
                <trait delete="all:1 sym:0" />
                <trait editname="all:1 sch:0" />
                <trait valuetype="BitVector 32 Hexadecimal" />
            </attr>
            <blockpin signalname="XLXN_105(31:0)" name="O" />
        </block>
        <block symbolname="type_select" name="XLXI_31">
            <blockpin signalname="PIN_1" name="PIN_1" />
            <blockpin signalname="PIN_2" name="PIN_2" />
            <blockpin signalname="PIN_20" name="PIN_20" />
            <blockpin signalname="PIN_22" name="PIN_22" />
            <blockpin signalname="PIN_23" name="PIN_23" />
            <blockpin signalname="PIN_26" name="PIN_26" />
            <blockpin signalname="PIN_27" name="PIN_27" />
            <blockpin signalname="type_sel(3:0)" name="TYPE_SEL(3:0)" />
            <blockpin signalname="A(10:0)" name="ADDR_10(10:0)" />
            <blockpin signalname="XLXN_120" name="RAM_CE" />
            <blockpin signalname="XLXN_121" name="RAM_OE" />
            <blockpin signalname="XLXN_122" name="RAM_WR" />
            <blockpin signalname="addr_intern(15:0)" name="ADDR_OUT(15:0)" />
        </block>
        <block symbolname="demo_dy1_new" name="XLXI_8">
            <blockpin signalname="SYSCLK" name="SYSCLK50" />
            <blockpin signalname="LED(7)" name="LED_BLINK" />
            <blockpin name="TICK_12HZ" />
            <blockpin signalname="DY1_RCLK" name="RCLK" />
            <blockpin signalname="DY1_SCLK" name="SCLK" />
            <blockpin signalname="DY1_SER" name="SER" />
            <blockpin name="BLINK_COUNT(7:0)" />
            <blockpin signalname="addr_intern(15:12)" name="DIGIT_L(3:0)" />
            <blockpin signalname="addr_intern(11:8)" name="DIGIT_M(3:0)" />
            <blockpin signalname="addr_intern(7:4)" name="DIGIT_R(3:0)" />
        </block>
        <block symbolname="buf" name="XLXI_35(3:0)">
            <blockpin signalname="type_sel(3:0)" name="I" />
            <blockpin signalname="LED(3:0)" name="O" />
        </block>
        <block symbolname="gnd" name="XLXI_37(2:0)">
            <blockpin signalname="LED(6:4)" name="G" />
        </block>
    </netlist>
    <sheet sheetnum="1" width="3520" height="2720">
        <instance x="672" y="720" name="XLXI_1" orien="R0">
        </instance>
        <branch name="SYSCLK">
            <wire x2="528" y1="208" y2="208" x1="304" />
            <wire x2="528" y1="208" y2="432" x1="528" />
            <wire x2="672" y1="432" y2="432" x1="528" />
            <wire x2="528" y1="432" y2="1760" x1="528" />
            <wire x2="2544" y1="1760" y2="1760" x1="528" />
            <wire x2="1744" y1="208" y2="208" x1="528" />
            <wire x2="1744" y1="208" y2="560" x1="1744" />
            <wire x2="1760" y1="560" y2="560" x1="1744" />
        </branch>
        <branch name="D(7:0)">
            <wire x2="2464" y1="816" y2="816" x1="2352" />
        </branch>
        <branch name="data_rx(31:0)">
            <attrtext style="alignment:SOFT-BCENTER;fontsize:28;fontname:Arial" attrname="Name" x="1280" y="688" type="branch" />
            <wire x2="1280" y1="688" y2="688" x1="1152" />
            <wire x2="1760" y1="688" y2="688" x1="1280" />
        </branch>
        <instance x="1760" y="976" name="XLXI_24" orien="R0">
        </instance>
        <branch name="XLXN_90">
            <wire x2="1760" y1="624" y2="624" x1="1152" />
        </branch>
        <branch name="SPI_MOSI">
            <wire x2="672" y1="624" y2="624" x1="320" />
        </branch>
        <branch name="SPI_SS">
            <wire x2="672" y1="560" y2="560" x1="320" />
        </branch>
        <branch name="SPI_SCK">
            <wire x2="672" y1="496" y2="496" x1="320" />
        </branch>
        <branch name="SPI_MISO">
            <wire x2="1200" y1="432" y2="432" x1="1152" />
        </branch>
        <iomarker fontsize="28" x="304" y="208" name="SYSCLK" orien="R180" />
        <iomarker fontsize="28" x="320" y="496" name="SPI_SCK" orien="R180" />
        <iomarker fontsize="28" x="320" y="560" name="SPI_SS" orien="R180" />
        <iomarker fontsize="28" x="320" y="624" name="SPI_MOSI" orien="R180" />
        <branch name="XLXN_102(31:0)">
            <wire x2="608" y1="320" y2="688" x1="608" />
            <wire x2="672" y1="688" y2="688" x1="608" />
            <wire x2="2416" y1="320" y2="320" x1="608" />
            <wire x2="2416" y1="320" y2="688" x1="2416" />
            <wire x2="2416" y1="688" y2="688" x1="2352" />
        </branch>
        <iomarker fontsize="28" x="2464" y="816" name="D(7:0)" orien="R0" />
        <iomarker fontsize="28" x="1200" y="432" name="SPI_MISO" orien="R0" />
        <branch name="XLXN_105(31:0)">
            <wire x2="1760" y1="752" y2="752" x1="1584" />
        </branch>
        <instance x="1440" y="720" name="XLXI_29" orien="R0">
        </instance>
        <branch name="DY1_RCLK">
            <wire x2="3216" y1="1824" y2="1824" x1="2992" />
        </branch>
        <branch name="DY1_SCLK">
            <wire x2="3216" y1="1888" y2="1888" x1="2992" />
        </branch>
        <branch name="DY1_SER">
            <wire x2="3216" y1="1952" y2="1952" x1="2992" />
        </branch>
        <branch name="addr_intern(11:8)">
            <attrtext style="alignment:SOFT-RIGHT;fontsize:28;fontname:Arial" attrname="Name" x="2432" y="1888" type="branch" />
            <wire x2="2544" y1="1888" y2="1888" x1="2432" />
        </branch>
        <branch name="addr_intern(7:4)">
            <attrtext style="alignment:SOFT-RIGHT;fontsize:28;fontname:Arial" attrname="Name" x="2432" y="1952" type="branch" />
            <wire x2="2544" y1="1952" y2="1952" x1="2432" />
        </branch>
        <branch name="addr_intern(15:12)">
            <attrtext style="alignment:SOFT-RIGHT;fontsize:28;fontname:Arial" attrname="Name" x="2432" y="1824" type="branch" />
            <wire x2="2544" y1="1824" y2="1824" x1="2432" />
        </branch>
        <instance x="2544" y="1984" name="XLXI_8" orien="R0">
        </instance>
        <iomarker fontsize="28" x="3216" y="1824" name="DY1_RCLK" orien="R0" />
        <iomarker fontsize="28" x="3216" y="1888" name="DY1_SCLK" orien="R0" />
        <iomarker fontsize="28" x="3216" y="1952" name="DY1_SER" orien="R0" />
        <branch name="XLXN_120">
            <wire x2="1760" y1="880" y2="880" x1="1584" />
        </branch>
        <branch name="XLXN_121">
            <wire x2="1760" y1="944" y2="944" x1="1584" />
        </branch>
        <branch name="XLXN_122">
            <wire x2="1760" y1="1008" y2="1008" x1="1584" />
        </branch>
        <branch name="addr_intern(15:0)">
            <attrtext style="alignment:SOFT-LEFT;fontsize:28;fontname:Arial" attrname="Name" x="1952" y="1888" type="branch" />
            <wire x2="1648" y1="1072" y2="1072" x1="1584" />
            <wire x2="1760" y1="1072" y2="1072" x1="1648" />
            <wire x2="1648" y1="1072" y2="1888" x1="1648" />
            <wire x2="1952" y1="1888" y2="1888" x1="1648" />
        </branch>
        <instance x="1088" y="1424" name="XLXI_31" orien="R0">
        </instance>
        <branch name="PIN_1">
            <wire x2="1088" y1="880" y2="880" x1="896" />
        </branch>
        <branch name="PIN_2">
            <wire x2="1088" y1="944" y2="944" x1="896" />
        </branch>
        <branch name="PIN_20">
            <wire x2="1088" y1="1008" y2="1008" x1="896" />
        </branch>
        <branch name="PIN_22">
            <wire x2="1088" y1="1072" y2="1072" x1="896" />
        </branch>
        <branch name="PIN_23">
            <wire x2="1088" y1="1136" y2="1136" x1="896" />
        </branch>
        <branch name="PIN_26">
            <wire x2="1088" y1="1200" y2="1200" x1="896" />
        </branch>
        <branch name="PIN_27">
            <wire x2="1088" y1="1264" y2="1264" x1="896" />
        </branch>
        <branch name="A(10:0)">
            <wire x2="1088" y1="1392" y2="1392" x1="896" />
        </branch>
        <iomarker fontsize="28" x="896" y="880" name="PIN_1" orien="R180" />
        <iomarker fontsize="28" x="896" y="944" name="PIN_2" orien="R180" />
        <iomarker fontsize="28" x="896" y="1008" name="PIN_20" orien="R180" />
        <iomarker fontsize="28" x="896" y="1072" name="PIN_22" orien="R180" />
        <iomarker fontsize="28" x="896" y="1136" name="PIN_23" orien="R180" />
        <iomarker fontsize="28" x="896" y="1200" name="PIN_26" orien="R180" />
        <iomarker fontsize="28" x="896" y="1264" name="PIN_27" orien="R180" />
        <iomarker fontsize="28" x="896" y="1392" name="A(10:0)" orien="R180" />
        <branch name="LED(7)">
            <wire x2="3216" y1="2048" y2="2048" x1="2992" />
        </branch>
        <iomarker fontsize="28" x="3216" y="2048" name="LED(7)" orien="R0" />
        <branch name="type_sel(3:0)">
            <attrtext style="alignment:SOFT-BCENTER;fontsize:28;fontname:Arial" attrname="Name" x="2000" y="1472" type="branch" />
            <wire x2="1088" y1="1328" y2="1328" x1="1008" />
            <wire x2="1008" y1="1328" y2="1472" x1="1008" />
            <wire x2="2000" y1="1472" y2="1472" x1="1008" />
            <wire x2="2432" y1="1472" y2="1472" x1="2000" />
            <wire x2="2432" y1="1136" y2="1136" x1="2352" />
            <wire x2="2432" y1="1136" y2="1472" x1="2432" />
            <wire x2="2624" y1="1136" y2="1136" x1="2432" />
        </branch>
        <branch name="LED(3:0)">
            <wire x2="3216" y1="1136" y2="1136" x1="2848" />
        </branch>
        <instance x="2624" y="1168" name="XLXI_35(3:0)" orien="R0" />
        <iomarker fontsize="28" x="3216" y="1136" name="LED(3:0)" orien="R0" />
        <branch name="LED(6:4)">
            <wire x2="3024" y1="1296" y2="1296" x1="3008" />
            <wire x2="3216" y1="1296" y2="1296" x1="3024" />
            <wire x2="3008" y1="1296" y2="1360" x1="3008" />
        </branch>
        <iomarker fontsize="28" x="3216" y="1296" name="LED(6:4)" orien="R0" />
        <branch name="LED(7:0)">
            <attrtext style="alignment:SOFT-LEFT;fontsize:28;fontname:Arial" attrname="Name" x="2768" y="1296" type="branch" />
            <wire x2="2768" y1="1296" y2="1296" x1="2624" />
        </branch>
        <instance x="2944" y="1488" name="XLXI_37(2:0)" orien="R0" />
    </sheet>
</drawing>