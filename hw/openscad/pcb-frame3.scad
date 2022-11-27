/* File: ~/GliderWinchItems/BMS/hw/openscad/pcb-frame3.scad
  * frame for working on BMS pc board
  * Latest edit: 20220815
 */
 
 $fn=40;
  
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
brdwid = 50-0.0;
brdlen = 140-0.0;
brdholdx = 134.0-2.0;
brdholdmx = brdlen/2 - 58.0+3.0;
brdholdy = 44;
brdholdmy = brdholdy/2 - 22;
brdholdia = 3.5;

basethk = 3;
baserim = 8;
baserad = 4;

crnr = 4;
   ll = baserim*2 + brdlen;
    ww = baserim*2 + brdwid;

module base()
{
 
    difference()
    {
        union()
        {
            translate([0,-ww/2,0])
            rounded_rectangle_hull(ll,ww,basethk,baserad);
            
            // Lip to strengthen cable connect end of pcb
            uu=136;
            translate([-uu/2,-brdwid/2+1,basethk])
            cube([uu,3,4],center=false);
            translate([-uu/2,brdwid/2-4,basethk])
            cube([uu,3,4],center=false);
      
        }
        union()
        {
            // punch out rectangle below pcb
            lll = brdlen/2 - baserim*2+2;
            www = brdwid - baserim;
            translate([lll/2+4,-www/2,-0.01])
            rounded_rectangle_hull(lll+16,www,basethk+10,baserad);
            
            translate([-lll/2-11-1,-www/2,-0.01])
            rounded_rectangle_hull(lll-1,www,basethk+10,baserad);            

  //          translate([-brdlen/2+dd/2,-brdwid/2+dd/2,-0.01])
//                 cube([brdlen-dd,brdwid-dd,50],center=false);
            
            // JIC base mounting holes
 /*           translate([-ll/2+4,-ww/2+4,0])cylinder(d=3.0,h=50,center=false);
            translate([-ll/2+4,+ww/2-4,0])cylinder(d=3.0,h=50,center=false);
            translate([+ll/2-4,-ww/2+4,0])cylinder(d=3.0,h=50,center=false);
            translate([+ll/2-4,+ww/2-4,0])cylinder(d=3.0,h=50,center=false);    
*/            
        }
    }
}
// Mounting post and holes for pcb
module post(a)
{
    jj = basethk+12+8;
    translate(a)
    { difference()
        {
            union()
            {
                translate([0,0,jj/2])
                cylinder(d1=crnr+2,d2=crnr,h=jj,center=true);
         }
            union()
            {
            }
        }
    }
}
module postsq(s,v)
{
    jj = basethk+12+8;
hy = 7;
hz = 20;
lip = 3;
lw = 2;
    translate(s)
    rotate(v)
    { difference()
        {
            union()
            {
                translate([0,0,jj/2+lip])
                cube([hy,hy,jj],center=true);           
            }
            union()
            {
                translate([lw+hz/2-hy/2,lw+hz/2-hy/2,(jj+lip)+24/2-lip])
                cube([hz,hz,24],center=true);
            }
        }
    }
}
module total()
{
xx = 0; // Hole centering adjustment    
    base();
    
    translate([0,0,0])
    {
cc = (141-5)/2;
bb = (51-3.3)/2;        
    postsq([-cc,-bb,0],[0,0, 0]);
    postsq([+cc,-bb,0],[0,0, 90]);
    postsq([-cc,+bb,0],[0,0,-90]);
    postsq([+cc,+bb,0],[0,0,180]);
        kk=-1;
    post([-brdholdmx/2+kk,-brdholdy/2,0]);
    post([-brdholdmx/2+kk,+brdholdy/2,0]);       
    post([-brdholdmx/2+kk,+brdholdmy/2,0]);        
    }
}

total();