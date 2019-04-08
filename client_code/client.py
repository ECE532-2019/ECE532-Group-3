#!/usr/bin/python3

# Load up them Libs
import socket
import random
import sys
import os
from PyQt4 import QtGui
from PyQt4 import QtCore
import functools
import numpy as np
import random as rd
import matplotlib
matplotlib.use("Qt4Agg")
from matplotlib.figure import Figure
from matplotlib.animation import TimedAnimation
from matplotlib.lines import Line2D
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
import time
import threading

from playsound import playsound

# Global Variables
turrent_ip = '127.0.0.1'
turrent_port = 44000
target_ctl_info = [0, 0, 0]     # x and y aiming direction and state of trigger
driver_ip = '192.168.2.153'
driver_port = 44400
driver_position = [0, 0]        # x and y driving direction
turret_ctl_info = [0, 0]
frame_size = 128*128            # number of pixels in frame

driver_frame_server_address = '192.168.0.16'
driver_frame_server_port = 44400

# Frame Buffers
driver_frame_buffer = np.zeros(1)


## To Do - Andrew - Write a function to convert a desired servo angle to data to be sent to FPGA
servoa_mapping = {'a':10000}
def convert_servo_angle(servo, angle):
    
    value = 10000
    
    if(servo is 'steer'):
        print("Steer")
    elif(servo is 'pitch'):
        print("pitch")
    elif(servo is 'yaw'):
        print('yaw')

    return value

# Network Helper Function
def send_udp_packet(data, dest_ip, dest_port):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(data, (dest_ip, dest_port))
   
#######################################DRIVER MANAGEMETT#################
## Manage Steering
def send_steering_command(direction):
    
    if(direction == 'left' and driver_position[0] > -3):
        driver_position[0] -= 1

    elif(direction == 'right' and driver_position[0] < 3):
        driver_position[0] += 1

    print("updating steering direction to: " + str(driver_position[0]))

    #Determine Value to Send to Servo
    if(driver_position[0] == -2):
        data = bytes([0x01, 0x07, 0xD0])                         # 2000 us
        send_udp_packet(data, driver_ip, driver_port)

    elif(driver_position[0] == -1):
        data = bytes([0x01, 0x06, 0xEF])                         # 1775 us
        send_udp_packet(data, driver_ip, driver_port)

    elif(driver_position[0] == 0):      
        data = bytes([0x01, 0x06, 0x0E])                         # 1550 us
        send_udp_packet(data, driver_ip, driver_port)

    elif(driver_position[0] == 1):
        data = bytes([0x01, 0x05, 0x2D])                         # 1325 us
        send_udp_packet(data, driver_ip, driver_port)

    elif(driver_position[0] == 2):
        data = bytes([0x01, 0x04, 0x4C])                         # 1100 us
        send_udp_packet(data, driver_ip, driver_port)

    return


## Manage Speed Control
def send_speed_control_command(direction):

    if(direction == 'backward' and driver_position[1] > -3):
        driver_position[1] -= 1
    
    elif(direction == 'forward' and driver_position[1] < 3):
        driver_position[1] += 1

    print("updating speed direction to: " + str(driver_position[1]))

    if(driver_position[1] == -2):
        data = bytes([0x05, 0x00, 0x00])                        # 0 usec -> direction
        send_udp_packet(data, driver_ip, driver_port)
        data = bytes([0x06, 0x27, 0x10])                        #10000 usec -> speed
        send_udp_packet(data, driver_ip, driver_port)

    elif(driver_position[1] == -1):
        data = bytes([0x05, 0x00, 0x00])                        # 0 usec -> direction
        send_udp_packet(data, driver_ip, driver_port)
        data = bytes([0x06, 0x13, 0x88])                        #5000 usec -> speed
        send_udp_packet(data, driver_ip, driver_port)

    elif(driver_position[1] == 0):
        data = bytes([0x05, 0x00, 0x00])                        # 0 usec -> direction
        send_udp_packet(data, driver_ip, driver_port)
        data = bytes([0x06, 0x00, 0x00])                        #0 usec -> speed
        send_udp_packet(data, driver_ip, driver_port)
     
    elif(driver_position[1] == 1):
        data = bytes([0x05, 0x4E, 0x20])                        #20000 usec -> direction
        send_udp_packet(data, driver_ip, driver_port)
        data = bytes([0x06, 0x13, 0x88])                        #5000 usec -> speed
        send_udp_packet(data, driver_ip, driver_port)

    elif(driver_position[1] == 1):
        data = bytes([0x05, 0x4E, 0x20])                        # 0 usec -> direction
        send_udp_packet(data, driver_ip, driver_port)
        data = bytes([0x06, 0x27, 0x10])                        #5000 usec -> speed
        send_udp_packet(data, driver_ip, driver_port)

    return

