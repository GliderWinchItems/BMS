/* File: ~/GliderWinchItems/BMS/hw/stncl-14921.scad
  * SMD Stencilling frames for MAX14921
  * Latest edit: 20230223
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

mlen = 160; 
mwid = 70;
mthk = 4;    // Space under pcb
pthk = 1.6;  // PCB thickness
lipwid = 10; // Lip width
lipthk = 1;
plen = 140;
pwid = 50;

tthk = mthk+pthk; // Total thickness

module post(a,ll,ww)
{
    translate(a)
    {
        translate([-plen/2+ll/2,0,mthk/2])
        cube([ll,ww,mthk],center=true);
    }
}
module main()
{
    difference()
    {
        union()
        {
          translate([0,0,tthk/2])
          cube([mlen,mwid,tthk],center=true);
           
          translate([0,mwid/2,lipthk/2])
          cube([mlen,lipwid+.01,lipthk],center=true); 
            
          translate([0,-mwid/2,lipthk/2])
          cube([mlen,lipwid+.01,lipthk],center=true);             

          translate([mlen/2,0,lipthk/2])
          cube([lipwid+.01,mwid,lipthk],center=true); 

          translate([-mlen/2,0,lipthk/2])
          cube([lipwid+.01,mwid,lipthk],center=true);   
        }
        union()
        {
          {  
            translate([0,0,tthk - pthk/2])
            cube([plen,pwid,50],center=true);
              
          }
        }
    }
}

main();

p1 = 16;
post([140-p1,0,0],p1,50);
post([140-90,pwid/2-3.7/2,0],90,3.7);
post([0,pwid/2-31/2,0],17,31);