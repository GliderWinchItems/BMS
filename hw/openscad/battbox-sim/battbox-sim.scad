/* File: battbox-sim.scad
 * Battery Box for small LipPo bms simulation of big box for winch
 * Author: deh
 * Latest edit: 20200429
 */
 $fn = 30;
 
 // base plate
 module rounded_bar(d, l, h)
{
    // Rounded end
    cylinder(d = d, h = h, center = false);
    // Bar
    translate([0, -d/2, 0])
       cube([l, d, h],false);
}
/* ***** eyebar *****
 * rounded bar with hole in rounded end
 * module eye_bar(d1, d2, len, ht)
d1 = outside diameter of rounded end, and width of bar
d2 = diameter of hole in end of bar
eye_bar(d1, d2, len, ht);
*/
module eye_bar(d1, d2, len, ht)
{
   difference()
   {   
      rounded_bar(d1,len,ht);
      cylinder(d = d2, h = ht + .001, center = false);
   }
}
/* ***** rounded_rectangle ******
rounded_rectangle(l,w,h,rad);
l = length (x direction)
w = width (y direction)
h = thickness (z direction)
rad = radius of corners
reference = center of rectangle x,y, bottom
*/
module rounded_rectangle(l,w,h,rad)
{
  translate([-(l-rad)/2,-(w-rad)/2,0])
  {
    // Four rounded edges
    translate([0,0,0])
      rounded_bar(rad*2,l-rad,h);
    translate([l-rad,0,0])
      rotate([0,0,90])
      rounded_bar(rad*2,w-rad,h);
    translate([l-rad,w-rad,0])
      rotate([0,0,180])
      rounded_bar(rad*2,l-rad,h);
    translate([0,w-rad,0])
      rotate([0,0,-90])
      rounded_bar(rad*2,w-rad,h);
    // Fill in center
    translate([0,0,0])
      cube([l-rad + .01,w-rad + .01,h],false);
  }
}
bat_dia = 8.5; // Battery diameter
bat_ht  = 56;  // Battery height/length
bat_spc = 4;   // Spacing between batteries

batnum_x = 6;  // Number of columns of batteries
batnum_y = 3;  // Number of rows of batteries
rad = 3;
batrack_thick = 4;

batrack_side = 3; // Allowance for side rails
module batrack(hole_dia,h)
{
    l = (batnum_x + 1)*bat_spc + (batnum_x * bat_dia);
    w = (batnum_y + 1)*bat_spc + (batnum_y * bat_dia);
    difference()
    {
        union()
        {
            rounded_rectangle(l,w,h,rad);
        }
        union()
        {
            del = bat_dia + bat_spc;
            a = batnum_x*.5;
            b = batnum_y*.5;
            off = bat_dia/4+bat_spc;
            for (x=[ -a: 1 : a-1] )
            {
                for (y=[-b : 1 : b-1] )
                {
                    translate([del*x+off,del*y+off,0])
                     cylinder(d=hole_dia,h=100,center=true);
                }
            }
            /*
            c = 2;//batnum_x*.5;
            d = 2;//batnum_y*.5;
            off2 = bat_dia/4;            
            for (x2=[ -c: 1 : c] )
            {
                for (y2=[-d : 1 : d] )
                {
                    translate([del*x2+off2,del*y2+off2,0])
                     star_hole([0,0,0]);
                }
            }
            */
        }
    }
}
cp_thick = 3;   // Thickness of wall
cp_inrad = rad; // Inside radius
cp_outrad = cp_inrad+cp_thick; // Outsid radius
cp_ht = 56;
cp_ldgw = 1;
cp_ldgz1 = cp_ldgw;
cp_ldgz2 = cp_ht/2;
cp_ldgz3 = cp_ht - batrack_thick;
cp_flat = 20;


module corner_p()
{
r2 = cp_inrad;
r1 = r2 - cp_ldgw;
r3 = r2 + cp_thick;   
    
    
h1 = cp_ldgz1;
h2 = cp_ldgz2 - cp_ldgw;
h3 = cp_ldgz2;
h4 = cp_ldgz3 -  cp_ldgw;
h5 = cp_ldgz3;
h6 = cp_ht;    
    
