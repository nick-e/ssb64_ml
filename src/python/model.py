import numpy as np
import tensorflow as tf

import data

'''
Creates a tensor meant to contain weights, randomly generates the initial values
based on a normal curve with a standard deviation of 0.05
'''
def create_weights(shape):
	return tf.Variable(tf.truncated_normal(shape, stddev=0.05))

# creates a 1D tensor meant to contain biases, the values are initialized to 0.05
def create_biases(length):
	return tf.Variable(tf.constant(0.05, shape=[length]))

# creates a convoltuion layer with pooling
def create_conv_layer(input, numInputChannels, filterWidth, filterHeight, numFilters):

	# shape of the filter/weights for the convolution
	shape = [filterWidth, filterHeight, numInputChannels, numFilters]

	# create weights, a.k.a. the filters with the given shape
	weights = create_weights(shape=shape)

	# create biases, 1 for each filter
	biases = create_biases(length=numFilters)

	layer = tf.nn.conv2d(input=input, filter = weights, strides = [1, 1, 1, 1], padding = 'SAME')

	# add the biases to the results of the convolution
	# each filter will have a bias added to it
	layer += biases

	# reduce the size of the convolution layer to 1/4 its original size by using pooling
	layer = tf.nn.max_pool(value = layer, ksize = [1, 4, 4, 1], strides = [1, 4, 4, 1], padding = 'SAME')

	# remove any negative numbers generated in this layer by setting those values to 0
	layer = tf.nn.relu(layer)

	return (layer, weights)

# creates a fully connected layer
def create_full_con_layer(input, numInputs, numOutputs, useRelu = True):
	weights = create_weights(shape = [numInputs, numOutputs])
	biases = create_biases(length = numOutputs)

	# the output of this layer is the result of multiply the previous layer's inputs with its weights and biases
	layer = tf.matmul(input, weights) + biases

	if useRelu:
		layer = tf.nn.relu(layer)

	return layer

# converts a convolution layer, which is a 4D tensor of [numImages, imageWidth, imageHeight, numChannels], to a 2D tensor, which is [numImages, imageWidth * imageHeight * numChannels]
def flatten_conv_layer(layer):
	layerShape = layer.get_shape()

	# numFeatures is imageWidth * imageHeight * numChannels
	numFeatures = np.array(layerShape[1:4], dtype = int).prod()

	# reshape the layer to the correct 2D shape
	layerFlat = tf.reshape(layer, [-1, numFeatures])

	return layerFlat, numFeatures

def create_model(imageWidth, imageHeight):
	imgDimFlat = imageWidth * imageHeight * 3

	# This will store the video input
	videoInput = tf.placeholder(tf.float32, shape = [None, imgDimFlat], name = "videoInput")

	# Convolutional layers require a 4D shape ([numImages, imageWidth, imageHeight, 3])
	videoInputConv = tf.reshape(videoInput, [-1, imageWidth, imageHeight, 3])

	convLayer1, convLayer1Weights = create_conv_layer(
		input = videoInputConv,
		numInputChannels = 3,
		filterWidth = 32, # arbitrary
		filterHeight = 18, # arbitrary
		numFilters = 32) # arbitrary

	convLayer2, convLayer2Weights = create_conv_layer(
		input = convLayer1,
		numInputChannels = 32, # numFilters in convLayer1
		filterWidth = 32, # arbitrary
		filterHeight = 18, # arbitrary
		numFilters = 64) # arbitrary

	convLayer2Flat, numConvLayer2Features = flatten_conv_layer(convLayer2)

	gamepadInput = tf.placeholder(tf.float32, shape = [None, data.numGamepadInputs], name = "gamepadInput")

	concatLayer = tf.concat([convLayer2Flat, gamepadInput], 1)

	fullConLayer1 = create_full_con_layer(
		input = concatLayer,
		numInputs = numConvLayer2Features + data.numGamepadInputs,
		numOutputs = 512) # arbitrary

	# final/output layer
	# does not use RELU and softmax because it's possible that more than 1 output node needs to have a value of 1
	# instead, sigmoid is used
	fullConLayer2 = create_full_con_layer(
		input = fullConLayer1,
		numInputs = 512, # numOutputs in fullConLayer1
		numOutputs = data.numGamepadInputs, # number of possible controller inputs
		useRelu = False)

	gamepadButtonOutput, gamepadAnalogOutput = tf.split(fullConLayer2, [data.numGamepadButtons, data.numGamepadAnalogs], 1, name = "split")
	gamepadButtonOutput = tf.sigmoid(gamepadButtonOutput)
	gamepadButtonLabel = tf.placeholder(tf.float32, shape = [None, data.numGamepadButtons], name = "gamepadButtonLabel")
	gamepadAnalogLabel = tf.placeholder(tf.float32, shape = [None, data.numGamepadAnalogs], name = "gamepadAnalogLabel")
	gamepadAnalogLoss = tf.reduce_mean(tf.square(tf.subtract(gamepadAnalogOutput, gamepadAnalogLabel)))
	gamepadButtonLoss = tf.reduce_mean(tf.nn.sigmoid_cross_entropy_with_logits(
		logits = gamepadButtonOutput,
		labels = gamepadButtonLabel))
	loss = tf.add(gamepadAnalogLoss, gamepadButtonLoss, name = "loss")
	optimizer = tf.train.AdamOptimizer(learning_rate = 1e-4).minimize(loss, name = "optimizer")

	return videoInput, gamepadInput, gamepadButtonOutput, gamepadAnalogOutput, gamepadButtonLabel, gamepadAnalogLabel, optimizer, loss

def save_model(sess, dst):
	saver = tf.train.Saver()
	saver.save(dst, sess)

def load_model(src, imageWidth, imageHeight):
    videoInput, gamepadInput, gamepadButtonOutput, gamepadAnalogOutput, \
		gamepadButtonLabel, gamepadAnalogLabel, optimizer, loss \
		= create_model(imageWidth, imageHeight)
    saver = tf.train.Saver()
    sess = tf.Session()
    saver.restore(sess, src)
    return (sess, videoInput, gamepadInput, gamepadButtonOutput,
		gamepadAnalogOutput, gamepadButtonLabel, gamepadAnalogLabel, optimizer,
		loss)
