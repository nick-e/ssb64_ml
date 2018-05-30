import sys
from enum import Enum

import numpy as np
from PIL import Image

import model
import gamepad
import child_program

class to_child_flag(Enum):
	test = b'\x02'
	clear_lstm_state = b'\x03'

def eprint(*args, **kwargs):
	print(*args, file=sys.stderr, **kwargs)

def main():
	if len(sys.argv) != 4:
		eprint("Incorrect number of arguments")
		return

	modelSrc = sys.argv[1]
	frameWidth = int(sys.argv[2])
	frameHeight = int(sys.argv[3])
	instance = model.test_model(frameWidth, frameHeight)
	sess = instance.load(modelSrc)

	child_program.write_flag(child_program.from_child_flag.child_initialized)
	while True:
		flag = child_program.next_flag();
		if flag == to_child_flag.test.value:
			videoInput = gamepad.get_image_data(frameWidth, frameHeight)
			#gamepad.display_image(videoInput)
			#break
			gamepadOutput = instance.run(sess, videoInput)
			child_program.write_data(gamepad.compress_gamepad(gamepadOutput))
		elif flag == to_child_flag.clear_lstm_state.value:
			instance.clear_lstm_state(sess)
		elif flag == child_program.to_child_flag.shutdown_request.value:
			break
		else:
			eprint("Received unkown flag ", flag)
			break

	sess.close()
	child_program.write_flag(child_program.from_child_flag.child_terminated)

if __name__ == "__main__":
	main()
