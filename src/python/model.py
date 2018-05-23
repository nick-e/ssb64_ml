import sys

import numpy as np
import tensorflow as tf

import gamepad

def eprint(*args, **kwargs):
	print(*args, file=sys.stderr, **kwargs)

class test_model:
	def __init__(self, imageWidth, imageHeight):
		self.videoInput = tf.placeholder(tf.float32, shape=[imageWidth * imageHeight * 3], name="videoInput")

		conv1 = tf.layers.conv2d(
			inputs=tf.reshape(self.videoInput, [1, imageWidth, imageHeight, 3]),
			filters=32,
			kernel_size=[10, 10],
			padding="same",
			activation=tf.nn.relu,
			name="conv1")
		pool1 = tf.layers.max_pooling2d(
			inputs=conv1,
			pool_size=[2, 2],
			strides=2,
			name="pool1")

		conv2 = tf.layers.conv2d(
			inputs=pool1,
			filters=64,
			kernel_size=[10, 10],
			padding="same",
			activation=tf.nn.relu,
			name="conv2")
		pool2 = tf.layers.max_pooling2d(
			inputs=conv2,
			pool_size=[2, 2],
			strides=2,
			name="pool2")

		flat = tf.reshape(pool2, [1, 1, int(pool2.shape[1]) * int(pool2.shape[2]) * int(pool2.shape[3])], name="flat")

		lstms = [
			tf.nn.rnn_cell.BasicLSTMCell(
				256,
				forget_bias=1.0,
				state_is_tuple=True,
				name="cell")
			for i in range(4)]
		drops = [
			tf.contrib.rnn.DropoutWrapper(
				lstm,
				output_keep_prob=1.0,
				variational_recurrent=True,
				dtype=tf.float32)
			for lstm in lstms]
		cell = tf.contrib.rnn.MultiRNNCell(drops)
		self.lstmInitialStates = cell.zero_state(1, tf.float32)
		lstmOutputs, self.lstmFinalStates = tf.nn.dynamic_rnn(cell, flat, time_major=False, initial_state=self.lstmInitialStates)
		self.lstmFinalOutput = tf.reshape(lstmOutputs[0, -1], [1, -1])
		self.lstmStates = []
		for c, h in self.lstmFinalStates:
			self.lstmStates.append((np.zeros(c.get_shape()), np.zeros(h.get_shape())))

		buttonLogits = tf.layers.dense(
			inputs=self.lstmFinalOutput,
			units=gamepad.numButtons,
			activation=None,
			name="buttonLogits")

		self.buttonOutput = tf.sigmoid(
			buttonLogits,
			name="buttonOutput")

		self.analogOutput = tf.layers.dense(
			inputs=self.lstmFinalOutput,
			units=gamepad.numAnalogs,
			activation=None,
			name="analogOutput")

	def run(self, sess, videoInput):
		feedDict = {
			self.videoInput: videoInput
		}
		for i, (c, h) in enumerate(self.lstmInitialStates):
			feedDict[c] = self.lstmStates[i][0]
			feedDict[h] = self.lstmStates[i][1]
		states = []
		for c, h in self.lstmFinalStates:
			states.append([c, h])
		fetches = [self.buttonOutput, self.analogOutput, states]

		buttonOutput, analogOutput, self.lstmStates = sess.run(fetches, feed_dict=feedDict)
		eprint(analogOutput)
		return (buttonOutput, analogOutput)

	def clear_lstm_state(self, sess):
		fetches = []
		feedDict = {}
		for i, (c, h) in enumerate(self.lstmInitialStates):
			feedDict[c] = np.zeros(c.get_shape())
			feedDict[h] = np.zeros(c.get_shape())
			fetches.append(c)
			fetches.append(h)
		sess.run(fetches, feedDict)

	def load(self, src):
		saver = tf.train.Saver()
		sess = tf.Session()
		saver.restore(sess, src)
		return sess

