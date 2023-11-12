import uasyncio
import queue
from micropython import const

slot_address = const(0x1)
command_queue = queue.Queue()
