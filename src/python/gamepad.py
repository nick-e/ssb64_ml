import sys
import struct

from PIL import Image
import numpy as np

numButtons = 11
numAnalogs = 8
numInputs = numButtons + numAnalogs
gamepadCompressedSize = 18

def eprint(*args, **kwargs):
	print(*args, file=sys.stderr, **kwargs)

def compress_gamepad(gamepadInput):
	arr = bytearray(gamepadCompressedSize)
	gamepadButtons = gamepadInput[0][0]
	gamepadAnalogs = gamepadInput[1][0]

	arr[0] = 1 if gamepadButtons[7] > 0.5 else 0
	arr[0] <<= 1
	arr[0] |= 1 if gamepadButtons[6] > 0.5 else 0
	arr[0] <<= 1
	arr[0] |= 1 if gamepadButtons[5] > 0.5 else 0
	arr[0] <<= 1
	arr[0] |= 1 if gamepadButtons[4] > 0.5 else 0
	arr[0] <<= 1
	arr[0] |= 1 if gamepadButtons[3] > 0.5 else 0
	arr[0] <<= 1
	arr[0] |= 1 if gamepadButtons[2] > 0.5 else 0
	arr[0] <<= 1
	arr[0] |= 1 if gamepadButtons[1] > 0.5 else 0
	arr[0] <<= 1
	arr[0] |= 1 if gamepadButtons[0] > 0.5 else 0
	arr[1] = 1 if gamepadButtons[10] > 0.5 else 0
	arr[1] <<= 1
	arr[1] |= 1 if gamepadButtons[9] > 0.5 else 0
	arr[1] <<= 1
	arr[1] |= 1 if gamepadButtons[8] > 0.5 else 0

	struct.pack_into('h', arr, 2, max(min(int(float(gamepadAnalogs[0]) * 65535 - 32768), 32767), -32768))
	struct.pack_into('h', arr, 4, max(min(int(float(gamepadAnalogs[1]) * 65535 - 32768), 32767), -32768))
	struct.pack_into('h', arr, 6, max(min(int(float(gamepadAnalogs[2]) * 1023), 1023), 0))
	struct.pack_into('h', arr, 8, max(min(int(float(gamepadAnalogs[3]) * 65535 - 32768), 32767), -32768))
	struct.pack_into('h', arr, 10, max(min(int(float(gamepadAnalogs[4]) * 65535 - 32768), 32767), -32768))
	struct.pack_into('h', arr, 12, max(min(int(float(gamepadAnalogs[5]) * 1023), 1023), 0))
	struct.pack_into('h', arr, 14, -1 if gamepadAnalogs[6] < -0.5 else (1 if gamepadAnalogs[6] > 0.5 else 0))
	struct.pack_into('h', arr, 16, -1 if gamepadAnalogs[7] < -0.5 else (1 if gamepadAnalogs[7] > 0.5 else 0))
	return arr

def decompress_gamepad_binary(binary):
	buttons = np.zeros(numButtons)
	buttons[0] = 1.0 if (binary[0] & 0x01) else 0.0
	buttons[1] = 1.0 if (binary[0] & 0x02) else 0.0
	buttons[2] = 1.0 if (binary[0] & 0x04) else 0.0
	buttons[3] = 1.0 if (binary[0] & 0x08) else 0.0
	buttons[4] = 1.0 if (binary[0] & 0x10) else 0.0
	buttons[5] = 1.0 if (binary[0] & 0x20) else 0.0
	buttons[6] = 1.0 if (binary[0] & 0x40) else 0.0
	buttons[7] = 1.0 if (binary[0] & 0x80) else 0.0
	buttons[8] = 1.0 if (binary[1] & 0x01) else 0.0
	buttons[9] = 1.0 if (binary[1] & 0x02) else 0.0
	buttons[10] = 1.0 if (binary[1] & 0x04) else 0.0

	analogs = np.zeros(numAnalogs)
	analogs[0] = (struct.unpack('h', binary[2:4])[0] + 32768) / 65535
	analogs[1] = (struct.unpack('h', binary[4:6])[0] + 32768) / 65535
	analogs[2] = struct.unpack('h', binary[6:8])[0] / 1023
	analogs[3] = (struct.unpack('h', binary[8:10])[0] + 32768) / 65535
	analogs[4] = (struct.unpack('h', binary[10:12])[0] + 32768) / 65535
	analogs[5] = struct.unpack('h', binary[12:14])[0] / 1023
	analogs[6] = (struct.unpack('h', binary[14:16])[0] + 1) / 2
	analogs[7] = (struct.unpack('h', binary[16:18])[0] + 1) / 2
	return (buttons, analogs)

def get_data(frameWidth, frameHeight):
	rgbBufSize = frameWidth * frameHeight * 3

	videoInput = np.empty((1, rgbBufSize))
	gamepadInput = np.empty((1, numInputs))

	videoInput[0] = np.frombuffer(sys.stdin.buffer.read(rgbBufSize), dtype=np.dtype('uint8'), count=rgbBufSize)
	compressed = sys.stdin.buffer.read(gamepadCompressedSize)
	gamepadInput[0] = decompress_gamepad_binary(compressed)

	return (videoInput, gamepadInput)

def get_gamepad_data():
	compressed = sys.stdin.buffer.read(gamepadCompressedSize)
	return decompress_gamepad_binary(compressed)

def get_image_data(imageWidth, imageHeight):
	size = imageWidth * imageHeight * 3
	video = np.frombuffer(sys.stdin.buffer.read(size), dtype=np.dtype('uint8'), count=size)
	video2 = bytes(video)
	#Image.frombytes('L', (128, 72), video2, 'raw').show()
	#for row in range(0, imageHeight):
	#	eprint('')
	#	for col in range(0, imageWidth):
	#		index = (row * imageWidth + col) * 3
	#		eprint(video2[index], video2[index + 1], video2[index + 2])
	#sys.exit()
	return video;

def get_batch(batchSize, frameWidth, frameHeight):
	rgbBufSize = frameWidth * frameHeight * 3

	videoInputBatch = np.empty((batchSize, rgbBufSize))
	gamepadInputBatch = np.empty((batchSize, numInputs))
	buttonTargetBatch = np.empty((batchSize, numButtons))
	analogTargetBatch = np.empty((batchSize, numAnalogs))

	for batch in range(batchSize):
		videoInputBatch[batch] = np.frombuffer(sys.stdin.buffer.read(rgbBufSize), dtype=np.dtype('uint8'), count=rgbBufSize)
		compressed = sys.stdin.buffer.read(gamepadCompressedSize)
		gamepadInputBatch[batch] = decompress_gamepad_binary(compressed)
		compressed = sys.stdin.buffer.read(gamepadCompressedSize)
		decompressed = decompress_gamepad_binary(compressed)
		buttonTargetBatch[batch] = decompressed[:numButtons]
		analogTargetBatch[batch] = decompressed[numButtons:]
		#eprint(batch, gamepadInputBatch[batch].tolist())

	return (videoInputBatch, gamepadInputBatch, buttonTargetBatch, \
		analogTargetBatch)
