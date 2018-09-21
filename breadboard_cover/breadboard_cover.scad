
//Breadboard itself (no feet) is 82mm x 55mm x 10mm
wallThickness = 2;
printTolerance = 0.8;

boardLength = 82;
boardWidth = 55;
boardHeight = 10;

coverHeight = 50;

//outer shell
difference(){
    cube([boardLength + wallThickness + printTolerance, boardWidth + wallThickness + printTolerance, coverHeight]);
    
    translate([(wallThickness + printTolerance)/2, (wallThickness + printTolerance)/2,-10])cube([boardLength, boardWidth, coverHeight-wallThickness+10]);
    holeFeet();
    usbHole();
    ccs811MountHole();
    bme680MountHole();
};

//feet holes for the breadboard

holeBlockWidth = wallThickness + 1 + 1;
holeWidth = 4.5;
holeHeight = 6.5;



module holeFeet(){
    //lengthwise holes
    //12-16mm
    translate([12,-1,-1])cube([holeBlockWidth, holeWidth ,holeHeight + 1]);

    //66.5-70.5mm
    translate([66.5,-1,-1])cube([holeBlockWidth, holeWidth ,holeHeight + 1]);



    //widthwise holes
    //3-7mm
    translate([boardLength, 3,-1])cube([holeBlockWidth, holeWidth ,holeHeight + 1]);

    //25.5-29.5mm
    translate([boardLength, 26.5,-1])cube([holeBlockWidth, holeWidth ,holeHeight + 1]);

    //48.5mm to 51.5
    translate([boardLength, 49.5,-1])cube([holeBlockWidth, holeWidth ,holeHeight + 1]);
}

//USB port
//21-35mm wide, and 10-20mm high
usbHoleHeight = 10;
usbHoleWidth = 14;
usbBaseHeight = 10;

module usbHole(){
    translate([-1 , 21 , usbBaseHeight])cube([holeBlockWidth, usbHoleWidth, usbHoleHeight]);
}


//Holes for mounting the sensors.
ccs811ScrewHole = 1.8; //holes are 2.54mm
ccs811Width = 22.5;     //has tolerance
ccs811Height = 7;




module ccs811MountHole(){
    translate([30,wallThickness +2,coverHeight-((wallThickness+4)/2)])cube([ccs811Height, ccs811Width, wallThickness + 5]);
}


bme680ScrewHole = 1.5; //2.286mm
bme680Width = 19.9;
bme680Height = 7;


module bme680MountHole(){
    translate([30,boardWidth-bme680Width-2,coverHeight-((wallThickness+4)/2)])cube([bme680Height, bme680Width, wallThickness + 5]);
}

