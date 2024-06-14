import os
import serial
import time
import csv
from datetime import datetime
import matplotlib.pyplot as plt
import matplotlib.animation as animation

### Branch of THE GREAT PUMPKIN PLOTTER ###
### Will F.                             ###

def init_serial(port):
    global ser

    ser = serial.Serial(port=port, baudrate=9600,
                        parity=serial.PARITY_NONE,
                        stopbits=serial.STOPBITS_ONE,
                        bytesize=serial.EIGHTBITS,
                            timeout=0)
    return ser

def writeCSV(file, data):
    with open(file,'a',newline='') as csvfile:
      writer = csv.writer(csvfile, delimiter=',')
      writer.writerow(data)

def init_file(folder, header):
    filePath = "data/" + str(folder)
    date = datetime.now().strftime('%Y-%m-%d-%H-%M-%S')

    if not os.path.exists(filePath):
        os.mkdir(filePath)

    csvFile = filePath + "/" + date + ".csv"

    with open(csvFile,'w',newline='') as csvfile:
      writer = csv.writer(csvfile, delimiter=',')
      writer.writerow(header)

    return csvFile


def animate(i, thrust, current, signal):
    '''
    Main Loop Called by FuncAnimation
    '''
    global buf
    while(ser.in_waiting):
        c = ser.read()
        if(c):
            buf = b''.join([buf, c])

            if buf[-1] == 13: #ends with carriage return
                message = buf.decode()
                message = message.split()
                print(message)
                if len(message) == 11:
                    thrust.pop(0)
                    current.pop(0)
                    signal.pop(0)
                    thrust.append(float(message[2]))
                    current.append(float(message[6]))
                    signal.append(int(message[10]))
                    writeCSV(file, message[0::2])
                buf = b''
    #Update line with new values
    l_thrust.set_ydata(thrust)
    l_current.set_ydata(current)
    l_signal.set_ydata(signal)
    return l_thrust, l_current, l_signal

############### SERIAL PORT VARIABLES ###############
# port = '/dev/cu.usbserial-2'
# port = '/dev/cu.usbserial-0001'
port = '/dev/cu.usbmodem101'
buf = b''

############## PLOT SETTINGS ################
size = 200
fs = 10
fig = plt.figure()
ax_thrust = fig.add_subplot(1,3,1)
plt.ylabel('Thrust (g)')
ax_current = fig.add_subplot(1,3,2)
plt.ylabel("Current (A)")
plt.xlabel("seconds")
ax_signal = fig.add_subplot(1,3,3)
plt.ylabel("PWM Signal")
plt.xlabel("seconds")
xs = [(size - 1 - i) / fs for i in range(size)]
thrust = [0] * size
current = [0] * size
signal = [0] * size
ax_thrust.set_ylim([0, 3500])
ax_current.set_ylim([0, 40])
ax_signal.set_ylim([1100, 2000])
l_thrust, = ax_thrust.plot(xs, thrust, color='b')
l_current, = ax_current.plot(xs, current, color='r')
l_signal, = ax_signal.plot(xs, signal, color='orange')
 
################ Initialize #################
header = ['time', 'thrust', 'voltage', 'current', 'power', 'pwm']
ser = init_serial(port)
ser.write(b'xx')
folder = input('Folder: ')
file = init_file(folder, header)
ser.write(b'aa')

ani = animation.FuncAnimation(fig,
    animate,
    fargs=(thrust, current, signal),
    interval=50, #was 50
    blit=True,
    cache_frame_data=False)
plt.show()
ser.write(b'xx')
ser.close()
