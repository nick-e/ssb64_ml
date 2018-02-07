import numpy as np
import cv2
import os
import tensorflow as tf
import struct

# converts the binary information from a ".conf" file to a numpy array of controller inputs
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

# creates a tensor meant to contain weights, randomly generates the initial values based on a normal curve with a standard deviation of 0.05
def createWeights(shape):
	return tf.Variable(tf.truncated_normal(shape, stddev = 0.05))

# creates a 1D tensor meant to contain biases, the values are initialized to 0.05
def createBiases(length):
	return tf.Variable(tf.constant(0.05, shape=[length]))

# creates a convoltuion layer with pooling
def createConvLayer(input, numInputChannels, filterWidth, filterHeight, numFilters):

	# shape of the filter/weights for the convolution
	shape = [filterWidth, filterHeight, numInputChannels, numFilters]

	# create weights, a.k.a. the filters with the given shape
	weights = createWeights(shape = shape)

	# create biases, 1 for each filter
	biases = createBiases(length = numFilters)

	layer = tf.nn.conv2d(input = input, filter = weights, strides = [1, 1, 1, 1], padding = 'SAME')

	# add the biases to the results of the convolution
	# each filter will have a bias added to it
	layer += biases

	# reduce the size of the convolution layer to 1/4 its original size by using pooling
	layer = tf.nn.max_pool(value = layer, ksize = [1, 4, 4, 1], strides = [1, 4, 4, 1], padding = 'SAME')

	# remove any negative numbers generated in this layer by setting those values to 0
	layer = tf.nn.relu(layer)

	return (layer, weights)

# converts a convolution layer, which is a 4D tensor of [numImages, imageWidth, imageHeight, numChannels], to a 2D tensor, which is [numImages, imageWidth * imageHeight * numChannels]
def flattenConvLayer(layer):
	layerShape = layer.get_shape()

	# numFeatures is imageWidth * imageHeight * numChannels
	numFeatures = np.array(layerShape[1:4], dtype = int).prod()

	# reshape the layer to the correct 2D shape
	layerFlat = tf.reshape(layer, [-1, numFeatures])

	return layerFlat, numFeatures

# creates a fully connected layer
def createFullConLayer(input, numInputs, numOutputs, useRelu = True):
	weights = createWeights(shape = [numInputs, numOutputs])
	biases = createBiases(length = numOutputs)

	# the output of this layer is the result of multiply the previous layer's inputs with its weights and biases
	layer = tf.matmul(input, weights) + biases

	if useRelu:
		layer = tf.nn.relu(layer)

	return layer

def createCNN(imageWidth, imageHeight, numInputChannels, numControllerInputs, numOutputs):
	imgDimFlat = imageWidth * imageHeight * numInputChannels

	# This will store the video input
	videoInputPlaceholder = tf.placeholder(tf.float32, shape = [None, imgDimFlat])

	# Convolutional layers require a 4D shape ([numImages, imageWidth, imageHeight, numInputChannels])
	videoInputPlaceholderConv = tf.reshape(videoInputPlaceholder, [-1, imageWidth, imageHeight, numInputChannels])

	convLayer1, convLayer1Weights = createConvLayer(
		input = videoInputPlaceholderConv,
		numInputChannels = numInputChannels,
		filterWidth = 32, # arbitrary
		filterHeight = 18, # arbitrary
		numFilters = 32) # arbitrary

	convLayer2, convLayer2Weights = createConvLayer(
		input = convLayer1,
		numInputChannels = 32, # numFilters in convLayer1
		filterWidth = 32, # arbitrary
		filterHeight = 18, # arbitrary
		numFilters = 64) # arbitrary

	convLayer2Flat, numConvLayer2Features = flattenConvLayer(convLayer2)

	controllerInputPlaceholder = tf.placeholder(tf.float32, shape = [None, numControllerInputs])

	concatLayer = tf.concat([convLayer2Flat, controllerInputPlaceholder], 1)

	fullConLayer1 = createFullConLayer(
		input = concatLayer,
		numInputs = numConvLayer2Features + numControllerInputs,
		numOutputs = 512) # arbitrary

	# final/output layer
	# does not use RELU and softmax because it's possible that more than 1 output node needs to have a value of 1
	# instead, sigmoid is used
	fullConLayer2 = createFullConLayer(
		input = fullConLayer1,
		numInputs = 512, # numOutputs in fullConLayer1
		numOutputs = numOutputs, # number of possible controller inputs
		useRelu = False)
	fullConLayer2 = tf.sigmoid(fullConLayer2)

	buttonOutputs, analogOutputs = tf.split(fullConLayer2, [7, 4], 1)
	buttonOutputs = tf.round(buttonOutputs)

	return fullConLayer2, videoInputPlaceholder, controllerInputPlaceholder

