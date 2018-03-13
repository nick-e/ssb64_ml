import sys
import numpy as np
import cv2
import struct
import os
import tensorflow as tf

import model

def controller_binary_to_numpy(binary):
    arr = np.empty(14)
    arr[0] = 1 if (binary[0] & 0x01) else 0
    arr[1] = 1 if (binary[0] & 0x02) else 0
    arr[2] = 1 if (binary[0] & 0x04) else 0
    arr[3] = 1 if (binary[0] & 0x08) else 0
    arr[4] = 1 if (binary[0] & 0x0f) else 0
    arr[5] = 1 if (binary[0] & 0x10) else 0
    arr[6] = 1 if (binary[0] & 0x20) else 0
    arr[7] = 1 if (binary[0] & 0x40) else 0

    arr[8] = struct.unpack('h', binary[1:3])[0]
    arr[9] = struct.unpack('h', binary[3:5])[0]
    arr[10] = struct.unpack('h', binary[5:7])[0]
    arr[11] = struct.unpack('h', binary[7:9])[0]
    arr[12] = struct.unpack('h', binary[9:11])[0]
    arr[13] = struct.unpack('h', binary[11:13])[0]

    return arr

def get_next_training_batch(batchSize, videoFile, controllerFile, videoBatch, controllerInputBatch, expectedControllerButtonOutputBatch, expectedControllerAnalogOutputBatch):
    # get a batch of controller input
    controllerBinaries = controllerFile.read(17 * batchSize)
    if len(controllerBinaries) != 17 * batchSize:
        return False
    for i in range(batchSize):
        controllerNp = controller_binary_to_numpy(controllerBinaries[i * 17:(i + 1) * 17])
        controllerInputBatch[i][:] = controllerNp

    # get a batch of video input
    for i in range(batchSize):
        ret, frame = videoFile.read()
        frame = frame.flatten()
        videoBatch[i][:] = frame

    # get a batch of expected outputs
    expectedControllerButtonOutputBatch[:batchSize - 1] = controllerInputBatch[1:, :8]
    expectedControllerAnalogOutputBatch[:batchSize - 1] = controllerInputBatch[1:, 8:14]
    tmp = controllerFile.read(17)
    if len(tmp) != 17:
        return False
    tmp = controller_binary_to_numpy(tmp)
    expectedControllerButtonOutputBatch[batchSize - 1] = tmp[:8]
    expectedControllerAnalogOutputBatch[batchSize - 1] = tmp[8:14]
    controllerFile.seek(-17, 1)
    return True

def main():
    if len(sys.argv) != 9:
        return

    print(sys.argv)

    print("Loading model")
    ckpt = sys.argv[1].rstrip(".meta")
    numEpochs = int(sys.argv[2])
    batchSize = int(sys.argv[3])
    imageWidth = int(sys.argv[4])
    imageHeight = int(sys.argv[5])
    numInputChannels = int(sys.argv[6])
    numControllerInputs = int(sys.argv[7])
    trainingDataDir = sys.argv[8]
    sess, videoInputPlaceholder, controllerInputPlaceholder, controllerButtonOutputs, controllerAnalogOutputs, expectedControllerButtonOutputsPlaceholder, expectedControllerAnalogOutputsPlaceholder, optimizer, loss = model.load_model(ckpt, imageWidth, imageHeight, numInputChannels, numControllerInputs, numControllerInputs)
    videoBatch = np.empty([batchSize, imageWidth * imageHeight * numInputChannels])
    controllerInputBatch = np.empty([batchSize, numControllerInputs])
    expectedControllerButtonOutputsBatch = np.empty([batchSize, 8])
    expectedControllerAnalogOutputsBatch = np.empty([batchSize, 6])

    print("Training model")
    for i in range(numEpochs):
        for filename in os.listdir(trainingDataDir):
            if filename.endswith('.avi'):
                prefix = filename.replace('.avi', '')
                print('\t' + prefix)
                videoFile = cv2.VideoCapture(trainingDataDir + "/" + filename)
                controllerFile = open(trainingDataDir + "/" + prefix + '.gamepad', 'rb')
                totalFrames = int(videoFile.get(cv2.CAP_PROP_FRAME_COUNT))
                totalFramesStr = str(totalFrames)
                currentFrame = 0

                while get_next_training_batch(
                    batchSize,
                    videoFile,
                    controllerFile,
                    videoBatch,
                    controllerInputBatch,
                    expectedControllerButtonOutputsBatch,
                    expectedControllerAnalogOutputsBatch):
                    _, v = sess.run(
                        [optimizer, loss],
                        feed_dict = {
                            videoInputPlaceholder: videoBatch,
                            controllerInputPlaceholder: controllerInputBatch,
                            expectedControllerButtonOutputsPlaceholder: expectedControllerButtonOutputsBatch,
                            expectedControllerAnalogOutputsPlaceholder: expectedControllerAnalogOutputsBatch
                        }
                    )
                    #o.write(str(c) + ',' + str(v) + '\n')
                    #c += 1
                    currentFrame += batchSize
                    print('\t\t' + str(currentFrame) + ' / ' + totalFramesStr + ", loss = " + str(v))

                controllerFile.close()
                videoFile.release()

    print("Saving model")
    saver = tf.train.Saver()
    saver.save(sess, ckpt)

    print("Done training")

if __name__ == "__main__":
    main()