#############################################################################
## Manage Turrent Pitch
def send_pitch_control_command(direction):

    if(direction == 'down' and turret_ctl_info[0] > -3):
        turret_ctl_info[0] -= 1
    
    elif(direction == 'up' and turret_ctl_info[0] < 3):
        turret_ctl_info[0] += 1

    print("updating pitch direction to: " + str(turret_ctl_info[1]))

    if(turret_ctl_info[0] == -2):
        data = bytes([0x02, 0x07, 0xD0])                         # 2000 us
        send_udp_packet(data, driver_ip, driver_port)

    elif(turret_ctl_info[0] == -1):
        data = bytes([0x02, 0x07, 0x3A])                         # 1850 us
        send_udp_packet(data, driver_ip, driver_port)

    elif(turret_ctl_info[0] == 0):      
        data = bytes([0x02, 0x06, 0xA4])                         # 1700 us
        send_udp_packet(data, driver_ip, driver_port)

    elif(turret_ctl_info[0] == 1):
        data = bytes([0x02, 0x06, 0x0E])                         # 1550 us
        send_udp_packet(data, driver_ip, driver_port)

    elif(turret_ctl_info[0] == 2):
        data = bytes([0x02, 0x05, 0x78])                         # 1400 us
        send_udp_packet(data, driver_ip, driver_port)

    return

## Manage Turrent Yaw
def send_yaw_control_command(direction):

    if(direction == 'left' and turret_ctl_info[1] > -3):
        turret_ctl_info[1] -= 1
    
    elif(direction == 'right' and turret_ctl_info[1] < 3):
        turret_ctl_info[1] += 1

    print("updating pitch direction to: " + str(turret_ctl_info[1]))

    if(turret_ctl_info[1] == -2):
        data = bytes([0x03, 0x07, 0xD0])                         # 2000 us
        send_udp_packet(data, driver_ip, driver_port)

    elif(turret_ctl_info[1] == -1):
        data = bytes([0x03, 0x06, 0xD6])                         # 1750 us
        send_udp_packet(data, driver_ip, driver_port)

    elif(turret_ctl_info[1] == 0):      
        data = bytes([0x03, 0x05, 0xDC])                         # 1500 us
        send_udp_packet(data, driver_ip, driver_port)

    elif(turret_ctl_info[1] == 1):
        data = bytes([0x03, 0x04, 0xE2])                         # 1250 us
        send_udp_packet(data, driver_ip, driver_port)

    elif(turret_ctl_info[1] == 2):
        data = bytes([0x03, 0x03, 0xE8])                         # 1000 us
        send_udp_packet(data, driver_ip, driver_port)

    return

## Manage Firing Command
def send_fire_command(status):

    if(status == 'on'):
        data = bytes([0x04, 0x07, 0xD0])                        # 2000 us
        send_udp_packet(data, driver_ip, driver_port)

    elif(status == 'off'):
        data = bytes([0x04, 0x03, 0xE8])
        send_udp_packet(data, driver_ip, driver_port)

    return

#############################################################################




# Process to listen for frames on a given port and append to buffer
def wait_for_frames(frame_buffer, port, server_ip):

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    server = (server_ip, port)
    sock.connect(server)

    while(1):
        data, server_addr = sock.recvfrom(1024)
        data_str = data.decode("ascii")
        data_matrix = np.array(data_str, "c").view(np.uint8)
        
        # alignment might be a problem
        frame_buffer.append(data_matrix)
        print('received a frame from ' + str(port))

    return
    


# GUI window
def setCustomSize(x, width, height):
    sizePolicy = QtGui.QSizePolicy(QtGui.QSizePolicy.Fixed, QtGui.QSizePolicy.Fixed)
    sizePolicy.setHorizontalStretch(0)
    sizePolicy.setVerticalStretch(0)
    sizePolicy.setHeightForWidth(x.sizePolicy().hasHeightForWidth())
    x.setSizePolicy(sizePolicy)
    x.setMinimumSize(QtCore.QSize(width, height))
    x.setMaximumSize(QtCore.QSize(width, height))

''''''

