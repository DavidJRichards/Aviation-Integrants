\ ********************************************************************* 
\ strings management for ESP32forth 
\    Filename:      strings.txt 
\    Date:          20 jun 2021 
\    Updated:       29 jan 2022 
\    File Version:  1.0 
\    MCU:           ESP32-WROOM-32 
\    Forth:         ESP32forth all versions 7.x++ 
\    Copyright:     Marc PETREMANN 
\    Author:        Marc PETREMANN 
\    GNU General Public License 
\ ********************************************************************* 
 
DEFINED? --str [if] forget --str  [then] 
create --str 
 
 
\ compare two strings 
: $= ( addr1 len1 addr2 len2 --- fl) 
    str= 
  ; 
 
\ define a strvar 
: string ( n --- names_strvar ) 
    create 
        dup 
        ,                   \ n is maxlength 
        0 ,                 \ 0 is real length 
        allot 
    does> 
        cell+ cell+ 
        dup cell - @ 
    ; 
 
\ get maxlength of a string 
: maxlen$  ( strvar --- strvar maxlen ) 
    over cell - cell - @ 
    ; 
 
\ store str into strvar 
: $!  ( str strvar --- ) 
    maxlen$                 \ get maxlength of strvar 
    nip rot min             \ keep min length 
    2dup swap cell - !      \ store real length 
    cmove                   \ copy string 
    ; 
  
\ set length of a string to zero 
: 0$! ( addr len -- ) 
    drop 0 swap cell - ! 
  ; 
 
 \ Example: 
 : s1 
     s" this is constant string" ; 
 200 string test 
 s1 test $! 

\ extract n chars right from string 
: right$  ( str1 n --- str2 ) 
    0 max over min >r + r@ - r> 
    ; 
 
\ extract n chars left frop string 
: left$  ( str1 n --- str2 ) 
    0 max min 
    ; 
 
\ extract n chars from pos in string 
: mid$  ( str1 pos len --- str2 ) 
    >r over swap - right$ r> left$ 
    ; 
 
\ append char c to string 
: c+$!  ( c str1 -- ) 
    over >r 
    + c! 
    r> cell - dup @ 1+ swap ! 
    ; 
 
\ work only with strings. Don't use with other arrays 
: input$ ( addr len -- ) 
    over swap maxlen$ nip accept 
    swap cell - ! 
  ; 
