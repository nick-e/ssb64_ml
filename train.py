import numpy as np
import cv2
import os
import tensorflow as tf
import struct

def controllerBinaryToNumpy(binary):
	controllerNp = np.empty(11)
	controllerNp[0] = 1 if (binary[0] & 0x01) else 0
	controllerNp[1] = 1 if (binary[0] & 0x02) else 0
	controllerNp[2] = 1 if (binary[0] & 0x04) else 0
	controllerNp[3] = 1 if (binary[0] & 0x08) else 0
	controllerNp[4] = 1 if (binary[0] & 0x0f) else 0
	controllerNp[5] = 1 if (binary[0] & 0x10) else 0
	controllerNp[6] = 1 if (binary[0] & 0x20) else 0
	controllerNp[7] = struct.unpack('f', binary[1:5])[0]
	controllerNp[8] = struct.unpack('f', binary[5:9])[0]
	controllerNp[9] = struct.unpack('f', binary[9:13])[0]
	controllerNp[10] = struct.unpack('f', binary[13:17])[0]
	return controllerNp

def main():
	video_input_layer = tf.layers.Input(
		shape = (960, 540, 3),
		batch_size = 10)

	# conv1's shape is (960, 540, 32)
	conv1 = tf.layers.conv2d(
		inputs = video_input_layer,
		filters = 32,
		kernel_size = [288, 162],
		padding = "same",
		activation = tf.nn.relu)

	# pool1's shape is (480, 270, 32)
	pool1 = tf.layers.max_pooling2d(
		inputs = conv1,
		pool_size = [2, 2],
		strides = [2, 2])

	# conv2's shape is (480, 270, 64)
	conv2 = tf.layers.conv2d(
		inputs = pool1,
		filters = 64,
		kernel_size = [288, 162],
		padding = "same",
		activation = tf.nn.relu)

	# pool2's shape is (240, 135, 64)
	pool2 = tf.layers.max_pooling2d(
		inputs = conv2,
		pool_size = [2, 2],
		strides = [2, 2])
	pool2_flat = tf.reshape(pool2, [-1, 240 * 135 * 64])

	controller_input_layer = tf.layers.Input(
		shape = (11,),
		batch_size = 10)

	dense_layer_input = tf.concat([pool2_flat, controller_input_layer], 1)

	dense = tf.layers.dense(
		inputs = dense_layer_input,
		units = 1024,
		activation = tf.nn.relu)

	dropout = tf.layers.dropout(
		inputs = dense,
		rate = 0.4,
		training = True)

	logits = tf.layers.dense(
		inputs = dropout,
		units = 11
	)

	# that is 960 * 540 * 3 * 10
	# since it's so huge, I'm not going to put it in the loop where it constantly needs to be reinitialized
	videoFrames = np.empty(15552000)
	for filename in os.listdir('data/'):
		if filename.endswith('.mp4'):
			prefix = filename.replace('.mp4', '')
			print("training " + filename)
			videoFile = cv2.VideoCapture('data/' + filename)
			controllerFile = open('data/' + prefix + '.cont', 'rb')

			while True:
				controllerBinaries = controllerFile.read(170)
				if len(controllerBinaries) != 170:
					break
				controllerInputs = np.empty(110)
				for i in range(10):
					controllerNp = controllerBinaryToNumpy(controllerBinaries[i * 17:(i + 1) * 17])
					controllerInputs[i * 11:(i + 1) * 11] = controllerNp

				for i in range(10):
					ret, frame = videoFile.read()
					frame = frame.flatten()
					videoFrames[i * 1555200:(i + 1) * 1555200] = frame

			controllerFile.close()
			videoFile.release()


if __name__ == '__main__':
	main()
