/* File: ~/GliderWinchItems/BMS/hw/proxybox.scad
  * frame for holding cells for proxy battery box
  * Latest edit: 20220803
 */
 
 $fn=80;
  
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
brdwid = 83-0.0;
brdlen = 101-0.0;
brdholdx = 134.0-2.0;
brdholdmx = brdlen/2 - 58.0+3.0;
brdholdy = 44;
brdholdmy = brdholdy/2 - 22;
brdholdia = 3.5;

basethk = 4;
baserim = 8;
baserad = 4;

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
//            translate([ll/2-0.01,-brdwid/2,0])
//            cube([4,brdwid,basethk+8],center=false);
      
        }
        union()
        {
            // punch out rectangle below pcb
            lll = 94;//brdlen/2 - baserim*2-2;
            www = 50;//brdwid - baserim;      
            translate([0,-www/2,-0.01])
            rounded_rectangle_hull(lll,www,basethk+10,baserad);            
            sss = 94;
            ttt = 70;
            translate([0,ttt/2,-0.01])
            rounded_rectangle_hull(sss,ttt,basethk+10,baserad);

            translate([0,-105,-0.01])
            rounded_rectangle_hull(sss,ttt,basethk+10,baserad);



  //          translate([-brdlen/2+dd/2,-brdwid/2+dd/2,-0.01])
//                 cube([brdlen-dd,brdwid-dd,50],center=false);
            mm = ll/8+3;
            for(c=[0:7])
            {
            // JIC base mounting holes
            translate([-ll/2+5,c*mm-ww/2+4,0])cylinder(d=3.0,h=50,center=false);
 //           translate([-ll/2+4,+ww/2-4,0])cylinder(d=3.0,h=50,center=false);
            translate([+ll/2-5,c*mm-ww/2+4,0])cylinder(d=3.0,h=50,center=false);
            }
 //           translate([+ll/2-4,+ww/2-4,0])cylinder(d=3.0,h=50,center=false);    
        }
    }
}
bdia = 8.7;       // Battery diameter
batt_len = 57.5; // Battery length (not including tabs)
spc = 1.3; // Spacing between batteries
bspc = bdia + spc;
n = 9; // number of batteries

module rack(a)
{
    barht = (bdia/2 - 0.4) + basethk;
    barwid = 5;
    barlen = bspc * n + 2;
    barofy = 27.5;          
    barofy2 = 18;
    difference()
    {
        union()
        {
            translate([0,-barofy,barht/2])
              cube([barlen,barwid,barht],center=true);
            
            translate([0, barofy,barht/2])
              cube([barlen,barwid,barht],center=true);
            
            translate([0, barofy2,barht/2])
              cube([barlen+3,barwid,barht],center=true);
            
            translate([0,-barofy2,barht/2])
              cube([barlen+3,barwid,barht],center=true);
        }
        union()
        {
            for (b=[0:9])
            {
            translate([(b*bspc-bspc*5),0,(basethk+barht-bdia/2+1)])
            rotate([90,0,0])
            cylinder(d=bdia,h=batt_len,center=true);
                
            translate([(b*bspc-bspc*5),0,(basethk+barht-bdia/2+2)])
            rotate([90,0,0])
            cylinder(d=bdia-0,h=batt_len+10,center=true);                
            }
        }
    }
}

