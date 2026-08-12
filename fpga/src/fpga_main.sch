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
        <signal name="CE" />
        <signal name="OE" />
        <signal name="WE" />
        <signal name="XLXN_90" />
        <signal name="SPI_MOSI" />
        <signal name="SPI_SS" />
        <signal name="SPI_SCK" />
        <signal name="SPI_MISO" />
        <signal name="DY1_RCLK" />
        <signal name="DY1_SCLK" />
        <signal name="DY1_SER" />
        <signal name="LED(7:0)" />
        <signal name="A(11:8)" />
        <signal name="A(7:4)" />
        <signal name="XLXN_102(31:0)" />
        <signal name="A(15:12)" />
        <signal name="A(15:0)" />
        <signal name="XLXN_105(31:0)" />
        <port polarity="Input" name="SYSCLK" />
        <port polarity="BiDirectional" name="D(7:0)" />
        <port polarity="Input" name="CE" />
        <port polarity="Input" name="OE" />
        <port polarity="Input" name="WE" />
        <port polarity="Input" name="SPI_MOSI" />
        <port polarity="Input" name="SPI_SS" />
        <port polarity="Input" name="SPI_SCK" />
        <port polarity="Output" name="SPI_MISO" />
        <port polarity="Output" name="DY1_RCLK" />
        <port polarity="Output" name="DY1_SCLK" />
        <port polarity="Output" name="DY1_SER" />
        <port polarity="Output" name="LED(7:0)" />
        <port polarity="Input" name="A(15:0)" />
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
            <timestamp>2026-8-12T14:2:23</timestamp>
            <line x2="0" y1="-416" y2="-416" x1="64" />
            <line x2="0" y1="-352" y2="-352" x1="64" />
            <rect width="64" x="0" y="-44" height="24" />
            <line x2="0" y1="-32" y2="-32" x1="64" />
            <line x2="0" y1="-224" y2="-224" x1="64" />
            <line x2="0" y1="-160" y2="-160" x1="64" />
            <line x2="0" y1="-96" y2="-96" x1="64" />
            <rect width="64" x="528" y="-172" height="24" />
            <line x2="592" y1="-160" y2="-160" x1="528" />
            <rect width="64" x="0" y="-300" height="24" />
            <line x2="0" y1="-288" y2="-288" x1="64" />
            <rect width="64" x="528" y="-300" height="24" />
            <line x2="592" y1="-288" y2="-288" x1="528" />
            <rect width="464" x="64" y="-448" height="548" />
            <rect width="64" x="0" y="52" height="24" />
            <line x2="0" y1="64" y2="64" x1="64" />
        </blockdef>
        <blockdef name="pullup">
            <timestamp>2000-1-1T10:10:10</timestamp>
            <line x2="64" y1="-108" y2="-128" x1="64" />
            <line x2="64" y1="-104" y2="-108" x1="80" />
            <line x2="80" y1="-88" y2="-104" x1="48" />
            <line x2="48" y1="-72" y2="-88" x1="80" />
            <line x2="80" y1="-56" y2="-72" x1="48" />
            <line x2="48" y1="-48" y2="-56" x1="64" />
            <line x2="64" y1="-32" y2="-48" x1="64" />
            <line x2="64" y1="-56" y2="-48" x1="48" />
            <line x2="48" y1="-72" y2="-56" x1="80" />
            <line x2="80" y1="-88" y2="-72" x1="48" />
            <line x2="48" y1="-104" y2="-88" x1="80" />
            <line x2="80" y1="-108" y2="-104" x1="64" />
            <line x2="64" y1="0" y2="-32" x1="64" />
            <line x2="32" y1="-128" y2="-128" x1="96" />
        </blockdef>
        <blockdef name="constant">
            <timestamp>2006-1-1T10:10:10</timestamp>
            <rect width="112" x="0" y="0" height="64" />
            <line x2="112" y1="32" y2="32" x1="144" />
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
        <block symbolname="demo_dy1_new" name="XLXI_8">
            <blockpin signalname="SYSCLK" name="SYSCLK50" />
            <blockpin name="LED_BLINK" />
            <blockpin name="TICK_12HZ" />
            <blockpin signalname="DY1_RCLK" name="RCLK" />
            <blockpin signalname="DY1_SCLK" name="SCLK" />
            <blockpin signalname="DY1_SER" name="SER" />
            <blockpin signalname="LED(7:0)" name="BLINK_COUNT(7:0)" />
            <blockpin signalname="A(15:12)" name="DIGIT_L(3:0)" />
            <blockpin signalname="A(11:8)" name="DIGIT_M(3:0)" />
            <blockpin signalname="A(7:4)" name="DIGIT_R(3:0)" />
        </block>
        <block symbolname="bram_wrapper" name="XLXI_24">
            <blockpin signalname="SYSCLK" name="SYSCLK" />
            <blockpin signalname="XLXN_90" name="END_TICK" />
            <blockpin signalname="CE" name="RAM_CE" />
            <blockpin signalname="OE" name="RAM_OE" />
            <blockpin signalname="WE" name="RAM_WR" />
            <blockpin signalname="data_rx(31:0)" name="SPI_RX(31:0)" />
            <blockpin signalname="A(15:0)" name="RAM_ADDR(15:0)" />
            <blockpin signalname="D(7:0)" name="RAM_DATA(7:0)" />
            <blockpin signalname="XLXN_102(31:0)" name="SPI_TX(31:0)" />
            <blockpin signalname="XLXN_105(31:0)" name="VERSION(31:0)" />
        </block>
        <block symbolname="pullup" name="XLXI_26">
            <blockpin signalname="CE" name="O" />
        </block>
        <block symbolname="pullup" name="XLXI_27">
            <blockpin signalname="OE" name="O" />
        </block>
        <block symbolname="pullup" name="XLXI_28">
            <blockpin signalname="WE" name="O" />
        </block>
        <block symbolname="constant" name="XLXI_29">
            <attr value="27512" name="CValue">
                <trait delete="all:1 sym:0" />
                <trait editname="all:1 sch:0" />
                <trait valuetype="BitVector 32 Hexadecimal" />
            </attr>
            <blockpin signalname="XLXN_105(31:0)" name="O" />
        </block>
    </netlist>
    <sheet sheetnum="1" width="3520" height="2720">
        <instance x="672" y="720" name="XLXI_1" orien="R0">
        </instance>
        <branch name="SYSCLK">
            <wire x2="528" y1="208" y2="208" x1="304" />
            <wire x2="528" y1="208" y2="432" x1="528" />
            <wire x2="672" y1="432" y2="432" x1="528" />
            <wire x2="528" y1="432" y2="1360" x1="528" />
            <wire x2="2480" y1="1360" y2="1360" x1="528" />
            <wire x2="1744" y1="208" y2="208" x1="528" />
            <wire x2="1744" y1="208" y2="560" x1="1744" />
            <wire x2="1760" y1="560" y2="560" x1="1744" />
        </branch>
        <branch name="D(7:0)">
            <wire x2="2464" y1="816" y2="816" x1="2352" />
        </branch>
        <instance x="2480" y="1584" name="XLXI_8" orien="R0">
        </instance>
        <branch name="data_rx(31:0)">
            <attrtext style="alignment:SOFT-BCENTER;fontsize:28;fontname:Arial" attrname="Name" x="1280" y="688" type="branch" />
            <wire x2="1280" y1="688" y2="688" x1="1152" />
            <wire x2="1760" y1="688" y2="688" x1="1280" />
        </branch>
        <branch name="CE">
            <wire x2="1472" y1="752" y2="752" x1="1376" />
            <wire x2="1744" y1="752" y2="752" x1="1472" />
            <wire x2="1760" y1="752" y2="752" x1="1744" />
            <wire x2="1472" y1="592" y2="752" x1="1472" />
        </branch>
        <branch name="OE">
            <wire x2="1568" y1="816" y2="816" x1="1376" />
            <wire x2="1744" y1="816" y2="816" x1="1568" />
            <wire x2="1760" y1="816" y2="816" x1="1744" />
            <wire x2="1568" y1="592" y2="816" x1="1568" />
        </branch>
        <branch name="WE">
            <wire x2="1648" y1="880" y2="880" x1="1376" />
            <wire x2="1744" y1="880" y2="880" x1="1648" />
            <wire x2="1760" y1="880" y2="880" x1="1744" />
            <wire x2="1648" y1="592" y2="880" x1="1648" />
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
            <wire x2="1168" y1="432" y2="432" x1="1152" />
            <wire x2="1200" y1="432" y2="432" x1="1168" />
        </branch>
        <iomarker fontsize="28" x="304" y="208" name="SYSCLK" orien="R180" />
        <branch name="DY1_RCLK">
            <wire x2="3152" y1="1424" y2="1424" x1="2928" />
        </branch>
        <branch name="DY1_SCLK">
            <wire x2="3152" y1="1488" y2="1488" x1="2928" />
        </branch>
        <branch name="DY1_SER">
            <wire x2="3152" y1="1552" y2="1552" x1="2928" />
        </branch>
        <branch name="LED(7:0)">
            <wire x2="3152" y1="1776" y2="1776" x1="2928" />
        </branch>
        <iomarker fontsize="28" x="3152" y="1424" name="DY1_RCLK" orien="R0" />
        <iomarker fontsize="28" x="3152" y="1488" name="DY1_SCLK" orien="R0" />
        <iomarker fontsize="28" x="3152" y="1552" name="DY1_SER" orien="R0" />
        <iomarker fontsize="28" x="3152" y="1776" name="LED(7:0)" orien="R0" />
        <iomarker fontsize="28" x="320" y="496" name="SPI_SCK" orien="R180" />
        <iomarker fontsize="28" x="320" y="560" name="SPI_SS" orien="R180" />
        <iomarker fontsize="28" x="320" y="624" name="SPI_MOSI" orien="R180" />
        <branch name="A(11:8)">
            <attrtext style="alignment:SOFT-RIGHT;fontsize:28;fontname:Arial" attrname="Name" x="2368" y="1488" type="branch" />
            <wire x2="2480" y1="1488" y2="1488" x1="2368" />
        </branch>
        <branch name="A(7:4)">
            <attrtext style="alignment:SOFT-RIGHT;fontsize:28;fontname:Arial" attrname="Name" x="2368" y="1552" type="branch" />
            <wire x2="2480" y1="1552" y2="1552" x1="2368" />
        </branch>
        <branch name="XLXN_102(31:0)">
            <wire x2="608" y1="320" y2="688" x1="608" />
            <wire x2="672" y1="688" y2="688" x1="608" />
            <wire x2="2416" y1="320" y2="320" x1="608" />
            <wire x2="2416" y1="320" y2="688" x1="2416" />
            <wire x2="2416" y1="688" y2="688" x1="2352" />
        </branch>
        <branch name="A(15:12)">
            <attrtext style="alignment:SOFT-RIGHT;fontsize:28;fontname:Arial" attrname="Name" x="2368" y="1424" type="branch" />
            <wire x2="2480" y1="1424" y2="1424" x1="2368" />
        </branch>
        <branch name="A(15:0)">
            <wire x2="1744" y1="944" y2="944" x1="1376" />
            <wire x2="1760" y1="944" y2="944" x1="1744" />
        </branch>
        <iomarker fontsize="28" x="2464" y="816" name="D(7:0)" orien="R0" />
        <iomarker fontsize="28" x="1376" y="752" name="CE" orien="R180" />
        <iomarker fontsize="28" x="1376" y="816" name="OE" orien="R180" />
        <iomarker fontsize="28" x="1376" y="880" name="WE" orien="R180" />
        <iomarker fontsize="28" x="1376" y="944" name="A(15:0)" orien="R180" />
        <instance x="1408" y="592" name="XLXI_26" orien="R0" />
        <instance x="1504" y="592" name="XLXI_27" orien="R0" />
        <instance x="1584" y="592" name="XLXI_28" orien="R0" />
        <iomarker fontsize="28" x="1200" y="432" name="SPI_MISO" orien="R0" />
        <branch name="XLXN_105(31:0)">
            <wire x2="1760" y1="1040" y2="1040" x1="1392" />
        </branch>
        <instance x="1248" y="1008" name="XLXI_29" orien="R0">
        </instance>
    </sheet>
</drawing>