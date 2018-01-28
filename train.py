import numpy as np
import cv2
import os
import tensorflow as tf

def main():
	for filename in os.listdir('/data'):
		if filename.endswith('.avi'):
			prefix = filename.replace('.avi', '')
			videoFile = cv2.VideoCapture(filename)
			controllerFile = open('/data' + filename.replace('.avi', '') + '.cont', 'rb')


if __name__ == '__main__':
	main()
