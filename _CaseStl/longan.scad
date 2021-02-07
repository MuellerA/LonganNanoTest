////////////////////////////////////////////////////////////////////////////////
// longan.scad
////////////////////////////////////////////////////////////////////////////////

// https://github.com/MuellerA/Thingiverse.git#ac669d2e31545f4aca594795901329ed9a7b87eb
use <../../../3D Druck/Thingiverse/PCB/pcb.scad> ;

pcb = "top" ; // [top,bottom,button]


/* [Hidden] */

length = 46.5 ;
width  = 27.5 ;
wallWidth  =  0.8 ;

pcbLength  = 46.5 ;
pcbWidth   = 20.5 ;
pcbHeight  =  1.9 ;
railHeight =  6.0 ;
railOffset =  3.0 ;

height = railHeight + pcbHeight + wallWidth + 0.5 ;
  
dispLength = 23.9 ;
dispWidth  = 13.5 ;
dispOffset =  3.5 ;

buttonD       = 3.0 ;
buttonLoffset = pcbLength / 2 - 8.9 ;
buttonWoffset = 3.5 ;

usbWidth  = 9.9 ;
usbHeight = 3.9 ;
usbOffset = 2.0 ;
  
$fn=40 ;

module base(height)
{
  translate([0, 0, height/2])
    difference()
  {
    translate([0, 0, -wallWidth/2])
      cube([length + 2*wallWidth, width + 2*wallWidth, height+wallWidth], center=true) ;
    translate([0, 0, +wallWidth/2])
      cube([length + 0*wallWidth-0.01, width + 0*wallWidth-0.01, height+wallWidth+0.01], center=true) ;
  }
}

module hooks(height, top)
{
  w = wallWidth ;
  d = w/2 ;
  points =
    [
      [ [0, 0], [w, -w], [0, -2*w], [-w, -2*w], [-w, 0] ],
      [ [0, 0], [0, -2*w], [w+d, -2*w], [w+d, 0.5*w], [d, 1.5*w], [w+d, 2.5*w], [2*w+d, 2.5*w], [2*w+d, -3*w], [0, -6*w], [-w, -6*w], [-w, 0] ]
    ] ;

  module hook(points)
  {
    rotate([0, 0, 90])
      rotate([90, 0, 0])
      linear_extrude(height=3, center=true)
      polygon(points) ;
  }

  p1 = (top) ? points[0] : points[1] ;
  p2 = (top) ? points[1] : points[0] ;
  
  translate([+length/2-5, -width/2, height]) rotate([0, 0,   0]) hook(p1) ;
  translate([0          , -width/2, height]) rotate([0, 0,   0]) hook(p2) ;
  translate([-length/2+5, -width/2, height]) rotate([0, 0,   0]) hook(p1) ;
  translate([+length/2-5, +width/2, height]) rotate([0, 0, 180]) hook(p1) ;
  translate([0          , +width/2, height]) rotate([0, 0, 180]) hook(p2) ;
  translate([-length/2+5, +width/2, height]) rotate([0, 0, 180]) hook(p1) ;
}

module top()
{
  module pins(n)
  {
    translate([0, 0, -wallWidth/2])
      cube([n*2.54+0.8, 2.54+0.8, 2*wallWidth], center=true) ;        
  }

  difference()
  {
    union()
    {
      // base
      base(height) ;
      
      // pcb frame
      cuts =
        [
          [ -1.5, +pcbWidth/2-railOffset, pcbLength-8, +2, -railHeight ],
          [ -1.5, -pcbWidth/2+railOffset, pcbLength-8, +2, -railHeight ]
        ] ;
      rotate([0, 0, 90])
        PcbHolder(pcbLength, pcbWidth, pcbHeight, wallWidth, railHeight, railOffset,
                  clip = [10, 5], cuts=cuts) ;

      translate([-dispLength/2+dispOffset-1.5*wallWidth, 0, 2/2]) cube([wallWidth, pcbWidth-2*railOffset-wallWidth , 2], center=true) ;
      translate([0, +pcbWidth/2-railOffset-wallWidth/4, 2/2]) cube([pcbLength, 0.4, 2], center=true) ;
      translate([0, -pcbWidth/2+railOffset+wallWidth/4, 2/2]) cube([pcbLength, 0.4, 2], center=true) ;
      
      // button
      bd = buttonD + 2 * 0.4 ;
      translate([-buttonLoffset, +buttonWoffset, 1.6/2]) cylinder(d=bd+2*wallWidth, h=1.6, center=true) ;      
      translate([-buttonLoffset, -buttonWoffset, 1.6/2]) cylinder(d=bd+2*wallWidth, h=1.6, center=true) ;      
    }

    union()
    {
      // display
      translate([dispOffset, 0, 0]) cube([dispLength, dispWidth, 10], center=true) ;

      // button
      bd = buttonD + 2 * 0.4 ;
      translate([-buttonLoffset, +buttonWoffset, 0]) cylinder(d=bd, h=10, center=true) ;
      translate([-buttonLoffset, -buttonWoffset, 0]) cylinder(d=bd, h=10, center=true) ;
      
      // usb
      translate([-length/2, 0, 10/2+usbOffset])
        cube([10, usbWidth, 10], center=true) ;

      // sdcard
      translate([-length/2 + 10/2 - 2*wallWidth, 0, railHeight + pcbHeight + 2])
        cube([10, 12, 4], center=true) ;
      
      // ser/jtag
      extra = pcbHeight/2 + wallWidth - 2.54 ;
      translate([+(pcbLength+wallWidth)/2, 0, railHeight + pcbHeight/2]) cube([wallWidth*2, 11, 2.54*2], center=true) ;

      // pin
      translate([+5*2.54, +3.5*2.54,0]) pins(8) ;
      translate([+5*2.54, -3.5*2.54,0]) pins(8) ;
      translate([-4.5*2.54, +3.5*2.54,0]) pins(9) ;
      translate([-4.5*2.54, -3.5*2.54,0]) pins(9) ;
    }
  }

  hooks(height, true) ;
}

module button()
{
  cylinder(h=1, d=4.4) ;
  cylinder(h=6, d=2.5) ;
}

module bottom()
{
  height = 3*wallWidth ;
  
  difference()
  {
    union()
    {
      base(height) ;

      hooks(height, false) ;
    }

    union()
    {
      translate([0, 0, -5-wallWidth])
      cube([80, 40, 10], center=true) ;

      // sdcard
      translate([-length/2 + 6/2 - 2*wallWidth, 0, (height+wallWidth)/2-wallWidth])
        cube([6, 12, height+2*wallWidth], center=true) ;

      // ser/jtag
      extra = pcbHeight/2 + wallWidth - 2.54 ;
      translate([+(pcbLength+wallWidth)/2, 0, height + 2.54 + extra]) cube([wallWidth*2, 11, 2.54*2], center=true) ;
    }
  }

}

if (pcb == "top")
  top() ;
else if (pcb == "bottom")
  bottom() ;
else if (pcb == "button")
  button() ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