class CustomMainWindow(QtGui.QMainWindow):

    def __init__(self):

        super(CustomMainWindow, self).__init__()

        # Define the geometry of the main window
        self.setGeometry(300, 300, 1400, 800)
        self.setWindowTitle("FPGA Client")

        # Create FRAME_A
        self.FRAME_A = QtGui.QFrame(self)
        self.FRAME_A.setStyleSheet("QWidget { background-color: %s }" % QtGui.QColor(210,210,235,255).name())
        self.LAYOUT_A = QtGui.QGridLayout()
        self.FRAME_A.setLayout(self.LAYOUT_A)
        self.setCentralWidget(self.FRAME_A)

        # Place the driver start stream button
        self.driverStreamBtn = QtGui.QPushButton(text = 'Start Driver Stream')
        setCustomSize(self.driverStreamBtn, 200, 50)
        self.driverStreamBtn.clicked.connect(self.zoomBtnAction)
        self.LAYOUT_A.addWidget(self.driverStreamBtn, *(1,1))

        # Place the aimer start stream button
        self.targetingStreamBtn = QtGui.QPushButton(text = 'Start Targeting Stream')
        setCustomSize(self.targetingStreamBtn, 200, 50)
        self.targetingStreamBtn.clicked.connect(self.zoomBtnAction)
        self.LAYOUT_A.addWidget(self.targetingStreamBtn, *(1,2))

        # Place the Driver Window
        self.myFig = CustomFigCanvas()
        self.myFig.ax1.set_title("Gunner View")
        self.LAYOUT_A.addWidget(self.myFig, *(0,1))

		# Place the Turret Control Window
        self.myFig2 = CustomFigCanvas()
        self.myFig2.ax1.set_title("Target View")
        self.LAYOUT_A.addWidget(self.myFig2, *(0,2))
        

        # Add the callbackfunc to ..
        myDriverLoop = threading.Thread(name = 'myDriverLoop', target = DriverSendLoop, daemon = True, args = (self.addDriverData_callbackFunc,))
        myTargetLoop = threading.Thread(name = 'myTargetLoop', target = TargetSendLoop, daemon = True, args = (self.addTargetData_callbackFunc,))
        myTargetLoop.start()
        myDriverLoop.start()

        self.show()

	# Handle Key Presses Here
    def keyPressEvent(self, event):
        self.code_of_last_pressed_key = event.key()

        # WASD -> Steering
        if(event.key() == 87):
            print("Forward")
            send_speed_control_command('forward')

        elif(event.key() == 83):
            print("Backward")
            send_speed_control_command('backward')

        elif(event.key() == 65):
            print("Left")
            send_steering_command('left')

        elif(event.key() == 68):
            print("Right")
            send_steering_command('right')

        # IJKL -> Turret Aiming
        elif(event.key() == 73):
            print("Pitch Up")
            send_pitch_control_command('up')        

        elif(event.key() == 74):
            print("Yaw Left")
            send_yaw_control_command('left')

        elif(event.key() == 75):
            print("Pitch Down")
            send_pitch_control_command('down')        

        elif(event.key() == 76):
            print("Yaw Right")
            send_yaw_control_command('right')


        # Numbers -> Target Choice Selection
        elif(event.key() ==  49):
            print("Targeting 1")
        elif(event.key() ==  50):
            print("Targeting 2")
        elif(event.key() ==  51):
            print("Targeting 3")
        elif(event.key() ==  52):
            print("Targeting 4")
        elif(event.key() ==  53):
            print("Targeting 5")
        elif(event.key() ==  54):
            print("Targeting 6")
        elif(event.key() ==  55):
            print("Targeting 7")
        elif(event.key() ==  56):
            print("Targeting 8")
        elif(event.key() ==  57):
            print("Targeting 9")


        # ?/ -> Fire
        elif(event.key() == 47):
            print("Firing Nerf Gun")
            send_fire_command('on')

        elif(event.key() == 46):
            print("Cease Firing")
            send_fire_command('off')

        # Easter Egg
        elif(event.key() == 77):
            print("Playing the Trumpets")
            playsound('invisible.mp3')

        print(event.key())
        self.update()
 
    def keyReleaseEvent(self, event):
        pass


    ''''''

    def zoomBtnAction(self):
        print("zoom in")
        self.myFig.zoomIn(5)

    ''''''

    def addTargetData_callbackFunc(self, value):
        # print("Add data: " + str(value))
        self.myFig.addData(value)

    def addDriverData_callbackFunc(self, value):
        # print("Add data: " + str(value))
        self.myFig2.addData(value)


''' End Class '''


