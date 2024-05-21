import time
from scapy.all import *

while(1):
    x = input()
    if(x == "no"): break
    capture = sniff()
    print(capture)
