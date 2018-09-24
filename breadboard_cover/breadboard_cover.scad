
//Breadboard itself (no feet) is 82mm x 55mm x 10mm

printTolerance = 0.7;

boardLength = 82;
boardWidth = 55;
boardHeight = 10;


//outer shell
coverThickness = 1.5;
coverLength = boardLength + coverThickness*2 + printTolerance;
coverWidth = boardWidth + coverThickness*2 + printTolerance;
coverHeight = 35;

difference(){
    cube([coverLength, coverWidth, coverHeight]);
    
    translate([coverThickness, coverThickness,coverThickness])cube([boardLength + printTolerance, boardWidth + printTolerance, coverHeight + coverThickness + 10]);
    holeFeet();
    usbHole();
    ccs811MountHole();
    bme680MountHole();
};


//lid for the sensors
lidThickness = coverThickness;
lidLength = coverLength + lidThickness*2 + printTolerance;
lidWidth = coverWidth + lidThickness*2 + printTolerance;
lidHeight = 15;

//to see how the lid stacks inside the box, use this translation
//translate([-(coverThickness+printTolerance/2), -(coverThickness+printTolerance/2), -(coverThickness+printTolerance/2)])


translate([-lidLength-5, -lidWidth-5, 0])difference(){
    cube([lidLength, lidWidth, lidHeight]);
    
    translate([lidThickness, lidThickness, lidThickness])cube([coverLength + printTolerance, coverWidth + printTolerance, lidHeight + lidThickness + 10]);
}





//feet holes for the breadboard
holeWallOffset = coverThickness;        //offset for the wall
holeBlockWidth = coverThickness + 1 + 1;
holeWidth = 4.5;
holeHeight = 6.5;

//remember that this is printed upside down, double check orientation
module holeFeet(){
    //lengthwise holes
    //12-16mm
    translate([12+holeWallOffset,coverWidth-(holeHeight/2),coverHeight-holeHeight])cube([holeBlockWidth, holeWidth ,holeHeight + 1]);

    //66.5-70.5mm
    translate([66.5+holeWallOffset,coverWidth-(holeHeight/2),coverHeight-holeHeight])cube([holeBlockWidth, holeWidth ,holeHeight + 1]);



    //widthwise holes
    //3-7mm
    translate([coverLength-(holeWidth/2), 3+holeWallOffset,coverHeight-holeHeight])cube([holeBlockWidth, holeWidth ,holeHeight + 1]);

    //25.5-29.5mm
    translate([coverLength-(holeWidth/2), 25.5+holeWallOffset,coverHeight-holeHeight])cube([holeBlockWidth, holeWidth ,holeHeight + 1]);

    //48.5mm to 51.5
    translate([coverLength-(holeWidth/2), 48.5+holeWallOffset,coverHeight-holeHeight])cube([holeBlockWidth, holeWidth ,holeHeight + 1]);
}

//USB port
//21-35mm wide, and 10-20mm high
usbHoleOffset = coverThickness;
usbHoleHeight = 8;
usbHoleWidth = 14;
usbBaseHeight = 10;

module usbHole(){
    translate([-1 , (coverWidth/2 - usbHoleWidth/2) , coverHeight-usbHoleHeight-usbBaseHeight])cube([holeBlockWidth, usbHoleWidth, usbHoleHeight]);
}


//Holes for mounting the sensors.
ccs811ScrewHole = 1.8; //holes are 2.54mm
ccs811Width = 21.5;     //has tolerance
ccs811Height = 4;



module ccs811MountHole(){
    translate([28+coverThickness,coverThickness +2,-((coverThickness+4)/2)])cube([ccs811Height, ccs811Width, coverThickness + 4]);
}


bme680ScrewHole = 1.5; //2.286mm
bme680Width = 19.9;
bme680Height = 4;


module bme680MountHole(){
    translate([28+coverThickness,(coverWidth-bme680Width)-coverThickness-2,-((coverThickness+4)/2)])cube([bme680Height, bme680Width, coverThickness + 4]);
}

