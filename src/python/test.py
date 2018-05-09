import sys
from enum import Enum

import model
import gamepad
import child_program

class from_child_flag(Enum):
    test_batch_request_ack = b'\x02'

class to_child_flag(Enum):
    test_batch_request = b'\x02'

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def main():
    if len(sys.argv) != 4:
        eprint("Incorrect number of arguments")
        return

    modelSrc = sys.argv[1]
    frameWidth = int(sys.argv[2])
    frameHeight = int(sys.argv[3])
    sess, videoInput, gamepadInput, buttonOutput, buttonTarget, analogOutput, \
        analogTarget, lstmFinalState, lstmInitialState, lstmState, optimizer, \
        loss \
        = model.load_model(modelSrc, frameWidth, frameHeight, 1)

    child_program.send_init_flag()
    while True:
        flag = child_program.next_flag();
        if flag == child_program.to_child_flag.shutdown_request.value:
            break
        elif flag == to_child_flag.test_batch_request.value:
            videoInput2, gamepadInput2 = gamepad.get_data(frameWidth, frameHeight)
            gamepadOutput = sess.run(
                [buttonOutput, analogOutput],
                feed_dict =
                {
                    videoInput: videoInput2,
                    gamepadInput: gamepadInput2
                    #lstmInitialState: lstmState
                }
            )
            #eprint(gamepadOutput)
            child_program.write(from_child_flag.test_batch_request_ack, \
                gamepad.compress_gamepad(gamepadOutput))
        else:
            eprint("Received unkown flag ", flag)
            break

    sess.close()
    child_program.terminate()


if __name__ == "__main__":
    main()
