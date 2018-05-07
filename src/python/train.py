import sys
import struct
from enum import Enum

import numpy as np

import model
import data
import child_program

class from_child_flag(Enum):
    train_batch_request_ack = b'\x02'
    save_model_request_ack = b'\x03'

class to_child_flag(Enum):
    train_batch_request = b'\x02'
    save_model_request = b'\x03'

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
    sess, videoInput, gamepadInput, gamepadButtonOutput, \
        gamepadAnalogOutput, gamepadButtonLabel, gamepadAnalogLabel, \
        optimizer, loss \
        = model.load_model(modelSrc, frameWidth, frameHeight)

    child_program.send_init_flag()
    while True:
        flag = child_program.next_flag();
        if flag == child_program.to_child_flag.shutdown_request.value:
            break
        elif flag == to_child_flag.save_model_request.value:
            model.save_model(modelSrc, sess)
            child_program.write(from_child_flag.save_model_request_ack, None)
            break
        elif flag == to_child_flag.train_batch_request.value:
            videoInputBatch, gamepadInputBatch, gamepadButtonLabelBatch, \
                gamepadAnalogLabelBatch \
                = data.get_batch(batchSize, frameWidth, frameHeight)
            _, lossValue = sess.run(
                [optimizer, loss],
                feed_dict =
                {
                    videoInput: videoInputBatch,
                    gamepadInput: gamepadInputBatch,
                    gamepadButtonLabel: gamepadButtonLabelBatch,
                    gamepadAnalogLabel: gamepadAnalogLabelBatch
                }
            )
            child_program.write(from_child_flag.train_batch_request_ack, \
                bytearray(struct.pack("f", lossValue)))
        else:
            eprint("Received unkown flag ", flag)
            break

    child_program.terminate()

if __name__ == "__main__":
    main()
