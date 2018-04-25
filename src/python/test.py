import sys
from enum import Enum

import model
import data

class Py2CC_Flag(Enum):
    ModelLoaded = b'\x41'
    Output = b'\x69'

class CC2Py_Flag(Enum):
    Data = b'\x33'
    Done = b'\x80'

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def main():
    if len(sys.argv) != 4:
        eprint("Incorrect number of arguments")
        return

    modelSrc = sys.argv[1]
    frameWidth = int(sys.argv[2])
    frameHeight = int(sys.argv[3])
    sess, videoInput, gamepadInput, gamepadButtonOutput, \
        gamepadAnalogOutput, gamepadButtonLabel, gamepadAnalogLabel, \
        optimizer, loss \
        = model.load_model(modelSrc, frameWidth, frameHeight)

    sys.stdout.buffer.write(Py2CC_Flag.ModelLoaded.value)
    sys.stdout.flush()

    while True:
        flag = sys.stdin.buffer.read(1)
        if flag == CC2Py_Flag.Done.value:
            break
        elif flag == CC2Py_Flag.Data.value:
            videoInput2, gamepadInput2 = data.get_data(frameWidth, frameHeight)
            gamepadOutput = sess.run(
                [gamepadButtonOutput, gamepadAnalogOutput],
                feed_dict =
                {
                    videoInput: videoInput2,
                    gamepadInput: gamepadInput2
                }
            )
            sys.stdout.buffer.write(Py2CC_Flag.Output.value)
            sys.stdout.buffer.write(data.compress_gamepad(gamepadOutput))
            sys.stdout.flush()
        else:
            eprint("Received unkown flag ", flag)
            break


if __name__ == "__main__":
    main()
