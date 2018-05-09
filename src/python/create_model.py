import sys

import tensorflow as tf

import model

def main():
	if len(sys.argv) != 4:
		print("Incorrect number of arguments")

	dst = sys.argv[1]
	imageWidth = int(sys.argv[2])
	imageHeight = int(sys.argv[3])

	videoInput, gamepadInput, buttonOutput, buttonTarget, analogOutput, \
		analogTarget, lstmFinalState, lstmInitialState, lstmState, optimizer, loss \
		= model.create_model(imageWidth, imageHeight, 1)

	with tf.Session() as sess:
		sess.run(tf.global_variables_initializer())
		model.save_model(dst, sess)

if __name__ == "__main__":
	main()
