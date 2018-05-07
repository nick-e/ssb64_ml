import sys
import struct

import numpy as np

numGamepadButtons = 11
numGamepadAnalogs = 8
numGamepadInputs = numGamepadButtons + numGamepadAnalogs
gamepadCompressedSize = 18

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def compress_gamepad(gamepadInput):
    arr = bytearray(gamepadCompressedSize)
    gamepadButtons = gamepadInput[0][0]
    gamepadAnalogs = gamepadInput[1][0]
    arr[0] = int(gamepadButtons[0]) | (int(gamepadButtons[1]) << 1) \
        | (int(gamepadButtons[2]) << 2) | (int(gamepadButtons[3]) << 3) \
        | (int(gamepadButtons[4]) << 4) | (int(gamepadButtons[5]) << 5) \
        | (int(gamepadButtons[6]) << 6) | (int(gamepadButtons[7]) << 7)
    arr[1] = int(gamepadButtons[8]) | (int(gamepadButtons[9]) << 1) \
        | (int(gamepadButtons[10]) << 2)
    struct.pack_into('h', arr, 2, min(max(int(float(gamepadAnalogs[0]) * 65535 - 32768), 32767), -32768))
    struct.pack_into('h', arr, 3, min(max(int(float(gamepadAnalogs[1]) * 65535 - 32768), 32767), -32768))
    struct.pack_into('h', arr, 4, min(max(int(float(gamepadAnalogs[2]) * 1023), 1023), -1024))
    struct.pack_into('h', arr, 5, min(max(int(float(gamepadAnalogs[3]) * 65535 - 32768), 32767), -32768))
    struct.pack_into('h', arr, 6, min(max(int(float(gamepadAnalogs[4]) * 65535 - 32768), 32767), -32768))
    struct.pack_into('h', arr, 7, min(max(int(float(gamepadAnalogs[5]) * 1023), 1023), -1024))
    struct.pack_into('h', arr, 8, min(max(int(float(gamepadAnalogs[6]) * 2 - 1), 1), -1))
    struct.pack_into('h', arr, 9, min(max(int(float(gamepadAnalogs[7]) * 2 - 1), 1), -1))
    return arr

def decompress_gamepad_binary(binary):
    arr = np.zeros(numGamepadInputs)
    arr[0] = 1.0 if (binary[0] & 0x01) else 0.0
    arr[1] = 1.0 if (binary[0] & 0x02) else 0.0
    arr[2] = 1.0 if (binary[0] & 0x04) else 0.0
    arr[3] = 1.0 if (binary[0] & 0x08) else 0.0
    arr[5] = 1.0 if (binary[0] & 0x10) else 0.0
    arr[6] = 1.0 if (binary[0] & 0x20) else 0.0
    arr[7] = 1.0 if (binary[0] & 0x40) else 0.0
    arr[7] = 1.0 if (binary[0] & 0x80) else 0.0
    arr[8] = 1.0 if (binary[1] & 0x01) else 0.0
    arr[9] = 1.0 if (binary[1] & 0x02) else 0.0
    arr[10] = 1.0 if (binary[1] & 0x04) else 0.0

    arr[11] = (struct.unpack('h', binary[2:4])[0] + 32768) / 65535
    arr[12] = (struct.unpack('h', binary[4:6])[0] + 32768) / 65535
    arr[13] = struct.unpack('h', binary[6:8])[0] / 1023
    arr[14] = (struct.unpack('h', binary[8:10])[0] + 32768) / 65535
    arr[15] = (struct.unpack('h', binary[10:12])[0] + 32768) / 65535
    arr[16] = struct.unpack('h', binary[12:14])[0] / 1023
    arr[17] = (struct.unpack('h', binary[14:16])[0] + 1) / 2
    arr[18] = (struct.unpack('h', binary[16:18])[0] + 1) / 2
    return arr

def get_data(frameWidth, frameHeight):
    rgbBufSize = frameWidth * frameHeight * 3

    videoInput = np.empty((1, rgbBufSize))
    gamepadInput = np.empty((1, numGamepadInputs))

    videoInput[0] = np.frombuffer(sys.stdin.buffer.read(rgbBufSize), dtype=np.dtype('uint8'), count=rgbBufSize)
    compressed = sys.stdin.buffer.read(gamepadCompressedSize)
    decompressed = decompress_gamepad_binary(compressed)
    gamepadInput[0] = np.frombuffer(decompressed, dtype=np.dtype('int16'), count=numGamepadInputs)

    return (videoInput, gamepadInput)

def get_batch(batchSize, frameWidth, frameHeight):
    rgbBufSize = frameWidth * frameHeight * 3

    videoInputBatch = np.empty((batchSize, rgbBufSize))
    gamepadInputBatch = np.empty((batchSize, numGamepadInputs))
    gamepadButtonLabelBatch = np.empty((batchSize, numGamepadButtons))
    gamepadAnalogLabelBatch = np.empty((batchSize, numGamepadAnalogs))

    for batch in range(batchSize):
        videoInputBatch[batch] = np.frombuffer(sys.stdin.buffer.read(rgbBufSize), dtype=np.dtype('uint8'), count=rgbBufSize)
        compressed = sys.stdin.buffer.read(gamepadCompressedSize)
        gamepadInputBatch[batch] = decompress_gamepad_binary(compressed)
        compressed = sys.stdin.buffer.read(gamepadCompressedSize)
        decompressed = decompress_gamepad_binary(compressed)
        gamepadButtonLabelBatch[batch] = decompressed[:numGamepadButtons]
        gamepadAnalogLabelBatch[batch] = decompressed[numGamepadButtons:]

    return (videoInputBatch, gamepadInputBatch, gamepadButtonLabelBatch, \
        gamepadAnalogLabelBatch)
