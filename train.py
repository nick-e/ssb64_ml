import numpy as np
import cv2
import os
import tensorflow as tf

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


	#for filename in os.listdir('/data'):
	#	if filename.endswith('.avi'):
	#		prefix = filename.replace('.avi', '')
	#		videoFile = cv2.VideoCapture(filename)
	#		controllerFile = open('/data' + filename.replace('.avi', '') + '.cont', 'rb')


if __name__ == '__main__':
	main()
