<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE eagle SYSTEM "eagle.dtd">
<eagle version="6.5.0">
<drawing>
<settings>
<setting alwaysvectorfont="no"/>
<setting verticaltext="up"/>
</settings>
<grid distance="0.1" unitdist="inch" unit="inch" style="lines" multiple="1" display="no" altdistance="0.01" altunitdist="inch" altunit="inch"/>
<layers>
<layer number="1" name="Top" color="4" fill="1" visible="no" active="no"/>
<layer number="16" name="Bottom" color="1" fill="1" visible="no" active="no"/>
<layer number="17" name="Pads" color="2" fill="1" visible="no" active="no"/>
<layer number="18" name="Vias" color="2" fill="1" visible="no" active="no"/>
<layer number="19" name="Unrouted" color="6" fill="1" visible="no" active="no"/>
<layer number="20" name="Dimension" color="15" fill="1" visible="no" active="no"/>
<layer number="21" name="tPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="22" name="bPlace" color="7" fill="1" visible="no" active="no"/>
<layer number="23" name="tOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="24" name="bOrigins" color="15" fill="1" visible="no" active="no"/>
<layer number="25" name="tNames" color="7" fill="1" visible="no" active="no"/>
<layer number="26" name="bNames" color="7" fill="1" visible="no" active="no"/>
<layer number="27" name="tValues" color="7" fill="1" visible="no" active="no"/>
<layer number="28" name="bValues" color="7" fill="1" visible="no" active="no"/>
<layer number="29" name="tStop" color="7" fill="3" visible="no" active="no"/>
<layer number="30" name="bStop" color="7" fill="6" visible="no" active="no"/>
<layer number="31" name="tCream" color="7" fill="4" visible="no" active="no"/>
<layer number="32" name="bCream" color="7" fill="5" visible="no" active="no"/>
<layer number="33" name="tFinish" color="6" fill="3" visible="no" active="no"/>
<layer number="34" name="bFinish" color="6" fill="6" visible="no" active="no"/>
<layer number="35" name="tGlue" color="7" fill="4" visible="no" active="no"/>
<layer number="36" name="bGlue" color="7" fill="5" visible="no" active="no"/>
<layer number="37" name="tTest" color="7" fill="1" visible="no" active="no"/>
<layer number="38" name="bTest" color="7" fill="1" visible="no" active="no"/>
<layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="no"/>
<layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="no"/>
<layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="no"/>
<layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="no"/>
<layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="no"/>
<layer number="44" name="Drills" color="7" fill="1" visible="no" active="no"/>
<layer number="45" name="Holes" color="7" fill="1" visible="no" active="no"/>
<layer number="46" name="Milling" color="3" fill="1" visible="no" active="no"/>
<layer number="47" name="Measures" color="7" fill="1" visible="no" active="no"/>
<layer number="48" name="Document" color="7" fill="1" visible="no" active="no"/>
<layer number="49" name="Reference" color="7" fill="1" visible="no" active="no"/>
<layer number="50" name="dxf" color="7" fill="1" visible="no" active="no"/>
<layer number="51" name="tDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="52" name="bDocu" color="7" fill="1" visible="no" active="no"/>
<layer number="56" name="wert" color="7" fill="1" visible="no" active="no"/>
<layer number="91" name="Nets" color="2" fill="1" visible="yes" active="yes"/>
<layer number="92" name="Busses" color="1" fill="1" visible="yes" active="yes"/>
<layer number="93" name="Pins" color="2" fill="1" visible="yes" active="yes"/>
<layer number="94" name="Symbols" color="4" fill="1" visible="yes" active="yes"/>
<layer number="95" name="Names" color="7" fill="1" visible="yes" active="yes"/>
<layer number="96" name="Values" color="7" fill="1" visible="yes" active="yes"/>
<layer number="97" name="Info" color="7" fill="1" visible="yes" active="yes"/>
<layer number="98" name="Guide" color="6" fill="1" visible="yes" active="yes"/>
<layer number="100" name="Muster" color="7" fill="1" visible="no" active="no"/>
<layer number="101" name="Patch_Top" color="12" fill="4" visible="yes" active="yes"/>
<layer number="102" name="Mittellin" color="7" fill="1" visible="no" active="yes"/>
<layer number="104" name="Name" color="7" fill="1" visible="no" active="yes"/>
<layer number="105" name="Beschreib" color="9" fill="1" visible="no" active="no"/>
<layer number="106" name="BGA-Top" color="4" fill="1" visible="no" active="no"/>
<layer number="107" name="BD-Top" color="5" fill="1" visible="no" active="no"/>
<layer number="116" name="Patch_BOT" color="9" fill="4" visible="yes" active="yes"/>
<layer number="151" name="HeatSink" color="7" fill="1" visible="no" active="yes"/>
<layer number="200" name="200bmp" color="1" fill="10" visible="no" active="no"/>
<layer number="250" name="Descript" color="3" fill="1" visible="no" active="no"/>
<layer number="251" name="SMDround" color="12" fill="11" visible="no" active="no"/>
<layer number="254" name="cooling" color="7" fill="1" visible="yes" active="yes"/>
</layers>
<schematic xreflabel="%F%N/%S.%C%R" xrefpart="/%S.%C%R">
<libraries>
<library name="supply1">
<packages>
</packages>
<symbols>
<symbol name="GNDA">
<wire x1="-1.905" y1="0" x2="1.905" y2="0" width="0.254" layer="94"/>
<wire x1="-1.0922" y1="-0.508" x2="1.0922" y2="-0.508" width="0.254" layer="94"/>
<text x="-2.54" y="-2.54" size="1.778" layer="96" rot="R90">&gt;VALUE</text>
<pin name="GNDA" x="0" y="2.54" visible="off" length="short" direction="sup" rot="R270"/>
</symbol>
<symbol name="+12V">
<wire x1="1.27" y1="-1.905" x2="0" y2="0" width="0.254" layer="94"/>
<wire x1="0" y1="0" x2="-1.27" y2="-1.905" width="0.254" layer="94"/>
<wire x1="1.27" y1="-0.635" x2="0" y2="1.27" width="0.254" layer="94"/>
<wire x1="0" y1="1.27" x2="-1.27" y2="-0.635" width="0.254" layer="94"/>
<text x="-2.54" y="-5.08" size="1.778" layer="96" rot="R90">&gt;VALUE</text>
<pin name="+12V" x="0" y="-2.54" visible="off" length="short" direction="sup" rot="R90"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="GNDA" prefix="GND">
<description>&lt;b&gt;SUPPLY SYMBOL&lt;/b&gt;</description>
<gates>
<gate name="1" symbol="GNDA" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="+12V" prefix="P+">
<description>&lt;b&gt;SUPPLY SYMBOL&lt;/b&gt;</description>
<gates>
<gate name="1" symbol="+12V" x="0" y="0"/>
</gates>
<devices>
<device name="">
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="DEH">
<packages>
<package name="MOD8P8C">
<wire x1="-7.62" y1="-16.51" x2="-3.81" y2="-16.51" width="0.127" layer="21"/>
<wire x1="-3.81" y1="-16.51" x2="5.08" y2="-16.51" width="0.127" layer="21"/>
<wire x1="5.08" y1="-16.51" x2="8.89" y2="-16.51" width="0.127" layer="21"/>
<wire x1="-7.62" y1="-16.51" x2="-7.62" y2="2.54" width="0.127" layer="21"/>
<wire x1="-7.62" y1="2.54" x2="8.89" y2="2.54" width="0.127" layer="21"/>
<wire x1="8.89" y1="2.54" x2="8.89" y2="-16.51" width="0.127" layer="21"/>
<wire x1="-3.81" y1="-16.51" x2="-3.81" y2="-5.08" width="0.127" layer="21"/>
<wire x1="-3.81" y1="-5.08" x2="5.08" y2="-5.08" width="0.127" layer="21"/>
<wire x1="5.08" y1="-5.08" x2="5.08" y2="-16.51" width="0.127" layer="21"/>
<pad name="1" x="-3.81" y="-2.54" drill="1.0668" diameter="1.397"/>
<pad name="2" x="-2.54" y="0" drill="1.0668" diameter="1.397"/>
<pad name="3" x="-1.27" y="-2.54" drill="1.0668" diameter="1.397"/>
<pad name="4" x="0" y="0" drill="1.0668" diameter="1.397"/>
<pad name="5" x="1.27" y="-2.54" drill="1.0668" diameter="1.397"/>
<pad name="6" x="2.54" y="0" drill="1.0668" diameter="1.397"/>
<pad name="7" x="3.81" y="-2.54" drill="1.0668" diameter="1.397"/>
<pad name="8" x="5.08" y="0" drill="1.0668" diameter="1.397"/>
<hole x="-5.08" y="-8.89" drill="3.175"/>
<hole x="6.35" y="-8.89" drill="3.175"/>
</package>
</packages>
<symbols>
<symbol name="MOD8P8C">
<wire x1="-13.97" y1="-15.24" x2="-12.7" y2="-15.24" width="0.4064" layer="94"/>
<wire x1="-12.7" y1="-15.24" x2="6.35" y2="-15.24" width="0.4064" layer="94"/>
<wire x1="6.35" y1="-15.24" x2="6.35" y2="7.62" width="0.4064" layer="94"/>
<wire x1="6.35" y1="7.62" x2="-12.7" y2="7.62" width="0.4064" layer="94"/>
<wire x1="-12.7" y1="7.62" x2="-13.97" y2="7.62" width="0.4064" layer="94"/>
<wire x1="-13.97" y1="7.62" x2="-13.97" y2="-15.24" width="0.4064" layer="94"/>
<wire x1="-12.7" y1="7.62" x2="-12.7" y2="-15.24" width="0.254" layer="94"/>
<text x="-8.89" y="8.255" size="1.778" layer="95">&gt;NAME</text>
<text x="-8.89" y="-17.78" size="1.778" layer="96">&gt;VALUE</text>
<pin name="1" x="2.54" y="5.08" visible="pad" length="short" direction="pas" function="dot" rot="R180"/>
<pin name="2" x="2.54" y="0" visible="pad" length="short" direction="pas" function="dot" rot="R180"/>
<pin name="3" x="2.54" y="-5.08" visible="pad" length="short" direction="pas" function="dot" rot="R180"/>
<pin name="4" x="2.54" y="-10.16" visible="pad" length="short" direction="pas" function="dot" rot="R180"/>
<pin name="5" x="-5.08" y="2.54" visible="pad" length="short" direction="pas" function="dot" rot="R180"/>
<pin name="6" x="-5.08" y="-2.54" visible="pad" length="short" direction="pas" function="dot" rot="R180"/>
<pin name="7" x="-5.08" y="-7.62" visible="pad" length="short" direction="pas" function="dot" rot="R180"/>
<pin name="8" x="-5.08" y="-12.7" visible="pad" length="short" direction="pas" function="dot" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="MOD8P8C" prefix="JP" uservalue="yes">
<gates>
<gate name="G$1" symbol="MOD8P8C" x="5.08" y="12.7"/>
</gates>
<devices>
<device name="" package="MOD8P8C">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="2" pad="3"/>
<connect gate="G$1" pin="3" pad="5"/>
<connect gate="G$1" pin="4" pad="7"/>
<connect gate="G$1" pin="5" pad="2"/>
<connect gate="G$1" pin="6" pad="4"/>
<connect gate="G$1" pin="7" pad="6"/>
<connect gate="G$1" pin="8" pad="8"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="pinhead">
<packages>
<package name="2X05_KEYED">
<wire x1="-6.35" y1="-1.905" x2="-5.715" y2="-2.54" width="0.1524" layer="21"/>
<wire x1="-4.445" y1="-2.54" x2="-3.81" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="-3.81" y1="-1.905" x2="-3.175" y2="-2.54" width="0.1524" layer="21"/>
<wire x1="-1.905" y1="-2.54" x2="-1.27" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="-1.27" y1="-1.905" x2="-0.635" y2="-2.54" width="0.1524" layer="21"/>
<wire x1="0.635" y1="-2.54" x2="1.27" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="1.27" y1="-1.905" x2="1.905" y2="-2.54" width="0.1524" layer="21"/>
<wire x1="3.175" y1="-2.54" x2="3.81" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="-6.35" y1="-1.905" x2="-6.35" y2="1.905" width="0.1524" layer="21"/>
<wire x1="-6.35" y1="1.905" x2="-5.715" y2="2.54" width="0.1524" layer="21"/>
<wire x1="-5.715" y1="2.54" x2="-4.445" y2="2.54" width="0.1524" layer="21"/>
<wire x1="-4.445" y1="2.54" x2="-3.81" y2="1.905" width="0.1524" layer="21"/>
<wire x1="-3.81" y1="1.905" x2="-3.175" y2="2.54" width="0.1524" layer="21"/>
<wire x1="-3.175" y1="2.54" x2="-1.905" y2="2.54" width="0.1524" layer="21"/>
<wire x1="-1.905" y1="2.54" x2="-1.27" y2="1.905" width="0.1524" layer="21"/>
<wire x1="-1.27" y1="1.905" x2="-0.635" y2="2.54" width="0.1524" layer="21"/>
<wire x1="-0.635" y1="2.54" x2="0.635" y2="2.54" width="0.1524" layer="21"/>
<wire x1="0.635" y1="2.54" x2="1.27" y2="1.905" width="0.1524" layer="21"/>
<wire x1="1.27" y1="1.905" x2="1.905" y2="2.54" width="0.1524" layer="21"/>
<wire x1="1.905" y1="2.54" x2="3.175" y2="2.54" width="0.1524" layer="21"/>
<wire x1="3.175" y1="2.54" x2="3.81" y2="1.905" width="0.1524" layer="21"/>
<wire x1="-3.81" y1="1.905" x2="-3.81" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="-1.27" y1="1.905" x2="-1.27" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="1.27" y1="1.905" x2="1.27" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="3.81" y1="1.905" x2="3.81" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="1.905" y1="-2.54" x2="3.175" y2="-2.54" width="0.1524" layer="21"/>
<wire x1="-0.635" y1="-2.54" x2="0.635" y2="-2.54" width="0.1524" layer="21"/>
<wire x1="-3.175" y1="-2.54" x2="-1.905" y2="-2.54" width="0.1524" layer="21"/>
<wire x1="-5.715" y1="-2.54" x2="-4.445" y2="-2.54" width="0.1524" layer="21"/>
<wire x1="3.81" y1="-1.905" x2="4.445" y2="-2.54" width="0.1524" layer="21"/>
<wire x1="5.715" y1="-2.54" x2="6.35" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="3.81" y1="1.905" x2="4.445" y2="2.54" width="0.1524" layer="21"/>
<wire x1="4.445" y1="2.54" x2="5.715" y2="2.54" width="0.1524" layer="21"/>
<wire x1="5.715" y1="2.54" x2="6.35" y2="1.905" width="0.1524" layer="21"/>
<wire x1="6.35" y1="1.905" x2="6.35" y2="-1.905" width="0.1524" layer="21"/>
<wire x1="4.445" y1="-2.54" x2="5.715" y2="-2.54" width="0.1524" layer="21"/>
<wire x1="-10.2" y1="4.3" x2="10.2" y2="4.3" width="0.127" layer="21"/>
<wire x1="10.2" y1="4.3" x2="10.2" y2="-4.3" width="0.127" layer="21"/>
<wire x1="10.2" y1="-4.3" x2="2.2" y2="-4.3" width="0.127" layer="21"/>
<wire x1="-10.2" y1="4.3" x2="-10.2" y2="-4.3" width="0.127" layer="21"/>
<wire x1="-2.2" y1="-4.3" x2="-10.2" y2="-4.3" width="0.127" layer="21"/>
<wire x1="-2.2" y1="-4.3" x2="-2.2" y2="-4" width="0.127" layer="21"/>
<wire x1="-2.2" y1="-4" x2="2.2" y2="-4" width="0.127" layer="21"/>
<wire x1="2.2" y1="-4" x2="2.2" y2="-4.3" width="0.127" layer="21"/>
<wire x1="2.2" y1="-4.3" x2="-2.1" y2="-4.3" width="0.127" layer="21"/>
<pad name="1" x="-5.08" y="-1.27" drill="1.0668" shape="octagon"/>
<pad name="2" x="-5.08" y="1.27" drill="1.0668" shape="octagon"/>
<pad name="3" x="-2.54" y="-1.27" drill="1.0668" shape="octagon"/>
<pad name="4" x="-2.54" y="1.27" drill="1.0668" shape="octagon"/>
<pad name="5" x="0" y="-1.27" drill="1.0668" shape="octagon"/>
<pad name="6" x="0" y="1.27" drill="1.0668" shape="octagon"/>
<pad name="7" x="2.54" y="-1.27" drill="1.0668" shape="octagon"/>
<pad name="8" x="2.54" y="1.27" drill="1.0668" shape="octagon"/>
<pad name="9" x="5.08" y="-1.27" drill="1.0668" shape="octagon"/>
<pad name="10" x="5.08" y="1.27" drill="1.0668" shape="octagon"/>
<text x="-6.35" y="6.985" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="2.54" y="6.985" size="1.27" layer="27">&gt;VALUE</text>
<rectangle x1="-5.334" y1="-1.524" x2="-4.826" y2="-1.016" layer="51"/>
<rectangle x1="-5.334" y1="1.016" x2="-4.826" y2="1.524" layer="51"/>
<rectangle x1="-2.794" y1="1.016" x2="-2.286" y2="1.524" layer="51"/>
<rectangle x1="-2.794" y1="-1.524" x2="-2.286" y2="-1.016" layer="51"/>
<rectangle x1="-0.254" y1="1.016" x2="0.254" y2="1.524" layer="51"/>
<rectangle x1="-0.254" y1="-1.524" x2="0.254" y2="-1.016" layer="51"/>
<rectangle x1="2.286" y1="1.016" x2="2.794" y2="1.524" layer="51"/>
<rectangle x1="2.286" y1="-1.524" x2="2.794" y2="-1.016" layer="51"/>
<rectangle x1="4.826" y1="1.016" x2="5.334" y2="1.524" layer="51"/>
<rectangle x1="4.826" y1="-1.524" x2="5.334" y2="-1.016" layer="51"/>
</package>
</packages>
<symbols>
<symbol name="PINH2X5">
<wire x1="-6.35" y1="-7.62" x2="8.89" y2="-7.62" width="0.4064" layer="94"/>
<wire x1="8.89" y1="-7.62" x2="8.89" y2="7.62" width="0.4064" layer="94"/>
<wire x1="8.89" y1="7.62" x2="-6.35" y2="7.62" width="0.4064" layer="94"/>
<wire x1="-6.35" y1="7.62" x2="-6.35" y2="-7.62" width="0.4064" layer="94"/>
<text x="-6.35" y="8.255" size="1.778" layer="95">&gt;NAME</text>
<text x="-6.35" y="-10.16" size="1.778" layer="96">&gt;VALUE</text>
<pin name="1" x="-2.54" y="5.08" visible="pad" length="short" direction="pas" function="dot"/>
<pin name="2" x="5.08" y="5.08" visible="pad" length="short" direction="pas" function="dot" rot="R180"/>
<pin name="3" x="-2.54" y="2.54" visible="pad" length="short" direction="pas" function="dot"/>
<pin name="4" x="5.08" y="2.54" visible="pad" length="short" direction="pas" function="dot" rot="R180"/>
<pin name="5" x="-2.54" y="0" visible="pad" length="short" direction="pas" function="dot"/>
<pin name="6" x="5.08" y="0" visible="pad" length="short" direction="pas" function="dot" rot="R180"/>
<pin name="7" x="-2.54" y="-2.54" visible="pad" length="short" direction="pas" function="dot"/>
<pin name="8" x="5.08" y="-2.54" visible="pad" length="short" direction="pas" function="dot" rot="R180"/>
<pin name="9" x="-2.54" y="-5.08" visible="pad" length="short" direction="pas" function="dot"/>
<pin name="10" x="5.08" y="-5.08" visible="pad" length="short" direction="pas" function="dot" rot="R180"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="PINHD-2X5_KEYED">
<gates>
<gate name="G$1" symbol="PINH2X5" x="0" y="0"/>
</gates>
<devices>
<device name="" package="2X05_KEYED">
<connects>
<connect gate="G$1" pin="1" pad="1"/>
<connect gate="G$1" pin="10" pad="10"/>
<connect gate="G$1" pin="2" pad="2"/>
<connect gate="G$1" pin="3" pad="3"/>
<connect gate="G$1" pin="4" pad="4"/>
<connect gate="G$1" pin="5" pad="5"/>
<connect gate="G$1" pin="6" pad="6"/>
<connect gate="G$1" pin="7" pad="7"/>
<connect gate="G$1" pin="8" pad="8"/>
<connect gate="G$1" pin="9" pad="9"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
<library name="solpad">
<description>&lt;b&gt;Solder Pads/Test Points&lt;/b&gt;&lt;p&gt;
&lt;author&gt;Created by librarian@cadsoft.de&lt;/author&gt;</description>
<packages>
<package name="SE14">
<description>&lt;b&gt;SOLDER PAD&lt;/b&gt;&lt;p&gt;
drill 1.4 mm</description>
<wire x1="-1.524" y1="0.635" x2="-1.524" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="0.635" y1="-1.524" x2="1.524" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="0.635" y1="1.524" x2="1.524" y2="0.635" width="0.1524" layer="21"/>
<wire x1="1.524" y1="0.635" x2="1.524" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="-1.524" y1="0.635" x2="-0.635" y2="1.524" width="0.1524" layer="21"/>
<wire x1="-0.635" y1="1.524" x2="0.635" y2="1.524" width="0.1524" layer="21"/>
<wire x1="-0.635" y1="-1.524" x2="-1.524" y2="-0.635" width="0.1524" layer="21"/>
<wire x1="0.635" y1="-1.524" x2="-0.635" y2="-1.524" width="0.1524" layer="21"/>
<circle x="0" y="0" radius="0.762" width="0.1524" layer="51"/>
<circle x="0" y="0" radius="0.381" width="0.254" layer="51"/>
<pad name="MP" x="0" y="0" drill="1.397" diameter="2.54" shape="octagon"/>
<text x="-1.397" y="1.778" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="0" y="0.381" size="0.0254" layer="27">&gt;VALUE</text>
</package>
<package name="LSP10">
<description>&lt;b&gt;SOLDER PAD&lt;/b&gt;&lt;p&gt;
drill 1.0 mm</description>
<wire x1="-1.27" y1="0.254" x2="-1.27" y2="-0.254" width="0.1524" layer="21"/>
<wire x1="1.27" y1="0.254" x2="1.27" y2="-0.254" width="0.1524" layer="21"/>
<wire x1="1.27" y1="0.254" x2="1.143" y2="0.254" width="0.1524" layer="21"/>
<wire x1="1.27" y1="-0.254" x2="1.143" y2="-0.254" width="0.1524" layer="21"/>
<wire x1="-1.27" y1="-0.254" x2="-1.143" y2="-0.254" width="0.1524" layer="21"/>
<wire x1="-1.27" y1="0.254" x2="-1.143" y2="0.254" width="0.1524" layer="21"/>
<wire x1="1.143" y1="0.254" x2="0.635" y2="0.254" width="0.1524" layer="51"/>
<wire x1="-1.143" y1="-0.254" x2="-0.635" y2="-0.254" width="0.1524" layer="51"/>
<wire x1="0.635" y1="0.254" x2="0.635" y2="-0.254" width="0.1524" layer="51"/>
<wire x1="0.635" y1="0.254" x2="-0.635" y2="0.254" width="0.1524" layer="51"/>
<wire x1="0.635" y1="-0.254" x2="1.143" y2="-0.254" width="0.1524" layer="51"/>
<wire x1="-0.635" y1="0.254" x2="-0.635" y2="-0.254" width="0.1524" layer="51"/>
<wire x1="-0.635" y1="0.254" x2="-1.143" y2="0.254" width="0.1524" layer="51"/>
<wire x1="-0.635" y1="-0.254" x2="0.635" y2="-0.254" width="0.1524" layer="51"/>
<pad name="MP" x="0" y="0" drill="1.016" diameter="2.159" shape="octagon"/>
<text x="-1.27" y="1.27" size="1.27" layer="25" ratio="10">&gt;NAME</text>
<text x="0" y="0.254" size="0.0254" layer="27">&gt;VALUE</text>
</package>
</packages>
<symbols>
<symbol name="LSP">
<wire x1="-1.016" y1="2.032" x2="1.016" y2="0" width="0.254" layer="94"/>
<wire x1="-1.016" y1="0" x2="1.016" y2="2.032" width="0.254" layer="94"/>
<circle x="0" y="1.016" radius="1.016" width="0.4064" layer="94"/>
<text x="-1.27" y="2.921" size="1.778" layer="95">&gt;NAME</text>
<pin name="MP" x="0" y="-2.54" visible="off" length="short" direction="pas" rot="R90"/>
</symbol>
</symbols>
<devicesets>
<deviceset name="SE14" prefix="LSP">
<description>&lt;b&gt;SOLDER PAD&lt;/b&gt;&lt;p&gt; E1553,  drill 1,4mm, distributor Buerklin, 07F820</description>
<gates>
<gate name="1" symbol="LSP" x="0" y="0"/>
</gates>
<devices>
<device name="" package="SE14">
<connects>
<connect gate="1" pin="MP" pad="MP"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
<deviceset name="LSP10" prefix="LSP">
<description>&lt;b&gt;SOLDER PAD&lt;/b&gt;&lt;p&gt; drill 1.0 mm, distributor Buerklin, 12H555</description>
<gates>
<gate name="1" symbol="LSP" x="0" y="0"/>
</gates>
<devices>
<device name="" package="LSP10">
<connects>
<connect gate="1" pin="MP" pad="MP"/>
</connects>
<technologies>
<technology name=""/>
</technologies>
</device>
</devices>
</deviceset>
</devicesets>
</library>
</libraries>
<attributes>
</attributes>
<variantdefs>
</variantdefs>
<classes>
<class number="0" name="default" width="0" drill="0">
</class>
</classes>
<parts>
<part name="P+1" library="supply1" deviceset="+12V" device=""/>
<part name="JP6" library="pinhead" deviceset="PINHD-2X5_KEYED" device="" value="BMS ribbon"/>
<part name="GND10" library="supply1" deviceset="GNDA" device=""/>
<part name="JP7" library="DEH" deviceset="MOD8P8C" device=""/>
<part name="JP8" library="DEH" deviceset="MOD8P8C" device=""/>
<part name="LSP1" library="solpad" deviceset="SE14" device=""/>
<part name="LSP4" library="solpad" deviceset="SE14" device=""/>
<part name="LSP5" library="solpad" deviceset="LSP10" device=""/>
<part name="LSP6" library="solpad" deviceset="LSP10" device=""/>
<part name="LSP2" library="solpad" deviceset="LSP10" device=""/>
</parts>
<sheets>
<sheet>
<plain>
<text x="28.575" y="53.975" size="1.778" layer="91" rot="R180">CAN BUS PLUS</text>
<text x="47.625" y="89.535" size="1.778" layer="91" rot="R270">PASS-THRU</text>
<text x="43.18" y="48.895" size="1.778" layer="91" rot="R90">CAN BUS PLUS</text>
<text x="52.705" y="48.26" size="1.778" layer="91" rot="R90">CAN BUS MINUS</text>
<text x="66.675" y="49.53" size="1.778" layer="91" rot="R90">CAN-H</text>
<text x="60.325" y="50.165" size="1.778" layer="91" rot="R90">CAN-L</text>
<text x="76.2" y="127" size="3.81" layer="91">RJ45 CAN cable</text>
<text x="96.52" y="121.92" size="3.81" layer="91">to</text>
<text x="83.82" y="116.84" size="3.81" layer="91">BMS pcb 2x10 keyed header</text>
<text x="80.645" y="88.265" size="1.778" layer="91">Master Reset</text>
<text x="79.375" y="27.94" size="1.778" layer="91">CAN minus</text>
<text x="85.725" y="76.2" size="1.778" layer="91">CAN-L</text>
<text x="85.725" y="66.675" size="1.778" layer="91">CAN-H</text>
</plain>
<instances>
<instance part="P+1" gate="1" x="19.685" y="46.99" rot="R270"/>
<instance part="JP6" gate="G$1" x="29.21" y="32.385" rot="R180"/>
<instance part="GND10" gate="1" x="16.51" y="15.24"/>
<instance part="JP7" gate="G$1" x="31.75" y="73.66"/>
<instance part="JP8" gate="G$1" x="31.75" y="107.315"/>
<instance part="LSP1" gate="1" x="76.2" y="112.395" rot="R270"/>
<instance part="LSP4" gate="1" x="76.2" y="25.4" rot="R270"/>
<instance part="LSP5" gate="1" x="80.01" y="85.725" rot="R270"/>
<instance part="LSP6" gate="1" x="80.01" y="74.93" rot="R270"/>
<instance part="LSP2" gate="1" x="80.01" y="66.04" rot="R270"/>
</instances>
<busses>
</busses>
<nets>
<net name="GNDA" class="0">
<segment>
<wire x1="53.975" y1="60.96" x2="53.975" y2="94.615" width="0.1524" layer="91"/>
<wire x1="40.005" y1="60.96" x2="53.975" y2="60.96" width="0.1524" layer="91"/>
<wire x1="34.29" y1="63.5" x2="40.005" y2="63.5" width="0.1524" layer="91"/>
<wire x1="26.67" y1="66.04" x2="40.005" y2="66.04" width="0.1524" layer="91"/>
<wire x1="40.005" y1="66.04" x2="40.005" y2="63.5" width="0.1524" layer="91"/>
<wire x1="40.005" y1="63.5" x2="40.005" y2="60.96" width="0.1524" layer="91"/>
<wire x1="40.64" y1="94.615" x2="53.975" y2="94.615" width="0.1524" layer="91"/>
<wire x1="34.29" y1="97.155" x2="40.64" y2="97.155" width="0.1524" layer="91"/>
<wire x1="40.64" y1="97.155" x2="40.64" y2="94.615" width="0.1524" layer="91"/>
<wire x1="26.67" y1="99.695" x2="40.64" y2="99.695" width="0.1524" layer="91"/>
<wire x1="40.64" y1="99.695" x2="40.64" y2="97.155" width="0.1524" layer="91"/>
<junction x="40.005" y="63.5"/>
<junction x="40.64" y="97.155"/>
<pinref part="JP7" gate="G$1" pin="4"/>
<pinref part="JP7" gate="G$1" pin="7"/>
<pinref part="JP8" gate="G$1" pin="4"/>
<pinref part="JP8" gate="G$1" pin="7"/>
<wire x1="31.75" y1="37.465" x2="36.83" y2="37.465" width="0.1524" layer="91"/>
<wire x1="36.83" y1="37.465" x2="36.83" y2="32.385" width="0.1524" layer="91"/>
<wire x1="36.83" y1="32.385" x2="31.75" y2="32.385" width="0.1524" layer="91"/>
<wire x1="36.83" y1="32.385" x2="36.83" y2="25.4" width="0.1524" layer="91"/>
<wire x1="36.83" y1="25.4" x2="53.975" y2="25.4" width="0.1524" layer="91"/>
<wire x1="36.83" y1="25.4" x2="36.83" y2="23.495" width="0.1524" layer="91"/>
<wire x1="36.83" y1="23.495" x2="24.13" y2="23.495" width="0.1524" layer="91"/>
<wire x1="24.13" y1="23.495" x2="24.13" y2="27.305" width="0.1524" layer="91"/>
<wire x1="24.13" y1="23.495" x2="16.51" y2="23.495" width="0.1524" layer="91"/>
<wire x1="16.51" y1="23.495" x2="16.51" y2="17.78" width="0.1524" layer="91"/>
<junction x="36.83" y="32.385"/>
<junction x="36.83" y="25.4"/>
<junction x="24.13" y="23.495"/>
<pinref part="JP6" gate="G$1" pin="2"/>
<pinref part="JP6" gate="G$1" pin="5"/>
<pinref part="JP6" gate="G$1" pin="9"/>
<pinref part="GND10" gate="1" pin="GNDA"/>
<junction x="53.975" y="60.96"/>
<wire x1="53.975" y1="60.96" x2="53.975" y2="25.4" width="0.1524" layer="91"/>
<pinref part="LSP4" gate="1" pin="MP"/>
<wire x1="53.975" y1="25.4" x2="73.66" y2="25.4" width="0.1524" layer="91"/>
<junction x="53.975" y="25.4"/>
</segment>
</net>
<net name="+12V" class="0">
<segment>
<wire x1="31.75" y1="27.305" x2="31.75" y2="21.59" width="0.1524" layer="91"/>
<wire x1="17.145" y1="46.99" x2="17.145" y2="34.925" width="0.1524" layer="91"/>
<wire x1="17.145" y1="34.925" x2="19.05" y2="34.925" width="0.1524" layer="91"/>
<wire x1="19.05" y1="34.925" x2="24.13" y2="34.925" width="0.1524" layer="91"/>
<wire x1="31.75" y1="21.59" x2="19.05" y2="21.59" width="0.1524" layer="91"/>
<wire x1="19.05" y1="21.59" x2="19.05" y2="34.925" width="0.1524" layer="91"/>
<wire x1="31.75" y1="34.925" x2="24.13" y2="34.925" width="0.1524" layer="91"/>
<wire x1="31.75" y1="34.925" x2="44.45" y2="34.925" width="0.1524" layer="91"/>
<junction x="19.05" y="34.925"/>
<junction x="24.13" y="34.925"/>
<junction x="31.75" y="34.925"/>
<pinref part="JP6" gate="G$1" pin="1"/>
<pinref part="P+1" gate="1" pin="+12V"/>
<pinref part="JP6" gate="G$1" pin="8"/>
<pinref part="JP6" gate="G$1" pin="7"/>
<wire x1="26.67" y1="109.855" x2="40.64" y2="109.855" width="0.1524" layer="91"/>
<wire x1="34.29" y1="107.315" x2="40.64" y2="107.315" width="0.1524" layer="91"/>
<wire x1="40.64" y1="107.315" x2="40.64" y2="109.855" width="0.1524" layer="91"/>
<wire x1="40.64" y1="109.855" x2="40.64" y2="112.395" width="0.1524" layer="91"/>
<wire x1="40.64" y1="112.395" x2="44.45" y2="112.395" width="0.1524" layer="91"/>
<wire x1="44.45" y1="79.375" x2="44.45" y2="112.395" width="0.1524" layer="91"/>
<wire x1="44.45" y1="79.375" x2="40.64" y2="79.375" width="0.1524" layer="91"/>
<wire x1="34.29" y1="73.66" x2="40.64" y2="73.66" width="0.1524" layer="91"/>
<wire x1="40.64" y1="73.66" x2="40.64" y2="76.2" width="0.1524" layer="91"/>
<wire x1="40.64" y1="76.2" x2="40.64" y2="78.74" width="0.1524" layer="91"/>
<wire x1="40.64" y1="78.74" x2="40.64" y2="79.375" width="0.1524" layer="91"/>
<wire x1="26.67" y1="76.2" x2="40.64" y2="76.2" width="0.1524" layer="91"/>
<junction x="40.64" y="109.855"/>
<junction x="40.64" y="76.2"/>
<pinref part="JP8" gate="G$1" pin="5"/>
<pinref part="JP8" gate="G$1" pin="2"/>
<pinref part="JP7" gate="G$1" pin="2"/>
<pinref part="JP7" gate="G$1" pin="5"/>
<wire x1="44.45" y1="79.375" x2="44.45" y2="34.925" width="0.1524" layer="91"/>
<junction x="44.45" y="79.375"/>
<pinref part="JP8" gate="G$1" pin="1"/>
<wire x1="34.29" y1="112.395" x2="40.64" y2="112.395" width="0.1524" layer="91"/>
<junction x="40.64" y="112.395"/>
<pinref part="JP7" gate="G$1" pin="1"/>
<wire x1="34.29" y1="78.74" x2="40.64" y2="78.74" width="0.1524" layer="91"/>
<junction x="40.64" y="78.74"/>
<pinref part="LSP1" gate="1" pin="MP"/>
<wire x1="44.45" y1="112.395" x2="73.66" y2="112.395" width="0.1524" layer="91"/>
<junction x="44.45" y="112.395"/>
</segment>
</net>
<net name="CANL" class="0">
<segment>
<wire x1="34.29" y1="68.58" x2="61.595" y2="68.58" width="0.1524" layer="91"/>
<pinref part="JP7" gate="G$1" pin="3"/>
<wire x1="61.595" y1="68.58" x2="61.595" y2="29.845" width="0.1524" layer="91"/>
<wire x1="31.75" y1="29.845" x2="61.595" y2="29.845" width="0.1524" layer="91"/>
<pinref part="JP6" gate="G$1" pin="3"/>
<pinref part="JP8" gate="G$1" pin="3"/>
<wire x1="34.29" y1="102.235" x2="61.595" y2="102.235" width="0.1524" layer="91"/>
<wire x1="61.595" y1="68.58" x2="61.595" y2="74.93" width="0.1524" layer="91"/>
<junction x="61.595" y="68.58"/>
<pinref part="LSP6" gate="1" pin="MP"/>
<wire x1="61.595" y1="74.93" x2="61.595" y2="102.235" width="0.1524" layer="91"/>
<wire x1="77.47" y1="74.93" x2="61.595" y2="74.93" width="0.1524" layer="91"/>
<junction x="61.595" y="74.93"/>
</segment>
</net>
<net name="CANH" class="0">
<segment>
<wire x1="26.67" y1="71.12" x2="67.945" y2="71.12" width="0.1524" layer="91"/>
<pinref part="JP7" gate="G$1" pin="6"/>
<wire x1="27.94" y1="29.845" x2="27.94" y2="28.575" width="0.1524" layer="91"/>
<wire x1="27.94" y1="28.575" x2="67.945" y2="28.575" width="0.1524" layer="91"/>
<pinref part="JP6" gate="G$1" pin="4"/>
<wire x1="67.945" y1="71.12" x2="67.945" y2="66.04" width="0.1524" layer="91"/>
<wire x1="67.945" y1="66.04" x2="67.945" y2="28.575" width="0.1524" layer="91"/>
<wire x1="27.94" y1="29.845" x2="24.13" y2="29.845" width="0.1524" layer="91"/>
<pinref part="JP8" gate="G$1" pin="6"/>
<wire x1="26.67" y1="104.775" x2="67.945" y2="104.775" width="0.1524" layer="91"/>
<wire x1="67.945" y1="71.12" x2="67.945" y2="104.775" width="0.1524" layer="91"/>
<junction x="67.945" y="71.12"/>
<pinref part="LSP2" gate="1" pin="MP"/>
<wire x1="77.47" y1="66.04" x2="67.945" y2="66.04" width="0.1524" layer="91"/>
<junction x="67.945" y="66.04"/>
</segment>
</net>
<net name="MASTER-RESET-SYS" class="0">
<segment>
<pinref part="JP7" gate="G$1" pin="8"/>
<wire x1="24.13" y1="32.385" x2="8.255" y2="32.385" width="0.1524" layer="91"/>
<pinref part="JP6" gate="G$1" pin="6"/>
<wire x1="26.67" y1="60.96" x2="8.255" y2="60.96" width="0.1524" layer="91"/>
<wire x1="8.255" y1="60.96" x2="8.255" y2="32.385" width="0.1524" layer="91"/>
<wire x1="8.255" y1="60.96" x2="8.255" y2="89.535" width="0.1524" layer="91"/>
<junction x="8.255" y="60.96"/>
<pinref part="JP8" gate="G$1" pin="8"/>
<wire x1="8.255" y1="89.535" x2="8.255" y2="94.615" width="0.1524" layer="91"/>
<wire x1="8.255" y1="94.615" x2="26.67" y2="94.615" width="0.1524" layer="91"/>
<wire x1="24.13" y1="32.385" x2="24.13" y2="33.655" width="0.1524" layer="91"/>
<junction x="24.13" y="32.385"/>
<wire x1="37.465" y1="85.725" x2="37.465" y2="89.535" width="0.1524" layer="91"/>
<wire x1="37.465" y1="89.535" x2="8.255" y2="89.535" width="0.1524" layer="91"/>
<junction x="8.255" y="89.535"/>
<pinref part="LSP5" gate="1" pin="MP"/>
<wire x1="37.465" y1="85.725" x2="77.47" y2="85.725" width="0.1524" layer="91"/>
</segment>
</net>
</nets>
</sheet>
</sheets>
</schematic>
</drawing>
</eagle>
