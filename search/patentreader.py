import sys
import lxml
from bs4 import BeautifulSoup

# parses a query file
def parse_query(filename):
    file = open(filename)
    # first line should be doctype line
    markup = file.readline()
    # try to fix markup to valid xml...
    # encapsulate the content in a single tag
    markup += "<doc>\n"
    line = file.readline()
    while line:
        markup += line
        line = file.readline()
    # end tag
    markup += "</doc>\n"
    # Hope this works!
    # parse the markup
    soup = BeautifulSoup(markup, ["lxml", "xml"])
    # get title tag
    title = soup.doc.title.string.strip().lower()
    # description tag
    desc = soup.doc.description.string.strip().lower()
    #print title, desc
    return title, desc

def isAbstract(tag):
    if tag == None:
        return False
    return tag.has_key("name") and tag['name']=="Abstract"

def isTitle(tag):
    if tag == None:
        return False
    return tag.has_key("name") and tag['name']=="Title"

def isPatentNum(tag):
    if tag == None:
        return False
    return tag.has_key("name") and tag["name"]=="Patent Number"

def isIPC(tag):
    if tag == None:
        return False
    return tag.has_key("name") and tag["name"]=="IPC Subclass"

# parses a patent file
def parsefile(filename):
    print "opening", filename
    file = open(filename)
    data = file.read()
    soup = BeautifulSoup(data, ["lxml", "xml"])
    abstract = ""
    title = "" 
    ptn = ""

    # select patentnumber tag
    for tag in soup.doc.find(isPatentNum):
        ptn = tag.string.strip()

    # select title tag
    for tag in soup.doc.find(isTitle):
        title = tag.string.strip()

    # there is not always an abstract tag, but try to find it
    tags = soup.doc.find(isAbstract)
    if tags:
        for tag in tags:
            abstract = tag.string.strip()

    ipc = []
    tags = soup.doc.find(isIPC)
    if tags:
        for tag in tags:
            temp = tag.string.strip().split("|")
            for string in temp:
                ipc.append(string.strip())

    return ptn, title, abstract, ipc

def main(arg):
    parsefile(arg)
#    parse_query(arg)

if __name__ == '__main__':
    filename = "patsnap-corpus/EP0049154B2.xml"
    if len(sys.argv) > 1:
        filename = sys.argv[1]
    main(filename)