    polygon(points=[
    [r2, 0],
    [r1,h1],
    [r2,h1],
    [r2,h2],
    [r1,h3],
    [r2,h3],
    [r2,h4],
    [r1,h5],
    [r2,h5],
    [r2,h6],
    [r3,h6],
    [r3, 0]
    ]);
}
module corner_side(zz)
{
    rotate([90,0,0])
    {
        linear_extrude(height=zz)
        {
            corner_p();
        }
    }
}
module corner_pole()
{
    difference()
    {
        union()
        {
            rotate_extrude(angle=90)
            {
                corner_p();
            }
            corner_side(cp_flat);
            translate([-cp_flat,0,0])
              rotate([0,0,90])
                corner_side(cp_flat);
        }
        union()
        {
        }
    }
}

module star_hole(a)
{
ds = (bat_dia+bat_spc)*.5+9;
dq =  (bat_dia+bat_spc)*.5;
df = dq - 1.5;
hh = 50;
    translate(a)
    { 
        difference()
        {
          union()
          {
            translate([0,0,0])
             cube([ds,ds,hh],center=true);
          }
          union()
          {
            translate([-ds/2, ds/2,0])cylinder(d=ds,h = hh,center=true);
            translate([-ds/2,-ds/2,0])cylinder(d=ds,h = hh,center=true);
            translate([ ds/2, ds/2,0])cylinder(d=ds,h = hh,center=true);
            translate([ ds/2,-ds/2,0])cylinder(d=ds,h = hh,center=true);
              
            difference()
            {
                cube([ds,ds,hh],center=true);
                cube([ds-df,ds-df,hh],center=true);
            }
          }
        }
    }
}

cp_endz = 20;
module longendpiece()
{
cp_l = (batnum_x + 1)*bat_spc + (batnum_x * bat_dia)-rad;
cp_w = (batnum_y + 1)*bat_spc + (batnum_y * bat_dia)-rad;
    difference()
    {
        union()
        {
            // Two corner pillars
            translate([cp_l/2,cp_w/2,0]) 
                corner_pole();
    
            translate([cp_l/2,-cp_w/2,0]) 
             rotate([0,0,-90]) 
                corner_pole(); 
 
            translate([-30,cp_w/2,0])
             rotate([0,0,90])
                corner_side(cp_l/2+cp_flat);   

            translate([30,-cp_w/2,0])
             rotate([0,0,-90])
                corner_side(cp_l/2+cp_flat);

        }
        union()
        {
            translate([-8+5,40,25+10])            
             rotate([90,0,0])            
                rounded_rectangle(70,50,80,4);       
            
            translate([0,0,35])            
             rotate([90,0,90])            
                rounded_rectangle(25,50,50,4);                
        }
    }
}

module angle_hole(a,r,dia,len)
{
    translate(a)
    {
        rotate([0,0,r])
          rotate([90,0,0])
           cylinder(d=dia,h=len,center=false);
    }
}
/* Bottom "rack" (plate) for batteries. */
bot_hole_dia = 2.8;
module bat_rack_bottom()
{
l = (batnum_x + 1)*bat_spc + (batnum_x * bat_dia)-9;
w = (batnum_y + 1)*bat_spc + (batnum_y * bat_dia)-9;
m = (batnum_x + 1)*bat_spc + (batnum_x * bat_dia)+9;
n = (batnum_y + 1)*bat_spc + (batnum_y * bat_dia)+9;
u = bot_hole_dia;
    
    difference()
    {
        union()
        { 
            translate([0,0,0]) batrack(bat_dia,batrack_thick);
            translate([0,0,0]) batrack(5,1.5); // Keep batt from falling thru
        }
        union()
        {
            // Corner holes for screws
            angle_hole([ l*.5,-w/2,batrack_thick*.5], 45,u,12);
            angle_hole([-l*.5,-w/2,batrack_thick*.5],-45,u,12);
            angle_hole([ m*.5, n/2,batrack_thick*.5],-45,u,12);
            angle_hole([-m*.5, n/2,batrack_thick*.5], 45,u,12);
            
            // Side holes 
            angle_hole([0,   n/2,batrack_thick*.5], 0,u,12);
            angle_hole([0,-w/2+3,batrack_thick*.5], 0,u,12);
        }
    }
}
module bat_rack_top()
{
l = (batnum_x + 1)*bat_spc + (batnum_x * bat_dia)-9;
w = (batnum_y + 1)*bat_spc + (batnum_y * bat_dia)-9;
m = (batnum_x + 1)*bat_spc + (batnum_x * bat_dia)+9;
n = (batnum_y + 1)*bat_spc + (batnum_y * bat_dia)+9;
u = bot_hole_dia;

