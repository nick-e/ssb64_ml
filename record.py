import numpy as np
import cv2
from PIL import ImageGrab
from PIL import Image
from inputs import get_key
from inputs import get_gamepad
import threading
import datetime
import struct
import os

recording = False

# Super smash bros doesn't need all the buttons on a controller
rightTrigger = False
leftTrigger = False
southButton = False
westButton = False
eastButton = False
northButton = False
rightBumper = False
leftAnalogX = 0
leftAnalogY = 0
rightAnalogX = 0
rightAnalogY = 0
recordButton = False

def listenForGamepad():
	global rightTrigger, southButton, westButton, eastButton, northButton, rightBumper, leftBumper, leftAnalogX, leftAnalogY, rightAnalogX, rightAnalogY, recording
	recordButtonReleased = True
	while True:
		events = get_gamepad()
		for event in events:
			if event.code == "BTN_SOUTH":
				if event.state:
					southButton = True
				else:
					southButton = False
			elif event.code == "BTN_NORTH":
				if event.state:
					northButton = True
				else:
					northButton = False
			elif event.code == "BTN_EAST":
				if event.state:
					eastButton = True
				else:
					eastButton = False
			elif event.code == "BTN_WEST":
				if event.state:
					westButton = True
				else:
					westButton = False
			elif event.code == "BTN_TR":
				if event.state:
					rightBumper = True
				else:
					rightBumper = False
			elif event.code == "ABS_LZ":
				if event.state:
					leftTrigger = True
				else:
					leftTrigger = False
			elif event.code == "ABS_RZ":
				if event.state == 255:
					rightTrigger = True
				else:
					rightTrigger = False
			elif event.code == "ABS_Y":
				if event.state > 3000 or event.state < -3000:
					tmp = (event.state + 32768) / 65535
					leftAnalogY = round(tmp, 1)
				else:
					leftAnalogY = 0.5
			elif event.code == "ABS_X":
				if event.state > 3000 or event.state < -3000:
					tmp = (event.state + 32768) / 65535
					leftAnalogX = round(tmp, 1)
				else:
					leftAnalogX = 0.5
			elif event.code == "ABS_RY":
				if event.state > 3000 or event.state < -3000:
					tmp = (event.state + 32768) / 65535
					rightAnalogY = round(tmp, 1)
				else:
					rightAnalogY = 0
			elif event.code == "ABS_RX":
				if event.state > 3000 or event.state < -3000:
					tmp = (event.state + 32768) / 65535
					rightAnalogX = round(tmp, 1)
				else:
					rightAnalogX = 0.5
			elif event.code == "BTN_START":
				if event.state and recordButtonReleased:
					recordButtonReleased = False
					recording = not recording
					recordButton = True
				else:
					recordButton = False
					recordButtonReleased = True

def createControllerBinary():
	global rightTrigger, leftTrigger, southButton, westButton, eastButton, northButton, rightBumper, leftAnalogX, leftAnalogY, rightAnalogX, rightAnalogY
	# a float is 4 bytes, there are 4 floats for the analog sticks and 1 byte for the buttons
	rax = bytearray(struct.pack('f', rightAnalogX))
	ray = bytearray(struct.pack('f', rightAnalogY))
	lax = bytearray(struct.pack('f', leftAnalogX))
	lay = bytearray(struct.pack('f', leftAnalogY))
	b = bytearray(17)
	b[0] = rightTrigger | (leftTrigger << 1) | (southButton << 2) | (westButton << 3) | (eastButton << 4) | (northButton << 5) | (rightBumper << 6)
	for i in range(4):
		b[1 + i] = rax[i]
		b[5 + i] = ray[i]
		b[9 + i] = lax[i]
		b[13 + i] = lay[i]
	return b

def record():
	current = recording
	videoOutFile = None
	controllerOutFile = None
	fourcc = cv2.VideoWriter_fourcc('M','J','P','G')
	start = None
	prev = None
	lastControllerRecording = 0
	while True:
		now = datetime.datetime.now()

		if current != recording:
			current = recording
			if current:
				start = now
				startStr = start.strftime("%Y-%m-%d %H.%M.%S")
				videoOutFile = cv2.VideoWriter('data/' + startStr + '.avi', fourcc, 15, (960, 540))
				controllerOutFile = open('data/' + startStr + '.cont', 'wb')
				prev = now
				print('Recording started. ' + start.strftime("%Y-%m-%d %H:%M:%S"))
			else:
				videoOutFile.release()
				controllerOutFile.close()
				videoOutFile = None
				dif = now - start
				print('\tRecording stopped. ' + str(dif.seconds) + ' seconds.')
		if current and videoOutFile != None:
			delta = (now - prev).microseconds / 1000
			prev = now
			lastControllerRecording += delta
			print(delta)
			if lastControllerRecording >= 66.667:
				controllerOutFile.write(createControllerBinary())
				lastControllerRecording -= 66.667

			img = ImageGrab.grab().resize((960, 540), Image.BILINEAR)
			img = np.array(img)

			videoOutFile.write(img)

def main():
	gamepadThread = threading.Thread(target = listenForGamepad)
	gamepadThread.daemon = True
	gamepadThread.start()

	record()

if __name__ == '__main__':
	main()

#while(True):
	#img = np.array(ImageGrab.grab())
	#frame = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
	#cv2.imshow('frame',frame)

	#events = get_key()
	#for event in events:
	#	print(event.ev_type, event.code, event.state)

	#if cv2.waitKey(25) & 0xFF == ord('q'):
	#	break

#cv2.destroyAllWindows()
