from socket import *
import threading
import time
import random
from load_common import *

# --- CONFIG --- #
messages_to_send = 3
sleep_time_between_sends = 0.00045  # MAC
#sleep_time_between_sends = 0.01  # PI
number_of_sending_threads = 1
sending_thread_list = []

my_ip = "127.0.0.1"
recv_port = 32005
load_dir_port = 32003
load_dir_ip = "127.0.0.1"

node_set = ["192.168.0.200",
            "192.168.0.201",
            "192.168.0.202",
            "192.168.0.203",
            "192.168.0.204",
            "192.168.0.205",
            "192.168.0.206",
            "192.168.0.207"]

dictionary_path = "target/dictionary.txt"

# --- END OF CONFIG --- #

query_object = Query(dict_filepath=dictionary_path)

random.seed()

receiving_thread = ReceiverThread(threadID=0, name="Receiver", my_ip=my_ip,
                                  nmessages=messages_to_send, port=recv_port)
receiving_thread.start()


for i in xrange(number_of_sending_threads):
    name = "Sender" + load_dir_ip
    sending_thread_list.append(ConnectionThread(threadID=i, name=name, my_ip=my_ip, target_ip=load_dir_ip,
                                                port=recv_port, target_port=load_dir_port, sleeptime=sleep_time_between_sends,
                                                nmessages=messages_to_send/number_of_sending_threads,
                                                query_object=query_object))

for thread in sending_thread_list:
    thread.start()

for thread in sending_thread_list:
    if thread.isAlive():
        thread.join()
receiving_thread.join()

print "Exiting Main Thread"
