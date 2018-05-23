from enum import Enum
import sys

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

class to_child_flag(Enum):
    shutdown_request = b'\x00'

class from_child_flag(Enum):
    child_initialized = b'\x00'
    child_terminated = b'\x01'

def write_flag(flag):
    #eprint("Sending ", flag.value)
    sys.stdout.buffer.write(flag.value)
    sys.stdout.flush()

def write_data(buf):
    sys.stdout.buffer.write(buf)
    sys.stdout.flush()

def next_flag():
    flag = sys.stdin.buffer.read(1)
    #eprint("Received ", flag)
    return flag