    difference()
    {
        union()
        { 
            translate([0,0,0]) batrack(bat_dia,batrack_thick);
        }
        union()
        {
            // Corner holes for screws
            angle_hole([ l*.5,-w/2,batrack_thick*.5], 45,u,12);
            angle_hole([-l*.5,-w/2,batrack_thick*.5],-45,u,12);
            angle_hole([ m*.5, n/2,batrack_thick*.5],-45,u,12);
            angle_hole([-m*.5, n/2,batrack_thick*.5], 45,u,12);
            
            // Side holes 
            angle_hole([0,   n/2,batrack_thick*.5], 0,u,12);
            angle_hole([0,-w/2+3,batrack_thick*.5], 0,u,12);
        }
    }
    
    
}

/* Mid-level plate with "star" cutouts. */
module bat_rack_star()
{
    difference()
    {
    translate([0,0,0]) batrack(bat_dia,batrack_thick);
        union()
        {
        zx = (bat_spc+bat_dia)*.5;
          star_hole([zx*0,-(bat_spc+bat_dia)*.5,0]);
          star_hole([zx*0, (bat_spc+bat_dia)*.5,0]);
        
          star_hole([zx*-2,-(bat_spc+bat_dia)*.5,0]);
          star_hole([zx*-2, (bat_spc+bat_dia)*.5,0]);

          star_hole([zx*-4,-(bat_spc+bat_dia)*.5,0]);
          star_hole([zx*-4, (bat_spc+bat_dia)*.5,0]);


          star_hole([zx* 2,-(bat_spc+bat_dia)*.5,0]);
          star_hole([zx* 2, (bat_spc+bat_dia)*.5,0]);

          star_hole([zx* 4,-(bat_spc+bat_dia)*.5,0]);
          star_hole([zx* 4, (bat_spc+bat_dia)*.5,0]);
            
           l = (batnum_x + 1)*bat_spc + (batnum_x * bat_dia)-9;
w = (batnum_y + 1)*bat_spc + (batnum_y * bat_dia)-9;
m = (batnum_x + 1)*bat_spc + (batnum_x * bat_dia)+9;
n = (batnum_y + 1)*bat_spc + (batnum_y * bat_dia)+9;
u = bot_hole_dia;// Corner holes for screws
            angle_hole([ l*.5,-w/2,batrack_thick*.5], 45,u,12);
            angle_hole([-l*.5,-w/2,batrack_thick*.5],-45,u,12);
            angle_hole([ m*.5, n/2,batrack_thick*.5],-45,u,12);
            angle_hole([-m*.5, n/2,batrack_thick*.5], 45,u,12);            
        }
    }
}
/* Mirroed end piece. */
module shortendpiece()
{
    difference()
    {
        mirror([1,0,0])
            longendpiece();
        
        translate([75,0,0])
            cube([100,100,100],center=true);
    }
}

/* Short end piece with sides shaved to dovetail withe other end. */
dovehole = 3.0;

module shortenddove()
{
difference()
{
    shortendpiece();
    union()
    {
       wj = (batnum_y + 1)*bat_spc + (batnum_y * bat_dia);
        translate([-30, -wj*.5-rad, 0])
          cube([55,10,30],center=false);            

        translate([-30, wj*.5+rad-10, 0])
          cube([55,10,30],center=false);   
     
        translate([0,0,4])
         rotate([90,0,0])
          cylinder(d=dovehole,h=200,center=true);
    }   
}
}
/* Original (long) end piece with sides cutout to dovetail. */
module longenddove()
{
    difference()
    {
        longendpiece();
        union()
        {
            wj = (batnum_y + 1)*bat_spc + (batnum_y * bat_dia);
            translate([-30, -wj*.5-10-rad, 0])
                cube([55,10,30],center=false);            

            translate([-30, wj*.5+rad, 0])
                cube([55,10,30],center=false);    
                 translate([0,0,4])

         rotate([90,0,0])
          cylinder(d=dovehole,h=200,center=true);  
        }
    }
}
translate([0,0,0]) longenddove();
//translate([0,0,0]) shortenddove();
//translate([0,0,cp_ldgz2]) bat_rack_star(); // Mid-level
translate([0,0,cp_ldgz1]) bat_rack_bottom(); // Bottom
translate([0,0,cp_ldgz3]) bat_rack_top(); // Top
