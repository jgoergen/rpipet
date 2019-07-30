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

    deg = 50
    degChange = 10
    
    print("Scan Full Starting")

    port.write("<campos:" + str(deg) + ">\r\n")
    time.sleep(2)

    while deg < 130:
        port.write("<campos:" + str(deg) + ">\r\n")
        readlineCR()
        readlineCR()
        port.write("<getdist:0>\r\n")
        print("Distance: " + str(deg) + " Degs: " + readlineCR())
        deg += degChange
        time.sleep(1)

    print("Scan Full Complete")

while True:

    print("Directional Tests Starting")

    print("Forward")
    port.write("<dir:1>\r\n")
    print(readlineCR())
    time.sleep(1)
    port.write("<dir:3>\r\n")
    print(readlineCR())
    time.sleep(1)

    print("Backward")
    port.write("<dir:2>\r\n")
    print(readlineCR())
    time.sleep(1)
    port.write("<dir:3>\r\n")
    print(readlineCR())
    time.sleep(1)

    print("Right")
    port.write("<dir:4>\r\n")
    print(readlineCR())
    port.write("<dir:1>\r\n")
    print(readlineCR())
    time.sleep(1)
    port.write("<dir:3>\r\n")
    print(readlineCR())
    time.sleep(1)

    print("Left")
    port.write("<dir:5>\r\n")
    print(readlineCR())
    port.write("<dir:1>\r\n")
    print(readlineCR())
    time.sleep(1)
    port.write("<dir:3>\r\n")
    print(readlineCR())
    time.sleep(1)

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