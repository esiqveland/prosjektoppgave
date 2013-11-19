# TODO: Do project...
import os
import nltk
import struct
import re
import sys
import getopt
import patentreader
import pickle

# FLAG
phrase = 0

# Blame user for not understanding
def usage():
    print "usage: " + sys.argv[0] + " -i path-to-corpus/ -d dictionary.txt -p postings.txt -m mapping_file"

def makeDict(dictionary, tokens, index):
    for token in tokens:
        term = re.sub('[^a-z0-9]', '', token)
        term = stemmer.stem(term)
        if term == '':
            continue

        if term in dictionary:
            if dictionary[term][index] is not None:
                if docID in dictionary[term][index][0]:
                    iposinTitle = dictionary[term][index][0].index(docID)
                    dictionary[term][index][1][iposinTitle] = dictionary[term][index][1][iposinTitle] + 1
                else:
                    dictionary[term][index][0].append(docID)
                    dictionary[term][index][1].append(1)
            else:
                if index == 0:
                    dictionary[term] = (([docID],[1]),dictionary[term][1])
                else:
                    dictionary[term] = (dictionary[term][0],([docID],[1]))
        else:
            if index == 0:
                dictionary[term] = (([docID],[1]),None)
            else:
                dictionary[term] = (None,([docID],[1]))


# Handle input arguments
mapping_file = input_file_p = input_file_d = input_i = None

try:
    opts, args = getopt.getopt(sys.argv[1:], 'i:d:p:m:x')
except getopt.GetoptError, err:
    usage()
    sys.exit(2)
for o, a in opts:
    if o == '-i':
        input_i = a
    elif o == '-d':
        input_file_d = a
    elif o == '-p':
        input_file_p = a
    elif o == '-x':
        phrase = 1
    elif o == '-m':
        mapping_file = a
    else:
        assert False, "unhandled option"

print input_file_p, input_file_d, input_i
if input_file_p == None or input_file_d == None or input_i == None:
    usage()
    sys.exit(2)

#-------    VARIABLES   -------#

dataPath = input_i
postDataDict = dict()
dictionary = dict()
stemmer = nltk.PorterStemmer()

files = os.listdir(dataPath)
allFilesList = []


#------- IMPLEMENTATION -------#


# Open the files one by one, tokenize the words, and generate the postDataDict
docID = 0
numdocs = 0
numterms = 0
docid_mapping = dict()
for file in files:
    if file == '.DS_Store':
        continue
    # Print progress
    progress = float(numdocs)/float(len(files)) * 100
    progressStr = str(progress)
    print progressStr[:5] + "%"

    #allFilesList.append(int(file))
    title = patentreader.parsefile(dataPath+file)
    docid_mapping[numdocs] = (file, numdocs)
    # set docID = numdocs if you want to do unique filename --> ID convertion
    docID = int(file)

    title = nltk.sent_tokenize(title.lower())
    filepos = 0
    for sentence in title:

        tokens = nltk.word_tokenize(sentence)

        # Perform stemming and replacement on token, then add to bigList
        makeDict(dictionary,tokens,0)
    numdocs += 1

print "100%"

print "Writing files to disk..."

# Write postDataDict to file as binery
post_file = open(input_file_p,'wb')
dict_file = open(input_file_d,'w')

for term in dictionary:
    dict_file.write(term + ' ')
    dict_file.write(str(post_file.tell()) + ' ')
    # If term is found in titles then write to dict and post
    if dictionary[term][0] is not None:
        dict_file.write(str(len(dictionary[term][0][0])) + ' ')
        titleTuple = dictionary[term][0]
        for i in range(0,len(titleTuple[0])):
            post_file.write(struct.pack('i',titleTuple[0][i]))
            post_file.write(struct.pack('i',titleTuple[1][i]))
    # If term not found write standard value -1 -1 and nothing to post
    else:
        dict_file.write('0 ')
    dict_file.write('\n')

dict_file.write('* 0 ' + str(numdocs) + " " + str(numdocs) + "\n")
dict_file.close()
post_file.close()

# Write Dictionary to file as text
#dict_file = open(input_file_d,'w')
#for k,(c,o) in dictionary.iteritems():
#    dict_file.write(k + " " + str(c) + " " + str(o) + "\n")
#
#dict_file.close()


docid_mapping_out = open(mapping_file,'w')
pickle.dump(docid_mapping,docid_mapping_out)
docid_mapping_out.close()




print "Done"


