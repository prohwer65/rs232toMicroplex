# rs232toMicroplex
Code for the RS232 to Microplex tool



Code taken from http://www.infidigm.net/projects/mpx/intrface/index.html

Updated to account for change of controller, from AT90S2313 to ATtiny2313.


Added features to unclude:
Fade to Black over x secs
Fade to new setting over x secs
Turn all channels off. 





Usage:

Send commands via RS232  38400 Baud, 8bits  No handshaking.
All values are sent as byte values, non-strings. 





Commands:

0xA7 (number of channels).  Initialize the number of Microplex channels between 6 and 48.  Default after powerup is 48.   More channels, the slower the response.

0xBB    Fast Black out.  All channels are set to level 0.  
0xFB  (seconds).   Causes a slow Fade to Black over X seconds.
0xAA (starting channel) (LevelX) [ (Level X+1) (Level X+2) ....].  Adjust the channel(s) to a new level, starting at given starting channel and continuing to the next channel(s).  Levels have an immediate affect. 

0xFA (seconds) (starting channel) (LevelX) [ (Level X+1) (Level X+2) ....].  Fade the channel(s) to a new level, starting at given starting channel and continuing to the next channel(s), over the next X seconds.  


Level values are from 0 to 0x9F (159).
Seconds values can be from 1 to 4 seconds.
Starting Channel values can be from 1 to (number of channels).


examples:
PYTHON:
import serial
ser = serial.Serial("COM5", 38400) 

# set number of channels to 24
command = b'\xA7\x18'       
ser.write(command)


# turn all channels to level 0;
command = b'\xBB'           
ser.write(command)

# Set Channel 15 (0x0F) to level 100% (0x9F)  immediately.
command = b'\xAA\x0F\x9F'
ser.write(command)

# Set Channels 10 thru 14 to level to 25% (0x27) immediately.
command = b'\xAA\x0A\x27\x27\x27\x27\x27'
ser.write(command)

# Set Channels 1 thru 24 to level to 50% (0x4F) immediately.
command = b'\xAA\x01\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F'
ser.write(command)

# Fade all channels to Black (level 0) over 2 secs;
command = b'\xFB\x02'           
ser.write(command)

# Fade levels to 100% (0x9F) on channels 1 thru 24 over 4 secs
command = b'\xFA\x04\x01\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F'
ser.write(command)






DOS is not recommended because ECHO adds a \LF and \CR at the end of the string
@echo off
mode COMx BAUD=38400 PARITY=n DATA=8
echo A718  >   example_dump_1.txt
echo BB    >>  example_dump_1.txt

certutil -decodehex example_dump_1.txt example_dump_1.bin

copy example_dump_1.bin \\.\comx /b
