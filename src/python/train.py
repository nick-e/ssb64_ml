import sys
import struct
from enum import Enum

import numpy as np

import model
import gamepad
import child_program

class from_child_flag(Enum):
    train_batch_request_ack = b'\x02'
    save_model_request_ack = b'\x03'
    val_batch_request_ack = b'\x04'

class to_child_flag(Enum):
    train_batch_request = b'\x02'
    save_model_request = b'\x03'
    val_batch_request = b'\x04'

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
    sess, videoInput, gamepadInput, buttonOutput, buttonTarget, analogOutput, \
        analogTarget, lstmFinalState, lstmInitialState, lstmState, optimizer, \
        loss \
        = model.load_model(modelSrc, frameWidth, frameHeight, batchSize)

    child_program.send_init_flag()
    while True:
        flag = child_program.next_flag();
        if flag == child_program.to_child_flag.shutdown_request.value:
            break
        elif flag == to_child_flag.save_model_request.value:
            model.save_model(modelSrc, sess)
            child_program.write(from_child_flag.save_model_request_ack, None)
            break
        elif flag == to_child_flag.val_batch_request.value:
            videoInputBatch, gamepadInputBatch, buttonTargetBatch, \
                analogTargetBatch \
                = gamepad.get_batch(batchSize, frameWidth, frameHeight)
            lossValue, lstmState = sess.run(
                [loss, lstmFinalState],
                feed_dict =
                {
                    videoInput: videoInputBatch,
                    gamepadInput: gamepadInputBatch,
                    buttonTarget: buttonTargetBatch,
                    analogTarget: analogTargetBatch,
                    lstmInitialState: lstmState
                }
            )
            child_program.write(from_child_flag.val_batch_request_ack, \
                bytearray(struct.pack("f", lossValue)))
        elif flag == to_child_flag.train_batch_request.value:
            videoInputBatch, gamepadInputBatch, buttonTargetBatch, \
                analogTargetBatch \
                = gamepad.get_batch(batchSize, frameWidth, frameHeight)
            _, lossValue = sess.run(
                [optimizer, loss],
                feed_dict =
                {
                    videoInput: videoInputBatch,
                    gamepadInput: gamepadInputBatch,
                    buttonTarget: buttonTargetBatch,
                    analogTarget: analogTargetBatch
                }
            )
            child_program.write(from_child_flag.train_batch_request_ack, \
                bytearray(struct.pack("f", lossValue)))
        else:
            eprint("Received unkown flag ", flag)
            break

    sess.close()
    child_program.terminate()

if __name__ == "__main__":
    main()
