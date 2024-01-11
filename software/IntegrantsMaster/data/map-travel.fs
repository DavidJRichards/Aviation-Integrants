\ **************************************************** 
\ moving map travel test
\ move map location around a circular path
\ heading adjusted to direction of travel
\ constants:
\ ab absolute value of map centre 
\ cd radius of circle travelled
\ dr degrees to radians conversion factor 360/pi
\ values:
\ lp step value of loop in degrees/10
\ ld loop delay value in mS
\ change value:
\ 10 to lp 
\ 1 to ld
\ disable resolver outputs:
\ ' vdummy to setNtoS
\ ' vdummy to setAbsolute
\ ' vdummy to setHeading 
\ start run using travel word, stops with key? press
\ **************************************************** 
DEFINED? --map-travel [if] forget --map-travel [then]
create --map-travel
57.29577951e fconstant dr
35e fconstant cd
705e fconstant ab
1 value lp
1 value ld
: vsetHeading -1e f* 90e  f+  Wheading putmenuidx ;
: vsetNtoS cd f* 90e f+ Wntos putmenuidx ;
: vsetAbsolute cd f* ab f+ Wabsolute putmenuidx ;
: vdummy f. ;
defer setHeading
defer setNtoS
defer setAbsolute
' vsetAbsolute is setAbsolute
' vsetHeading is setHeading
' vsetNtoS is setNtoS
: esc key? if key 27 = else 0 then ;
: travel 4501 900 do I dup S>F 10e f/ f. dup S>F 10e f/ setHeading S>F 10e f/ dr F/ fsincos setNtoS setAbsolute cr ld ms esc if leave then lp  +loop ; 