module spacer(a)
{
    translate(a)
    difference()
    {
        union()
        {
            cube([11, 66, 7.5], center=true);
        }
        union()
        {
            for (p=[0:3])
            {
                translate([0,(p-2)*18+9,0])
                //rotate([0,90,0])
                cylinder(d=3.0,h=20, center=true);
                
               translate([0,(p-2)*18,0])
                rotate([0,90,0])
                cylinder(d=3.0,h=20, center=true);                
            }
        }
    }
}
module endplate()
{
ee = 65; // y
ff = 60; // x
uu = 23; //Offset from edge
kk = 3;  // Thickness
    difference()
    {
        union()
        {
            translate([0,-ee/2,0])
            rounded_rectangle_hull(ff,ee,kk,baserad);  
            
            translate([0,ee/2-6/2,kk+6/2])
            cube([40, 6, 6], center=true);
            
           
            
        }
        union()
        {
            jj = 40; mm = 34;
            translate([0,-jj/2,0])
            rounded_rectangle_hull(mm,jj,20,baserad);  
/*            
            qq = 5; ss = 27; dd = 2.9;
               translate([qq,ss,0])
                cylinder(d=dd,h=20, center=true);                

               translate([-qq,ss,0])
                cylinder(d=dd,h=20, center=true);                
*/      
           hh = 17;
           translate([hh,ee/2-6/2,kk+6/2])
            rotate([90,0,0])
            cylinder(d = 2.9, h=100,center=true);  
      
           translate([-hh,ee/2-6/2,kk+6/2])
            rotate([90,0,0])
            cylinder(d = 2.9, h=100,center=true);  
            
            for (p=[0:3])
            {
               translate([ uu,(p-2)*18,0])
                cylinder(d=3.5,h=20, center=true);                
                
               translate([-uu,(p-2)*18,0])
                cylinder(d=3.5,h=20, center=true);                                
            }            
 
        }
    }    
}
module bottom_support()
{
wwx = 82;
zz = 13;
lll = 105+12;//brdlen/2 - baserim*2-2;
www = 54;//brdwid - baserim;  
gg = 34;
vbn = 107.5/2;
   difference()
    {
        union()
        {
            translate([0,-wwx/2,0])
            rounded_rectangle_hull(ll+15,wwx,basethk,baserad);
            
            translate([vbn,gg,zz/2+basethk-2])
            cube([12,4,zz+2],center=true);
            
            translate([-vbn,gg,zz/2+basethk-2])
            cube([12,4,zz+2],center=true);

            translate([vbn,-gg,zz/2+basethk-2])
            cube([12,4,zz-2],center=true);

            translate([-vbn,-gg,zz/2+basethk-2])
            cube([12,4,zz-2],center=true);            
        }
        union()
        {
            // punch out rectangle below pcb    
            translate([0,-www/2,-0.01])
            rounded_rectangle_hull(lll,www,basethk+10,baserad);            
            sss = 94;
            ttt = 70;
            translate([0,ttt/2,-0.01])
            rounded_rectangle_hull(sss,ttt,basethk+10,baserad);

            translate([0,-105,-0.01])
            rounded_rectangle_hull(sss,ttt,basethk+10,baserad);

            translate([vbn,30,basethk+1+7])
            rotate([90,0,0])
            cylinder(d=2.8,h=20,center=true);        
           
            translate([vbn,-30,basethk+1+6])
            rotate([90,0,0])
            cylinder(d=2.8,h=20,center=true);      
            
            translate([-vbn,30,basethk+1+7])
            rotate([90,0,0])
            cylinder(d=2.8,h=20,center=true);        
           
            translate([-vbn,-30,basethk+1+6])
            rotate([90,0,0])
            cylinder(d=2.8,h=20,center=true);                

        } 
    } 
}
module ribbon_cap()
{
ee = 65; // y
ff = 60; // x
uu = 23; //Offset from edge
kk = 3;  // Thickness
    difference()
    {
        union()
        {
            translate([0,ee/2-6/2,kk+6/2])
            cube([40, 6, 6], center=true);
        }
        union()
        {
            jj = 40; mm = 34;
            translate([0,-jj/2,0])
            rounded_rectangle_hull(mm,jj,20,baserad);  
           hh = 17;
           translate([hh,ee/2-6/2,kk+6/2])
            rotate([90,0,0])
            cylinder(d = 2.9, h=100,center=true);  
      
           translate([-hh,ee/2-6/2,kk+6/2])
            rotate([90,0,0])
            cylinder(d = 2.9, h=100,center=true);  
            
            for (p=[0:3])
            {
               translate([ uu,(p-2)*18,0])
                cylinder(d=3.5,h=20, center=true);                
                
               translate([-uu,(p-2)*18,0])
                cylinder(d=3.5,h=20, center=true);                                
            }         
            translate([0,57,0])
            cube([27,50,50],center=true);
        }
    }    
}
module resistorbox(s)
{
    translate(s)
    {
        difference()
        {
            union()
            {
                cube([54+4,10+2,17],center=true);
            }
            union()
            {
                translate([0,0,30/2-7])
                cube([54,10,30],center=true);
            }
        }
    }
}
module loadbar()
{
abn = 107.5/2;
xcv = -27;    
   difference()
   {
      union()
      {
          cube([abn*2+10,4,17],center=true);
          
          translate([10+xcv,-8,0])
          cube([8,12,17],center=true);

 
                      
      }
     union()
     {
    translate([11.5+xcv,-9,-15])
         rotate([0,0,90])
         rounded_rectangle_hull(5.0,3.0,30,1.25);    
    
         
          translate([abn,0,17/2-3])
            rotate([90,0,0])
            cylinder(d=2.8,h=20,center=true);                    

          translate([-abn,0,17/2-3])
            rotate([90,0,0])
            cylinder(d=2.8,h=20,center=true);                    
     }  
  } 
      resistorbox([16,-8,0]);
}

module total()
{
    base();
    
    translate([0,0,0])
    {
        rack([0,0,0]);
    }
}
//loadbar(); //proxybox_L
//rotate([90,0,0]) ribbon_cap(); // proxybox_R
bottom_support(); // proxybox_B
//endplate(); // proxybox_A
//total();
//spacer([ ll/2-5.0,-1,basethk+7.5/2]);
//spacer([-ll/2+5.0,-1,basethk+7.5/2]);