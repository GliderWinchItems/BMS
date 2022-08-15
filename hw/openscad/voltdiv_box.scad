/* File: ~/GliderWinchItems/BMS/hw/CantoRibbonBox.scad
  * frame for pc board
  * Latest edit: 20210930
 */
 
 $fn=20;
  
module rounded_rectangle_hull(wid,slen,ht,rad)
{
 hull()
 {    
    translate([-wid/2+rad,rad,0])
        cylinder(r=rad,h=ht,center=false);

   translate([ wid/2-rad,rad,0])
        cylinder(r=rad,h=ht,center=false);

    translate([-wid/2+rad,slen-rad,0])
        cylinder(r=rad,h=ht,center=false);
    
    translate([ wid/2-rad,slen-rad,0])
        cylinder(r=rad,h=ht,center=false);
 }
}
len = 90.0+0.0;
wid = 60.3+0.0;
ht = 15;
wall = 3;
bot = 3;

psq = 6.0;
pht = ht - 1.5;
pholey = wid-6;
pholex = len-6;

module post(a)
{
    translate(a)
    {
        difference()
        {
            union()
            {
                translate([0,0,pht/2])
                cube([psq,psq,pht],center = true);
            }
            union()
            {
                cylinder(d=2.9,h=100, center=false);
            }
        }
    }
}


module box()
{
    del1 = 2.5;
    difference()
    {
        union()
        {
            translate([0,0,ht/2])
            cube([len+2*wall,wid+2*wall,ht],center = true);
            translate([0,0,2/2])
            cube([len+2*wall+10,wid+2*wall+10,2],center = true);

        }
        union()
        {
            translate([0,0,ht/2+bot])
            cube([len-wall+del1,wid-wall+del1,ht],center = true);
            
            xofx = len/2+wall+2;
            xofy = wid/2+wall+2;
            translate([ xofx, xofy,0]) cylinder(d=3.5,h=50,center=true);
            translate([ xofx,-xofy,0]) cylinder(d=3.5,h=50,center=true);
            translate([-xofx, xofy,0]) cylinder(d=3.5,h=50,center=true);
            translate([-xofx,-xofy,0]) cylinder(d=3.5,h=50,center=true);
        }
    }
}
poffx = pholex/2-0.8;
poffy = pholey/2-0.8;
box();
post([ poffx, poffy,0]);
post([ poffx,-poffy,0]);
post([-poffx, poffy,0]);
post([-poffx,-poffy,0]);