class train_model:
	def __init__(self, imageWidth, imageHeight, batchSize):
		self.analogLabel = tf.placeholder(tf.float32, shape=[1, gamepad.numAnalogs], name="analogLabel")
		self.buttonLabel = tf.placeholder(tf.float32, shape=[1, gamepad.numButtons], name="buttonLabel")

		with tf.name_scope('videoInput') as scope:
			self.videoInput = tf.placeholder(tf.float32, shape=[batchSize, imageWidth * imageHeight * 3])

		conv1 = tf.layers.conv2d(
			inputs=tf.reshape(self.videoInput, [batchSize, imageWidth, imageHeight, 3]),
			filters=32,
			kernel_size=[10, 10],
			padding="same",
			activation=tf.nn.relu,
			name="conv1")
		pool1 = tf.layers.max_pooling2d(
			inputs=conv1,
			pool_size=[2, 2],
			strides=2,
			name="pool1")

		conv2 = tf.layers.conv2d(
			inputs=pool1,
			filters=64,
			kernel_size=[10, 10],
			padding="same",
			activation=tf.nn.relu,
			name="conv2")
		pool2 = tf.layers.max_pooling2d(
			inputs=conv2,
			pool_size=[2, 2],
			strides=2,
			name="pool2")

		flat = tf.reshape(pool2, [1, batchSize, int(pool2.shape[1]) * int(pool2.shape[2]) * int(pool2.shape[3])], name="flat")

		lstms = [
			tf.nn.rnn_cell.BasicLSTMCell(
				256,
				forget_bias=1.0,
				state_is_tuple=True,
				name="cell")
			for i in range(4)]
		drops = [
			tf.contrib.rnn.DropoutWrapper(
				lstm,
				output_keep_prob=0.5,
				variational_recurrent=True,
				dtype=tf.float32)
			for lstm in lstms]
		cell = tf.contrib.rnn.MultiRNNCell(drops)
		self.lstmInitialStates = cell.zero_state(1, tf.float32)
		lstmOutputs, self.lstmFinalStates = tf.nn.dynamic_rnn(cell, flat, time_major=False, initial_state=self.lstmInitialStates)
		self.lstmFinalOutput = tf.reshape(lstmOutputs[0, -1], [1, -1])
		self.lstmStates = []
		for c, h in self.lstmFinalStates:
			self.lstmStates.append((np.zeros(c.get_shape()), np.zeros(h.get_shape())))

		#cell = tf.contrib.rnn.DropoutWrapper(
		#	tf.nn.rnn_cell.BasicLSTMCell(
		#		128,
		#		forget_bias=1.0,
		#		state_is_tuple=True,
		#		name="cell"),
		#	output_keep_prob=0.5,
		#	variational_recurrent=True,
		#	dtype=tf.float32)
		#self.lstmInitialState = cell.zero_state(1, tf.float32)
		#lstmOutputs, lstmStates = tf.nn.static_rnn(cell, tf.unstack(flat, batchSize, 0), initial_state=self.lstmInitialState)
		#self.lstmFinalOutput = lstmOutputs[-1]
		#self.lstmFinalCellState = lstmStates[-1]
		#self.lstmCellState = np.zeros(self.lstmFinalCellState.get_shape())
		#self.lstmPrevFinalOutput = np.zeros(self.lstmFinalOutput.get_shape())

		buttonLogits = tf.layers.dense(
			inputs=self.lstmFinalOutput,
			units=gamepad.numButtons,
			activation=None,
			name="buttonLogits")

		# this is binary cross-entropy, not multiclass cross-entropy
		with tf.name_scope('buttonLoss') as scope:
			buttonLoss = tf.reduce_mean(
				tf.nn.sigmoid_cross_entropy_with_logits(
					logits=buttonLogits,
					labels=self.buttonLabel))

		analogOutput = tf.layers.dense(
			inputs=self.lstmFinalOutput,
			units=gamepad.numAnalogs,
			activation=None,
			name="analogOutput")

		with tf.name_scope('analogLoss') as scope:
			analogLoss = tf.reduce_mean(
				tf.square(tf.subtract(analogOutput, self.analogLabel)))

		self.loss = tf.add(
			analogLoss,
			buttonLoss,
			name="loss")

		self.optimizer = tf.train.AdamOptimizer(
			learning_rate=1e-4,
			name="optimizer").minimize(self.loss)

	def run(self, sess, videoInputBatch, buttonLabel, analogLabel):
		feedDict = {
			self.videoInput: videoInputBatch,
			self.buttonLabel: [buttonLabel],
			self.analogLabel: [analogLabel]
		}
		for i, (c, h) in enumerate(self.lstmInitialStates):
			feedDict[c] = self.lstmStates[i][0]
			feedDict[h] = self.lstmStates[i][1]
		states = []
		for c, h in self.lstmFinalStates:
			states.append([c, h])
		fetches = [self.optimizer, self.loss, states]

		_, loss, self.lstmStates = sess.run(fetches, feed_dict=feedDict)
		return loss

	def validate(self, sess, videoInputBatch, buttonLabelBatch, analogLabelBatch):
		feedDict = {
			self.videoInput: videoInputBatch,
			self.buttonLabel: [buttonLabel],
			self.analogLabel: [analogLabel]
		}
		for i, (c, h) in enumerate(self.lstmInitialStates):
			feedDict[c] = self.lstmStates[i][0]
			feedDict[h] = self.lstmStates[i][1]
		states = []
		for c, h in self.lstmFinalStates:
			states.append([c, h])
		fetches = [self.loss, states]

		loss, self.lstmStates = sess.run(fetches, feed_dict=feedDict)
		return loss

	def clear_lstm_state(self, sess):
		fetches = []
		feedDict = {}
		for i, (c, h) in enumerate(self.lstmInitialStates):
			feedDict[c] = np.zeros(c.get_shape())
			feedDict[h] = np.zeros(c.get_shape())
			fetches.append(c)
			fetches.append(h)
		sess.run(fetches, feedDict)

	def save(self, dst, sess):
		saver = tf.train.Saver()
		saver.save(sess, dst)

	def load(self, src):
		saver = tf.train.Saver()
		sess = tf.Session()
		saver.restore(sess, src)
		self.writer = tf.summary.FileWriter("/home/nick/Documents/ssb64_ml/data/tensorboard/", graph=sess.graph)
		return sess

	def __exit__(self, exc_type, exc, traceback):
		self.writer.flush()
		self.writer.close()
