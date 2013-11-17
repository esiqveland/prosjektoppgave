# load_common.py
import socket
import random
import threading
import time
import struct

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
    def __init__(self, my_ip, target_ip, target_port, port, query_object=None):
        self.ip = my_ip
        self.target_ip = target_ip
        self.port = port
        self.target_port = target_port
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.bufferSize = 1024
        self.query_object = query_object

    def getConnectionSocket(self):
        return self.socket

    def getConnectionAddrTuple(self):
        return (self.ip, self.port)

    def generateMessage(self):
        if self.query_object:
            return self.query_object.generateQuery(numberOfWords=random.randrange(1, 6))
        return ""

    def sendMessage(self, counts, sleeptime):
        time.sleep(5)

        print "sending packets:", counts
        addrTuple = (self.target_ip, self.target_port)
        print "target:", addrTuple
        while counts > 0:
            #print("Sending message " + message + " to IP: " + self.ip)
            time.sleep(sleeptime)
            msg = self.generateMessage()
            if self.target_port % 32000 == 3:
                msg = struct.pack("=4sH1024s", socket.inet_aton(self.ip), socket.htons(self.port), msg)
            self.socket.sendto(msg, addrTuple)
            counts -= 1
        self.socket.close()
        print "Done!"

    def bind(self):
        print "Listening on:", (self.ip, self.port)
        self.socket.bind((self.ip, self.port))
        self.socket.settimeout(15)

    def receiveData(self, count):
        t0 = time.time()
        counter = 0
        bytes_recv = 0

        print "Receiver: waiting for data...\n"
        while True:
            if counter == count:
                break
            try:
                message = self.socket.recv(1024)
                counter += 1
            except socket.timeout:
                print "done: Received " + str(counter) + " answers for " + str(count) + " queries."
                break
        t1 = time.time()
        print "done: Received " + str(counter) + " answers for " + str(count) + " queries."
        print "Time:", t1-t0

class ConnectionThread (threading.Thread):
    def __init__(self, threadID, name, my_ip, target_ip, sleeptime, nmessages, target_port, port=32003, query_object=None):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.sleeptime = sleeptime
        self.nmessages = nmessages
        self.query_object = query_object
        self.connection = Connection(my_ip=my_ip, target_ip=target_ip, target_port=target_port, port=port, query_object=query_object)

    def run(self):
        print "Starting " + self.name + "\n"
        self.connection.sendMessage(self.nmessages, self.sleeptime)

        print str(self.threadID) + "Exiting " + self.name + "\n"

class ReceiverThread (threading.Thread):
    def __init__(self, threadID, name, nmessages, my_ip, port=32003):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.nmessages = nmessages
        self.connection = Connection(my_ip=my_ip, target_ip="0.0.0.0", port=port, target_port=32002)

    def run(self):
        print "Starting " + self.name + "\n"
        self.connection.bind()
        self.connection.receiveData(self.nmessages)
        print str(self.threadID) + "Exiting " + self.name + "\n"


