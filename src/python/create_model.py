import sys

import tensorflow as tf

import model

def main():
	if len(sys.argv) != 4:
		print("Incorrect number of arguments")

	dst = sys.argv[1]
	imageWidth = int(sys.argv[2])
	imageHeight = int(sys.argv[3])
	print(imageWidth)
	print(imageHeight)

	videoInput, gamepadInput, gamepadButtonOutput, gamepadAnalogOutput, \
		gamepadButtonLabel, gamepadAnalogLabel, optimizer, loss \
		= model.create_model(imageWidth, imageHeight)

	with tf.Session() as sess:
		sess.run(tf.global_variables_initializer())
		model.save_model(dst, sess)

if __name__ == "__main__":
	main()
