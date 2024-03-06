# rs232toMicroplex
Code for the RS232 to Microplex tool



Code was original developed and posted at http://www.infidigm.net/projects/mpx/intrface/index.html

Updated code and naming conventions because of the change of controller, from AT90S2313 to ATtiny2313.



Added features to include:
Fade to Black over x halfsecs
Fade to new setting over x halfsecs
Turn all channels off immediately. 





Usage:

Send commands via RS232  38400 Baud, 8bits  No handshaking.
All values are sent as byte values, non-strings. 





Commands:

0xA7 _(number of channels)_.  
- Initialize the number of Microplex channels between 6 and 48.  Default after powerup is 48.   More channels, the slower the response.

0xBB    
- Fast Black out.  All channels are set to level 0.  

0xFB _(halfseconds)_.   
- Causes a slow Fade to Black over X half seconds.

0xAA _(starting channel)_ _(LevelX) [ (Level X+1) (Level X+2) ....]_.  
- Adjust the channel(s) to a new level, starting at given starting channel and continuing to the next channel(s).  Levels have an immediate affect. 

0xFA _(halfseconds) (starting channel) (LevelX) [ (Level X+1) (Level X+2) ....]_.  
- Fade the channel(s) to a new level, starting at given starting channel and continuing to the next channel(s), over the next X half seconds.  


Arguments:
- _Level_ values are from 0 to 0x9F (159).

- _HalfSeconds_ values can be from 1 to 10 Halfseconds.

- _Starting Channel_ values can be from 1 to _(number of channels)_.


## EXAMPLES in PYTHON:
```python
import serial
ser = serial.Serial("COM5", 38400) 
```

### Set number of channels to 24
```python
command = b'\xA7\x18'       
ser.write(command)
```



### Turn all channels to level 0; immediately.
```python
command = b'\xBB'           
ser.write(command)
```


### Set Channel 15 (0x0F) to level 100% (0x9F)  immediately.
```python
command = b'\xAA\x0F\x9F'
ser.write(command)
```


### Set Channels 10 thru 14 to level to 25% (0x27) immediately.
```python
command = b'\xAA\x0A\x27\x27\x27\x27\x27'
ser.write(command)
```


### Set Channels 1 thru 24 to level to 50% (0x4F) immediately.
```python
command = b'\xAA\x01\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F\x4F'
ser.write(command)
```


### Fade all channels to Black (level 0) over 2 secs;
```python
command = b'\xFB\x02'           
ser.write(command)
```


### Fade levels to 100% (0x9F) on channels 1 thru 24 over 4 secs
```python
command = b'\xFA\x04\x01\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F\x9F'
ser.write(command)
```








Markdown editted with //dillinger.io/
