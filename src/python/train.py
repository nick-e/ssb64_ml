import sys
import struct
from enum import Enum

import numpy as np

import model
import gamepad
import child_program

class from_child_flag(Enum):
	save_ack = b'\x03'

class to_child_flag(Enum):
	train = b'\x02'
	save = b'\x03'
	validate = b'\x04'
	new_file = b'\x05'

def eprint(*args, **kwargs):
	print(*args, file=sys.stderr, **kwargs)

def main():
	if len(sys.argv) != 5:
		eprint("Incorrect number of arguments")
		child_program.terminate()
		return

	modelSrc = sys.argv[1]
	frameWidth = int(sys.argv[2])
	frameHeight = int(sys.argv[3])
	batchSize = int(sys.argv[4])
	instance = model.train_model(frameWidth, frameHeight, batchSize)
	sess = instance.load(modelSrc)
	videoInputBatch = []
	child_program.write_flag(child_program.from_child_flag.child_initialized)
	try:
		while True:
			flag = child_program.next_flag();
			if flag == to_child_flag.save.value:
				instance.save(modelSrc, sess)
				child_program.write_flag(from_child_flag.save_ack)
				break
			elif flag == to_child_flag.train.value:
				frame = gamepad.get_image_data(frameWidth, frameHeight)
				videoInputBatch.append([frame])
				#gamepad.display_image(frame)
				#break
				buttonLabel, analogLabel = gamepad.get_gamepad_data()
				loss = instance.run(
					sess,
					np.reshape(videoInputBatch, (batchSize, frameWidth * frameHeight * 3)),
					buttonLabel,
					analogLabel)
				child_program.write_data(bytearray(struct.pack("f", loss)))
				videoInputBatch.pop(0)
			elif flag == to_child_flag.validate.value:
				videoInputBatch.append(
					[gamepad.get_image_data(frameWidth, frameHeight)])
				buttonLabel, analogLabel = gamepad.get_gamepad_data()
				loss = instance.validate(
					sess,
					np.reshape(videoInputBatch, (batchSize, frameWidth * frameHeight * 3)),
					buttonLabel,
					analogLabel)
				child_program.write_data(bytearray(struct.pack("f", loss)))
				videoInputBatch.pop(0)
			elif flag == to_child_flag.new_file.value:
				instance.clear_lstm_state(sess)
				videoInputBatch = []
				for i in range(batchSize - 1):
					videoInputBatch.append(
						[gamepad.get_image_data(frameWidth, frameHeight)])
			elif flag == child_program.to_child_flag.shutdown_request.value:
				break
			else:
				eprint("Received unkown flag ", flag)
				break
	except Exception as e:
		eprint(e)

	instance.__exit__(None, None, None)
	sess.close()
	child_program.write_flag(child_program.from_child_flag.child_terminated)

if __name__ == "__main__":
	main()
