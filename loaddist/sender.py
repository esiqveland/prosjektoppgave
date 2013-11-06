from socket import *
import threading
import time
import random


class Query:
    def __init__(self, dict_filepath=None):
        self.wordlist = []
        self.populateWordlist(dictionary_file=dict_filepath)
        self.query = self.generateQuery(numberOfWords=random.randrange(1, 6))

    def readWordsFromDict(self, dict_filepath):
        try:
            with open(dict_filepath, 'r') as dictfp:
                for line in dictfp:
                    entry = line.split()
                    self.wordlist.append(entry[0])
                return True
        except IOError:
            print "Could not open file:", dict_filepath
            return False

    def populateWordlist(self, dictionary_file=None):
        if dictionary_file:
            if self.readWordsFromDict(dict_filepath=dictionary_file):
                return
        # if we were not able to read the dictionary for some reason, add the dummy wordlist
        self.addDummyWords()

    def addDummyWords(self):
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
        for i in range(0, numberOfWords):
            index = random.randrange(0, len(self.wordlist))
            query_string += self.wordlist[index] + " "
        return query_string


class Connection:
    def __init__(self, port, query_object=None):
        self.ip = my_ip
        self.port = port
        self.socket = socket(AF_INET, SOCK_DGRAM)
        self.bufferSize = connection_buffer_size
        self.query_object = query_object

    def getConnectionSocket(self):
        return self.socket

    def getConnectionAddrTuple(self):
        return (self.ip, self.port)

    def generateMessage(self):
        if self.query_object:
            return self.query_object.generateQuery(numberOfWords=random.randrange(1, 6))
        return ""

    def sendMessage(self, counts):
        time.sleep(5)

        print "sending packets:", counts
        addrTuple = (load_dir_ip, load_dir_port)
        while counts > 0:
            #print("Sending message " + message + " to IP: " + self.ip)
            time.sleep(sleep_time_between_sends)
            self.socket.sendto(self.generateMessage(), addrTuple)
            counts -= 1
        self.socket.close()
        print "Done!"

    def bind(self):
        self.socket.bind((self.ip, self.port))
        self.socket.settimeout(rec_timeout)

    def receiveData(self):
        t0 = time.time()
        counter = 0
        bytes_recv = 0

        print "Receiver: waiting for data...\n"
        while True:
            if counter == messages_to_send:
                break
            try:
                message = self.socket.recv(connection_buffer_size)
                counter += 1
            except timeout:
                print "done: Received " + str(counter) + " answers for " + str(messages_to_send) + " queries."
                break
        t1 = time.time()
        print "done: Received " + str(counter) + " answers for " + str(messages_to_send) + " queries."
        print "Time:", t1-t0


class ConnectionThread (threading.Thread):
    def __init__(self, threadID, name, port, query_object=None):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.query_object = query_object
        self.connection = Connection(port=port, query_object=query_object)

    def run(self):
        print "Starting " + self.name + "\n"
        if self.threadID > 0:
            self.connection.sendMessage(messages_to_send)

        elif self.threadID == 0:
            self.connection.bind()
            self.connection.receiveData()

        print str(self.threadID) + "Exiting " + self.name + "\n"


# --- CONFIG --- #
messages_to_send = 1000
sleep_time_between_sends = 0.001  # MAC
#sleep_time_between_sends = 0.01  # PI
number_of_sending_threads = 1
sending_thread_list = []
connection_buffer_size = 1024

my_rec_port = 32000
my_ip = "192.168.0.101"
rec_timeout = 15
load_dir_port = 32002
load_dir_ip = "192.168.0.200"
dictionary_path = "target/dictionary.txt"

# --- END OF CONFIG --- #

query_object = Query(dict_filepath=dictionary_path)

random.seed()

receiving_thread = ConnectionThread(threadID=0, name="Receiver", port=my_rec_port)
receiving_thread.start()


for i in range(1, number_of_sending_threads + 1):
    name = "Sender" + str(i)
    sending_thread_list.append(ConnectionThread(threadID=i, name=name, port=0))

for thread in sending_thread_list:
    thread.start()

for thread in sending_thread_list:
    if thread.isAlive():
        thread.join()
receiving_thread.join()

print "Exiting Main Thread"
