/* File: ~/GliderWinchItems/BMS/hw/ELCON-CANbox.scad
  * frame for pc board
  * Latest edit: 20230126
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
len = 46.2;
wid = 50.2;
ht = 15;
wall = 3;
bot = 3;

psq = 6.0;
pht = ht - 1.5;
phole = 48;//54;

module post(a,xht)
{
    translate(a)
    {
        difference()
        {
            union()
            {
                translate([0,0,xht/2])
                cube([psq,psq,xht],center = true);
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
    del1 = 2.8;
    difference()
    {
        union()
        {
            translate([0,0,ht/2])
            cube([len+2*wall,wid+2*wall,ht],center = true);
            translate([0,0,2/2])
            cube([len+2*wall+18,wid+2*wall+12,2],center = true);

        }
        union()
        {
            translate([0,0,ht/2+bot])
            cube([len-wall+del1,wid-wall+del1,ht],center = true);
            
            // Mounting holes in base
            xof = len/2+wall+2;
            xofx = len/2+wall+5;
            translate([ xofx, xof,0]) cylinder(d=3.5,h=50,center=true);
            translate([ xofx,-xof,0]) cylinder(d=3.5,h=50,center=true);
            translate([-xofx, xof,0]) cylinder(d=3.5,h=50,center=true);
            translate([-xofx,-xof,0]) cylinder(d=3.5,h=50,center=true);
        }
    }
}
poff = phole/2;
py2 = poff + 2;
box();
post([ poff, py2,0],pht);
post([ poff,-py2,0],pht);
post([-poff, py2,0],pht);
post([-poff,-py2,0],pht);

px3 = 18;
py3 = 31;

post([ px3, py3,0],ht);
post([ px3,-py3,0],ht);
post([-px3, py3,0],ht);
post([-px3,-py3,0],ht);

module clamp()
{
 del2 = -5;
    cht = 5;
 difference()
 {
     union()
     {
        translate([0,0,ht/2])
        cube([len+2*wall,wid+2*wall,cht],center = true);
     }
     union()
     {
        translate([0,0,ht/2+bot])
        cube([len-wall+del2,wid-wall+del2,20],center = true);
         
        translate([14,15,ht/2+bot])
        cube([15,15,20],center = true);
         
     }
 }   
}
translate([0,0,11])clamp();