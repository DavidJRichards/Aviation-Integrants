0 tftsetup
serial
9600 Serial2.begin

: tt-type tfttype drop ;
: s2-type serial Serial2.write drop ;
: s2-key? ( -- f ) serial Serial2.available 0<> pause ;
: s2-key ( -- ch ) begin s2-key? until 0 >r rp@ 1 serial Serial2.readBytes drop r> ;
: tt-on  ['] tt-type is type      ['] s2-key is key      ['] s2-key? is key? ;
: s2-on  ['] s2-type is type      ['] s2-key is key      ['] s2-key? is key? ;
: s2-off ['] default-type is type ['] default-key is key ['] default-key? is key? ;

s2-on
