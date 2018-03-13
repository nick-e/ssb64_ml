import numpy as np
import tensorflow as tf

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

def create_model(imageWidth, imageHeight, numInputChannels, numControllerInputs, numOutputs):
	imgDimFlat = imageWidth * imageHeight * numInputChannels

	# This will store the video input
	videoInputPlaceholder = tf.placeholder(tf.float32, shape = [None, imgDimFlat], name = "videoInputPlaceholder")

	# Convolutional layers require a 4D shape ([numImages, imageWidth, imageHeight, numInputChannels])
	videoInputPlaceholderConv = tf.reshape(videoInputPlaceholder, [-1, imageWidth, imageHeight, numInputChannels])

	convLayer1, convLayer1Weights = create_conv_layer(
		input = videoInputPlaceholderConv,
		numInputChannels = numInputChannels,
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

	controllerInputPlaceholder = tf.placeholder(tf.float32, shape = [None, numControllerInputs], name = "controllerInputPlaceholder")

	concatLayer = tf.concat([convLayer2Flat, controllerInputPlaceholder], 1)

	fullConLayer1 = create_full_con_layer(
		input = concatLayer,
		numInputs = numConvLayer2Features + numControllerInputs,
		numOutputs = 512) # arbitrary

	# final/output layer
	# does not use RELU and softmax because it's possible that more than 1 output node needs to have a value of 1
	# instead, sigmoid is used
	fullConLayer2 = create_full_con_layer(
		input = fullConLayer1,
		numInputs = 512, # numOutputs in fullConLayer1
		numOutputs = numOutputs, # number of possible controller inputs
		useRelu = False)

	controllerButtonOutputs, controllerAnalogOutputs = tf.split(fullConLayer2, [8, 6], 1, name = "split")
	controllerButtonOutputs = tf.sigmoid(controllerButtonOutputs)
	expectedControllerButtonOutputsPlaceholder = tf.placeholder(tf.float32, shape = [None, 8], name = "expectedControllerButtonOutputsPlaceholder")
	expectedControllerAnalogOutputsPlaceholder = tf.placeholder(tf.float32, shape = [None, 6], name = "expectedControllerAnalogOutputsPlaceholder")
	controllerAnalogLoss = tf.reduce_mean(tf.square(tf.subtract(controllerAnalogOutputs, expectedControllerAnalogOutputsPlaceholder)))
	controllerButtonLoss = tf.reduce_mean(tf.nn.sigmoid_cross_entropy_with_logits(
		logits = controllerButtonOutputs,
		labels = expectedControllerButtonOutputsPlaceholder))
	loss = tf.add(controllerAnalogLoss, controllerButtonLoss, name = "loss")
	optimizer = tf.train.AdamOptimizer(learning_rate = 1e-4).minimize(loss, name = "optimizer")

	return videoInputPlaceholder, controllerInputPlaceholder, controllerButtonOutputs, controllerAnalogOutputs, expectedControllerButtonOutputsPlaceholder, expectedControllerAnalogOutputsPlaceholder, optimizer, loss

def save_model(dst, imageWidth, imageHeight, numInputChannels, numControllerInputs, numOutputs):
	videoInputPlaceholder, controllerInputPlaceholder, controllerButtonOutputs, controllerAnalogOutput, expectedControllerButtonOutputsPlaceholder, expectedControllerAnalogOutputPlaceholder, optimizer, loss = create_model(imageWidth, imageHeight, numInputChannels, numControllerInputs, numOutputs)
	sess = tf.Session()
	sess.run(tf.global_variables_initializer())
	saver = tf.train.Saver()
	saver.save(sess, dst)

def load_model(src, imageWidth, imageHeight, numInputChannels, numControllerInputs, numOutputs):
    videoInputPlaceholder, controllerInputPlaceholder, controllerButtonOutputs, controllerAnalogOutputs, expectedControllerButtonOutputsPlaceholder, expectedControllerAnalogOutputsPlaceholder, optimizer, loss = create_model(imageWidth, imageHeight, numInputChannels, numControllerInputs, numOutputs)
    saver = tf.train.Saver()
    sess = tf.Session()
    saver.restore(sess, src)
    return (sess, videoInputPlaceholder, controllerInputPlaceholder, controllerButtonOutputs, controllerAnalogOutputs, expectedControllerButtonOutputsPlaceholder, expectedControllerAnalogOutputsPlaceholder, optimizer, loss)
