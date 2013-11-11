import sys

# parses a patent file
def parsefile(filename):
    print "opening", filename
    file = open(filename, "r")
    data = file.read()

    return data

def main(arg):
    parsefile(arg)
#    parse_query(arg)

if __name__ == '__main__':
    filename = "patsnap-corpus/EP0049154B2.xml"
    if len(sys.argv) > 1:
        filename = sys.argv[1]
    main(filename)
