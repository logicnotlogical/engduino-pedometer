STAND_INCLINE = 240;
STAND_LENGTH = 120;
STAND_HEIGHT = 10;

BASE_WIDTH = 80;
BASE_LENGTH = 75;
BASE_HEIGHT = 2;

DUMMY_HEIGHT = 10;

ENGDUINO_WIDTH = 60;
ENGDUINO_LENGTH = 80;
ENGDUINO_HEIGHT = 5;

TEXT_HEIGHT = 2;
TEXT_FONT = "Lato";
TEXT_SIZE = 7;

module base(length = BASE_LENGTH, width = BASE_WIDTH, height = BASE_HEIGHT) {
    cube (size = [length, width, height]);
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
    translate([-15, width-15, 0]) {
        cube(size = [15, 15, height]);
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
    rotate(a = incline, v=[0,1,0]){
        difference() {
            cube (size = [length, width, height]);
            translate([(length - ENGDUINO_LENGTH), (BASE_WIDTH - ENGDUINO_WIDTH) / 2, 0]) {
                engduino_fill();
            }
            
            text_ucl();
        }
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
base();
translate(v = [BASE_LENGTH, 0, BASE_HEIGHT]){
        stand();
}