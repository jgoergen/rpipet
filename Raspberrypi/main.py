import serial
import time

port = serial.Serial("/dev/ttyAMA0", baudrate=115200, timeout=3.0)
first = True

def readlineCR():
    global port

    rv = ""
    
    while True:
        ch = port.read()
        rv += ch
        if ch=='\r' or ch=='':
            return rv.replace("\n", "").replace("\r", "")

def scanFull():
    global port

    deg = 0
    degChange = 10
    
    print("Scan Full Starting")

    port.write("<campos:" + str(deg) + ">\r\n")
    time.sleep(6)

    while deg < 190:
        port.write("<campos:" + str(deg) + ">\r\n")
        port.write("<getdist:0>\r\n")
        print(str(deg) + ": " + readlineCR())
        deg += degChange
        time.sleep(0.6)

    print("Scan Full Complete")

while True:

    if first == True:
        scanFull()
        first = False
    
    rcv = readlineCR()
    print(repr(rcv))


# When system started:
# 	ready|{getVcc()}{line break}

# when battery low:
# 	warning|battery

# when object close:
# 	warning|close

# commands:
# 	<getdist:0>
# 		returns reading from ultrasonic sensor
# 		ok|{distance in centimeters}
# 
# 	<campos:{0-180}>
# 		returns the number you gave it
# 		ok|{0-180}
# 
# 	<dir:{0-4}>
# 		0 - stop
# 		1 - forward
# 		2 - right
# 		3 - backward
# 		4 - left
# 		returns the number you gave it
# 		ok|{0-4}
# 
# 	<bat:0>
# 		returns getVcc() reading of the battery