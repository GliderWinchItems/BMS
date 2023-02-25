/* File: ~/GliderWinchItems/BMS/hw/proxy2.scad
  * frame for proxy pack 650 mah batteries
  * Latest edit: 20230215
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
blen = 53.0; // Battery length
bwid = 30.5; // Battery width
bthk =  7.0; // Battery thickness
bipst = 4.0; // Inner posts between batterys stacks
bepst = 6.0; // End posts 
bhpst = bthk*6 + 2.0;// Post height
pcbwid = 25; // pcb mounting space

len = bwid*3 + 2*bipst + 2*bepst;
wid = blen + 2*bepst + pcbwid ;
ht = bhpst;;
wall = 3;
bot = 3;

psq = 6.0;
pht = ht - 0; // Post height
phole = 48;//54;

module post(a,xht,sq,da)
{
    translate(a)
    {
        difference()
        {
            union()
            {
                translate([0,0,xht/2])
                cube([sq,sq,xht],center = true);
            }
            union()
            {
                translate([0,0,xht-24])
                cylinder(d=da,h=25, center=false);
            }
        }
    }
}
pcboff = 3.2;    
pcby2 = 60  - pcboff;
pcby1 = pcboff;
pcbdia = 2.8;
module postpcb(a)
{

   
    translate(a)
    {
      difference()
    {
        union()
        {
          translate([0,0,bot])
          rotate([90,0,-90])
           linear_extrude(height = 6, center = true, convexity = 10, twist = 0)
             polygon(points=[[0,0],[0,60],[9,60],[20,0]]);

 
        }
        union()
        {
            translate([0,0,pcboff+bot])
            rotate([90,0,0])
            cylinder(d=pcbdia,h=100,center=true);
            
            translate([0,0,pcboff+bot+(60-2*pcboff)])
            rotate([90,0,0])
            cylinder(d=pcbdia,h=100,center=true);
            

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
            translate([0,pcbwid/2,bot/2])
            cube([len,wid,bot],center = true);
            
            translate([-len/2+1,7.5,8/2])
            cube([2,80,8],center=true);
            
            translate([ len/2-1,7.5,8/2])
            cube([2,80,8],center=true);
 
        }
        union()
        {
            qlen = 25; qwid = 50;
           translate([0,0,0])
            cube([qlen,qwid,100],center = true); 
           translate([bwid*1+bipst,0,0])
            cube([qlen,qwid,100],center = true);  
            translate([-(bwid*1+bipst),0,0])
            cube([qlen,qwid,100],center = true); 
  
            rlen = len-28; rwid = 18;
            translate([5,wid/2-4,0])
            cube([rlen,rwid,100],center = true); 
            
 
        }
    }
}
pdel = 6;
poff = bwid/2 + bipst/2;
py2 = blen/2 - bipst/2 - pdel;
px3 = bwid*1.5 + bipst + bepst/2;
py3 = blen/2 -pdel;
px4 = 0 + bipst/2;
py4 = blen/2 + bipst/2;

module boxNpost()
{
box();
post([ poff, py2,0],pht,bipst,0);
post([ poff,-py2,0],pht,bipst,0);
post([-poff, py2,0],pht,bipst,0);
post([-poff,-py2,0],pht,bipst,0);

post([ px3, py3,0],ht,bepst,2.9);
post([ px3,-py3,0],ht,bepst,2.9);
post([-px3, py3,0],ht,bepst,2.9);
post([-px3,-py3,0],ht,bepst,2.9);

post([ px4, py4,0],ht,bipst,0);
post([ px4,-py4,0],ht,bipst,0);
post([-px4, py4,0],ht,bipst,0);
post([-px4,-py4,0],ht,bipst,0);

px5 = bwid* 1 + bipst; 
post([ px5, py4,0],ht,bipst,0);
post([ px5,-py4,0],ht,bipst,0);
post([-px5, py4,0],ht,bipst,0);
post([-px5,-py4,0],ht,bipst,0);
}

module senseRtab(a,dd,rr)
{
    zh = 2;
    z2 = 8;
    z1 = z2+zh;    
    z3 = z2/2;
    translate(a)
    {
      rotate([0,0, rr])
      rotate([90,0,0])
      rotate([0,90,0])
      {
        difference()
        {
        union()
        {
          linear_extrude(height = zh, center = false, convexity = 10, twist = 0)
          polygon(points=[[0,0],[0,z1],[z2,z1],[z2,z3]]);
        }
        union()
        {
            translate([z2/2+1,z1-3,0])
            cylinder(d=dd,h=50,center=true);
        }
       }
     }
    }
}
frame = 1;
if (frame==1)
{

// 0.1 ohm current sense resistor mounting tabs
senseRtab([-50+9-3,len/2-2-2,pcboff+bot+(60-2*pcboff-10)],1.5,90);
senseRtab([-50+9-3,len/2-2-2,-2],1.5,90);

// +/- Power pole wire strain relief
senseRtab([-50+9-3,len/2-2-0,45],3.2,0);
senseRtab([50+9-5,len/2-2-0,45],3.2,0);

boxNpost();

translate([-50+9,len/2,0])
union()
{
  postpcb([  0,0,0]);
  postpcb([(100-2*pcboff),0,0]);
}
}

module clamp()
{
 del2 = -5;
    cht = 5;
 difference()
 {
     union()
     {
        translate([0,0,ht/2])
        cube([2*(px3+2.9),2*(py3+2.9),cht],center = true);
     }
     union()
     {
       cube([2*(px3-2.9),2*(py3-2.9),50],center = true);
         
       translate([ px3, py3,0]) cylinder(d=3.2,h=60);       
       translate([-px3, py3,0]) cylinder(d=3.2,h=60);       
       translate([ px3,-py3,0]) cylinder(d=3.2,h=60);       
       translate([-px3,-py3,0]) cylinder(d=3.2,h=60);       
     }
 }   
}

translate([0,0,35])clamp();