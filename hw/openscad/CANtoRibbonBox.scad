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
len = 60.5;
wid = 60.5;
ht = 15;
wall = 3;
bot = 3;

psq = 6.0;
pht = ht - 1.5;
phole = 54;

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
            
            xof = len/2+wall+2;
            translate([ xof, xof,0]) cylinder(d=3.5,h=50,center=true);
            translate([ xof,-xof,0]) cylinder(d=3.5,h=50,center=true);
            translate([-xof, xof,0]) cylinder(d=3.5,h=50,center=true);
            translate([-xof,-xof,0]) cylinder(d=3.5,h=50,center=true);
        }
    }
}
poff = phole/2;
box();
post([ poff, poff,0]);
post([ poff,-poff,0]);
post([-poff, poff,0]);
post([-poff,-poff,0]);
