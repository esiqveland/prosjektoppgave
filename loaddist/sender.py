from socket import *
import sys
import threading
import time

import random




class Query:
	def __init__(self):
		self.wordlist = []
		self.populateWordlist()
		self.query = self.generateQuery(random.randrange(1,6))
		
	def populateWordlist(self):
		# 1 - 10
		self.wordlist.append("multipl")
		self.wordlist.append("product")
		self.wordlist.append("articl")
		self.wordlist.append("mechan")
		self.wordlist.append("unmark")
		self.wordlist.append("car")
		self.wordlist.append("distributionretent")
		self.wordlist.append("phosphor")
		self.wordlist.append("cellular")
		self.wordlist.append("convert")
		# 11 - 20
		self.wordlist.append("aerob")
		self.wordlist.append("suitabl")
		self.wordlist.append("interpret")
		self.wordlist.append("aquat")
		self.wordlist.append("compon")
		self.wordlist.append("contentbas")
		self.wordlist.append("sugar")
		self.wordlist.append("hydrocarbon")
		self.wordlist.append("said")
		self.wordlist.append("basic")
		# 21 - 30
		self.wordlist.append("basin")
		self.wordlist.append("intern")
		self.wordlist.append("classif")
		self.wordlist.append("diagnosi")
		self.wordlist.append("intermedi")
		self.wordlist.append("map")
		self.wordlist.append("pathway")
		self.wordlist.append("tile")
		self.wordlist.append("sulfon")
		self.wordlist.append("beamdy")
		# 31 - 40
		self.wordlist.append("equip")
		self.wordlist.append("thin")
		self.wordlist.append("pour")
		self.wordlist.append("materi")
		self.wordlist.append("by")
		self.wordlist.append("odor")
		self.wordlist.append("project")
		self.wordlist.append("hyperlink")
		self.wordlist.append("load")
		self.wordlist.append("aquacultur")
		# 41 - 50
		self.wordlist.append("supersatur")
		self.wordlist.append("methyliminobisalkylacetamid")
		self.wordlist.append("mixingaer")
		self.wordlist.append("interchang")
		self.wordlist.append("reverseproject")
		self.wordlist.append("hydrolyt")
		self.wordlist.append("accommod")
		self.wordlist.append("simplifi")
		self.wordlist.append("shale")
		self.wordlist.append("object")

	def generateQuery(self, numberOfWords):
		query_string = ""
		for i in range(0,numberOfWords):
			number = random.randrange(0,50)
			query_string += self.wordlist[number] + " "
		return query_string

	



class Connection:
	def __init__(self, port):
		self.ip = my_ip
		self.port = port
		self.socket = socket(AF_INET,SOCK_DGRAM)
		self.bufferSize = connection_buffer_size

	def getConnectionSocket(self):
		return self.socket

	def getConnectionAddrTuple(self):
		return (self.ip, self.port)

	def sendMessage(self, counts):
		time.sleep(5)

		addrTuple = (load_dir_ip,load_dir_port)
		while counts > 0:
			#print("Sending message " + message + " to IP: " + self.ip)
			time.sleep(sleep_time_between_sends)
			query_object = Query()
			self.socket.sendto(query_object.query, addrTuple)
			counts -= 1
		self.socket.close()

	def bind(self):
		self.socket.bind((self.ip, self.port))
		self.socket.settimeout(rec_timeout)
		
	def receiveData(self):
		t0 = time.time()
		t1 = time.time()
		counter = 0

		print "Receiver: waiting for data...\n"
		while True:
			try:
				message_received = self.socket.recv(connection_buffer_size)
				counter += 1
			except timeout:
				print "timeout: counter = " + str(counter) 
				break





class connectionThread (threading.Thread):
    def __init__(self, threadID, name, port):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.connection = Connection(port)

    def run(self):
        print "Starting " + self.name + "\n"
        if self.threadID > 0:
        	self.connection.sendMessage(messages_to_send)

        elif self.threadID == 0:
        	self.connection.bind()
        	self.connection.receiveData()
        
        print "Exiting " + self.name + "\n"


# --- CONFIG --- #
messages_to_send = 1000
sleep_time_between_sends = 0.0003
number_of_sending_threads = 1
sending_thread_list = []
connection_buffer_size = 1024

my_rec_port = 32000
my_ip = "127.0.0.1"
rec_timeout = 15
load_dir_port = 32002
load_dir_ip = "127.0.0.1"

# --- END OF CONFIG --- #


random.seed()

receiving_thread = connectionThread(0, "Receiver", my_rec_port)
receiving_thread.start()

for i in range(1,number_of_sending_threads + 1):
	name = "Sender" + str(i)
	sending_thread_list.append(connectionThread(i, name, 0))

for thread in sending_thread_list:
	thread.start()

print "Exiting Main Thread"
