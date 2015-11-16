STAND_INCLINE = 240;
STAND_LENGTH = 120;
STAND_HEIGHT = 10;

BASE_WIDTH = 90;
BASE_LENGTH = 75;
BASE_HEIGHT = 2;

DUMMY_HEIGHT = 10;

ENGDUINO_WIDTH = 60;
ENGDUINO_LENGTH = 80;
ENGDUINO_HEIGHT = 5;

TEXT_HEIGHT = 2;
TEXT_FONT = "Lato";
TEXT_SIZE = 7;

ATTACH_POINT_HEIGHT = 1;
ATTACH_POINT_RADIUS = 2;

module attachment_point(x, y, z) {
    translate([x, y, z]) {
        cylinder(h = ATTACH_POINT_HEIGHT, r = ATTACH_POINT_RADIUS);
    }
}

module base(length = BASE_LENGTH, width = BASE_WIDTH, height = BASE_HEIGHT) {
    difference() {
        cube (size = [length, width, height]);
        translate([width / 4,width * 1 / 8,0])
            cube (size = [length, width * 0.75, height]);
        attachment_point(BASE_LENGTH - 7,5,1);
        attachment_point(BASE_LENGTH - 7,BASE_WIDTH - 5,1);
    }
    
}

// The dummy element removes any extruding parts from the bottom.
module dummy_element(length = BASE_LENGTH, width = BASE_WIDTH, height = DUMMY_HEIGHT) {
    translate([0, 0, -height]) {
        cube(size = [length, width, height]);
    }
}

// the space where the engduino goes.
module engduino_fill(length = ENGDUINO_LENGTH, width = ENGDUINO_WIDTH, height = ENGDUINO_HEIGHT) {
    cube(size = [length,width,height]);
    translate([0, width-15, 0]) {
        rotate(a = [0, 0, 45]) {
            cube(size = [15, 20, height]);
        }
    }
} 

module text_ucl() {
    translate([5,2,TEXT_HEIGHT])
    // correctly align text to face right way
    rotate(a = [0,180,-90])
        linear_extrude(height = TEXT_HEIGHT)
            text("UCL Engineering", size = TEXT_SIZE, halign = "left", valign = "center", $fn = 16);
}

// the block to be inclined.
module stand_base(incline, length, width, height) {
    difference() {
        rotate(a = incline, v=[0,1,0]){
            difference() {
                cube (size = [length, width, height]);
                translate([(length - ENGDUINO_LENGTH), (BASE_WIDTH - ENGDUINO_WIDTH) / 2, 0]) {
                    engduino_fill();
                }
                
                text_ucl();
            }
        }
        attachment_point(-7,5,-BASE_HEIGHT);
        attachment_point(-7,BASE_WIDTH - 5,-BASE_HEIGHT);
    }
}

module stand(incline = STAND_INCLINE, width = BASE_WIDTH, height = STAND_HEIGHT, length = STAND_LENGTH) {
    // using the dummy element to remove extrusions from base.
    difference() {
        stand_base(incline, length,width,height);        
        translate([-BASE_LENGTH, 0, -BASE_HEIGHT])
            dummy_element(length,width,height);
   }
}


// Call the components.
translate(v = [0, BASE_WIDTH + 10, 0] )
    base();
rotate([0,180-STAND_INCLINE,0])
translate(v = [0, 0, STAND_HEIGHT]){
        stand();
}