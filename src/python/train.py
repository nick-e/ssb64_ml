import sys
from enum import Enum

import model
import data

class Py2CC_Flag(Enum):
    ModelLoaded = b'\x41'
    BatchTrained = b'\x17'
    ModelSaved = b'\x99'

class CC2Py_Flag(Enum):
    Batch = b'\x33'
    Done = b'\x80'

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def main():
    if len(sys.argv) != 5:
        eprint("Incorrect number of arguments")
        return

    modelSrc = sys.argv[1]
    frameWidth = int(sys.argv[2])
    frameHeight = int(sys.argv[3])
    batchSize = int(sys.argv[4])
    sess, videoInput, gamepadInput, gamepadButtonOutput, \
        gamepadAnalogOutput, gamepadButtonLabel, gamepadAnalogLabel, \
        optimizer, loss \
        = model.load_model(modelSrc, frameWidth, frameHeight)

    sys.stdout.buffer.write(Py2CC_Flag.ModelLoaded.value)
    sys.stdout.flush()

    while True:
        flag = sys.stdin.buffer.read(1)
        if flag == CC2Py_Flag.Done.value:
            model.save_model(modelSrc, sess)
            sys.stdout.buffer.write(Py2CC_Flag.ModelSaved.value)
            sys.stdout.flush()
            break
        elif flag == CC2Py_Flag.Batch.value:
            videoInputBatch, gamepadInputBatch, gamepadButtonLabelBatch, \
                gamepadAnalogLabelBatch \
                = data.get_batch(batchSize, frameWidth, frameHeight)
            _, v = sess.run(
                [optimizer, loss],
                feed_dict =
                {
                    videoInput: videoInputBatch,
                    gamepadInput: gamepadInputBatch,
                    gamepadButtonLabel: gamepadButtonLabelBatch,
                    gamepadAnalogLabel: gamepadAnalogLabelBatch
                }
            )
            eprint(v)
            sys.stdout.buffer.write(Py2CC_Flag.BatchTrained.value)
            sys.stdout.flush()
        else:
            eprint("Received unkown flag ", flag)
            break


if __name__ == "__main__":
    main()
