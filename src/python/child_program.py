from enum import Enum
import sys

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

class to_child_flag(Enum):
    shutdown_request = b'\x00'

class from_child_flag(Enum):
    child_initialized = b'\x00'
    child_terminated = b'\x01'

def send_init_flag():
    write(from_child_flag.child_initialized, None)

def terminate():
    write(from_child_flag.child_terminated, None)

def write(flag, buf):
    #eprint("Sending ", flag.value)
    sys.stdout.buffer.write(flag.value)
    if buf is not None:
        sys.stdout.buffer.write(buf)
    sys.stdout.flush()

def next_flag():
    flag = sys.stdin.buffer.read(1)
    #eprint("Received ", flag)
    return flag
