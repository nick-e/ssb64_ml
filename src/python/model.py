import numpy as np
import tensorflow as tf

import gamepad

def create_lstm(inputs, outputSize, time):
	inputsShape = inputs.get_shape().as_list()
	cell = tf.nn.rnn_cell.BasicLSTMCell(outputSize, forget_bias=1.0, state_is_tuple=True)
	initialState = cell.zero_state(time, tf.float32)
	state = cell.zero_state(1, tf.float32)
	outputs, states = tf.nn.static_rnn(cell, [inputs], initial_state=initialState)
	return outputs[-1], states[-1], initialState, state

def create_model(imageWidth, imageHeight, batchSize):
	videoInput = tf.placeholder(tf.float32, shape=[batchSize, imageWidth * imageHeight * 3])
	gamepadInput = tf.placeholder(tf.float32, shape=[batchSize, gamepad.numInputs])
	analogTarget = tf.placeholder(tf.float32, shape=[batchSize, gamepad.numAnalogs])
	buttonTarget = tf.placeholder(tf.float32, shape=[batchSize, gamepad.numButtons])

	conv1 = tf.layers.conv2d(
		inputs=tf.reshape(videoInput, [batchSize, imageWidth, imageHeight, 3]),
		filters=32,
		kernel_size=[16, 9],
		padding='same',
		activation=tf.nn.relu)
	pool1 = tf.layers.max_pooling2d(
		inputs=conv1,
		pool_size=[2, 2],
		strides=2)

	conv2 = tf.layers.conv2d(
		inputs=pool1,
		filters=64,
		kernel_size=[16, 9],
		padding='same',
		activation=tf.nn.relu)
	pool2 = tf.layers.max_pooling2d(
		inputs=conv2,
		pool_size=[2, 2],
		strides=2)

	pool2Shape = pool2.get_shape().as_list()
	flat = tf.reshape(pool2, [batchSize, pool2Shape[1] * pool2Shape[2] * pool2Shape[3]])

	lstmOutput, lstmFinalState, lstmInitialState, lstmState = create_lstm(
		inputs=flat,
		outputSize=32,
		time=batchSize)

	buttonOutput = tf.layers.dense(
		inputs=lstmOutput,
		units=gamepad.numButtons,
		activation=tf.nn.sigmoid)

	buttonLoss = tf.reduce_mean(tf.nn.sigmoid_cross_entropy_with_logits(
		logits=buttonOutput,
		labels=buttonTarget))

	analogOutput = tf.layers.dense(
		inputs=lstmOutput,
		units=gamepad.numAnalogs,
		activation=None)

	analogLoss = tf.reduce_mean(tf.square(tf.subtract(analogOutput,
		analogTarget)))

	loss = tf.add(analogLoss, tf.multiply(buttonLoss, 100.0))
	optimizer = tf.train.AdamOptimizer(learning_rate = 1e-4).minimize(loss)

	return videoInput, gamepadInput, buttonOutput, buttonTarget, analogOutput, \
		analogTarget, lstmFinalState, lstmInitialState, lstmState, optimizer, loss

def save_model(sess, dst):
	saver = tf.train.Saver()
	saver.save(dst, sess)

def load_model(src, imageWidth, imageHeight, batchSize):
	videoInput, gamepadInput, buttonOutput, buttonTarget, analogOutput, \
		analogTarget, lstmFinalState, lstmInitialState, lstmState, optimizer, \
		loss \
		= create_model(imageWidth, imageHeight, batchSize)
	saver = tf.train.Saver()
	sess = tf.Session()
	saver.restore(sess, src)
	return (sess, videoInput, gamepadInput, buttonOutput, buttonTarget, \
		analogOutput, analogTarget, lstmFinalState, lstmInitialState, \
		lstmState, optimizer, loss)