def getNextTrainingBatch(batchSize, videoFile, controllerFile, videoBatch, controllerBatch, expectedBatch):
	# get a batch of controller input
	controllerBinaries = controllerFile.read(17 * batchSize)
	if len(controllerBinaries) != 17 * batchSize:
		return False
	for i in range(batchSize):
		controllerNp = controllerBinaryToNumpy(controllerBinaries[i * 17:(i + 1) * 17])
		controllerBatch[i][:] = controllerNp

	# get a batch of video input
	for i in range(batchSize):
		ret, frame = videoFile.read()
		frame = frame.flatten()
		videoBatch[i][:] = frame

	# get a batch of expected outputs
	expectedBatch[:batchSize - 1] = controllerBatch[1:]
	tmp = controllerFile.read(17)
	if len(tmp) != 17:
		return False
	expectedBatch[batchSize - 1][:] = controllerBinaryToNumpy(tmp)
	controllerFile.seek(-17, 1)

	return True

def main():
	os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

	imageWidth = 256
	imageHeight = 144
	numInputChannels = 3
	numControllerInputs = 11
	numOutputs = numControllerInputs
	print('creating cnn')
	cnnModel, videoInputPlaceholder, controllerInputPlaceholder = createCNN(imageWidth, imageHeight, numInputChannels, numControllerInputs, numControllerInputs)

	# training
	print('training cnn')
	batchSize = 10
	numEpochs = 2
	videoBatch = np.empty([batchSize, imageWidth * imageHeight * numInputChannels])
	controllerBatch = np.empty([batchSize, numControllerInputs])
	expectedBatch = np.empty([batchSize, numOutputs])

	# This will store the expected values, a.k.a. what buttons should be pressed on the controller
	expectedPlaceholder = tf.placeholder(tf.float32, shape = [None, numOutputs])

	lossFunc = tf.nn.sigmoid_cross_entropy_with_logits(
		logits = cnnModel,
		labels = expectedPlaceholder)
	# since the training is done in batches, take the average loss of those batches to correct the neural network
	loss = tf.reduce_mean(lossFunc)
	optimizer = tf.train.AdamOptimizer(learning_rate = 1e-4).minimize(loss)

	#config = tf.ConfigProto(device_count = {'GPU': 0})
	config = tf.ConfigProto()
	config.gpu_options.allow_growth = True
	#config.gpu_options.per_process_gpu_memory_fraction = 0.7
	session = tf.Session(config = config)
	session.run(tf.global_variables_initializer())

	for filename in os.listdir('training/'):
		if filename.endswith('.mp4'):
			prefix = filename.replace('.mp4', '')
			print('\t' + prefix)
			videoFile = cv2.VideoCapture('training/' + filename)
			controllerFile = open('training/' + prefix + '.cont', 'rb')
			totalFrames = videoFile.get(cv2.CAP_PROP_FRAME_COUNT)
			totalFramesStr = str(totalFrames)
			currentFrame = 0

			while getNextTrainingBatch(batchSize, videoFile, controllerFile, videoBatch, controllerBatch, expectedBatch):
				session.run(optimizer, feed_dict = {videoInputPlaceholder: videoBatch, controllerInputPlaceholder: controllerBatch, expectedPlaceholder: expectedBatch})

				currentFrame += batchSize
				print('\t' + str(currentFrame) + ' / ' + totalFramesStr)

			controllerFile.close()
			videoFile.release()

	session.close()

if __name__ == '__main__':
	main()