class CustomFigCanvas(FigureCanvas, TimedAnimation):

    def __init__(self):

        self.addedData = []
        print(matplotlib.__version__)

        # The data
        self.xlim = 640
        self.ylim = 480
        self.data = np.zeros((self.ylim, self.xlim))

        # FROM EXAMPLE
        self.n = np.linspace(0, self.xlim - 1, self.xlim)

        # The window
        self.fig = Figure(figsize=(5,5), dpi=200)
        self.ax1 = self.fig.add_subplot(111)
        self.im = self.ax1.imshow(self.data, animated=True, vmin=0, vmax=255)

        # Set the Scale and Lables for Axes
        self.ax1.set_xlim(0, self.xlim - 1)
        self.ax1.set_ylim(0, self.ylim - 1)


        FigureCanvas.__init__(self, self.fig)
        TimedAnimation.__init__(self, self.fig, interval = 50, blit = True)

    def new_frame_seq(self):
        return iter(range(self.n.size))

    def _init_draw(self):
         self.im.set_data(np.zeros((self.ylim, self.xlim)))

    # Appened to the addedData queue
    def addData(self, value):
        self.addedData.append(value)

    def zoomIn(self, value):
        bottom = self.ax1.get_ylim()[0]
        top = self.ax1.get_ylim()[1]
        bottom += value
        top -= value
        self.ax1.set_ylim(bottom,top)
        self.draw()


    def _step(self, *args):
        # Extends the _step() method for the TimedAnimation class.
        try:
            TimedAnimation._step(self, *args)
        except Exception as e:
            self.abc += 1
            print(str(self.abc))
            TimedAnimation._stop(self)
            pass

    # Pop off the addedData queue into the frame
    def _draw_frame(self, framedata):
        margin = 2

        while(len(self.addedData) > 0):
            self.im.set_data(self.addedData[0])
            del(self.addedData[0])
            self._drawn_artists = [self.im]


''' End Class '''


# You need to setup a signal slot mechanism, to 
# send data to your GUI in a thread-safe way.
# Believe me, if you don't do this right, things
# go very very wrong..
class Communicate(QtCore.QObject):
    driver_signal = QtCore.pyqtSignal(np.ndarray)
    target_signal = QtCore.pyqtSignal(np.ndarray)

''' End Class '''


# Send from this thread to the addedData queue
def DriverSendLoop(addData_callbackFunc):
    # Setup the signal-slot mechanism.
    mySrc = Communicate()
    mySrc.driver_signal.connect(addData_callbackFunc)

    # Simulate some data -> New Data is Gathered Here
    # Go the frame buffer, see if there is a new frame, if yes push it into the "to be rendered queue"
    # For now this just generates a random frame as a proof of concept

    while(True):
        if( (len(driver_frame_buffer) - 1) % frame_size == 0):
            print("Driver had a full frame")

            new_data = driver_frame_buffer[:frame_size]
            del driver_frame_buffer[:frame_size]            # Pop the driver frame from the queue -> probably slow implementation
            new_data = new_data.reshape((128,128))          # From Will's exampel
            #rint = random.randint(1,100)
            #time.sleep(0.01)
            mySrc.driver_signal.emit(new_data) # <- Here you emit a signal!
    ###
###

## TO DO -> Make this match driver
def TargetSendLoop(addData_callbackFunc):
    # Setup the signal-slot mechanism.
    mySrc = Communicate()
    mySrc.target_signal.connect(addData_callbackFunc)

    # Simulate some data
    # Go to the Frame Buffer and Update the Data
    # For now this just generates a random frame as a proof of concept

    while(True):
        new_data = np.zeros((480, 640))
        rint = random.randint(1,100)
        nrows = len(new_data)
        ncols = len(new_data[0])
        for i in range(nrows):
            for j in range(ncols):
                new_data[i][j] = float(rint)
        #time.sleep(0.05)
        mySrc.target_signal.emit(new_data) # <- Here you emit a signal!
    ###
###


if __name__== '__main__':

    ## TO DO - Andrew Spool a pair of threads to listen for inbound UDP Packets from FPGAs
    #########  Have the threads store inbound frames into a buffer -> GUI will read from that buffer
    driver_receiver = threading.Thread(target=wait_for_frames, args=[driver_frame_buffer, driver_frame_server_port, driver_frame_server_address])
    #turret_receiver = threading.Thread(target=wait_for_frames, args=[turret_frame_buffer, turret_frame_server_port, turrent_frame_server_address])  

    app = QtGui.QApplication(sys.argv)
    QtGui.QApplication.setStyle(QtGui.QStyleFactory.create('Plastique'))
    myGUI = CustomMainWindow()

    sys.exit(app.exec_())

''''''
