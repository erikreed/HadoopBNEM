#hide_vars.py

import sys
import pickle as p

if len(sys.argv) < 4:
    print "\nUsage:\n python hide_vars.py [input .tab filename] [input .dat filename] [output .tab filename] [node to hide #1] [node to hide #2] ..."
    exit()

infile = sys.argv[1]
data_file = sys.argv[2]
outfile = sys.argv[3]
vars_to_hide = sys.argv[4:len(sys.argv)]

class node:
    def __init__(self):
        self.properties = None
        self.states = None
        self.name = None

class potential:
    def __init__(self):
        self.data = None
        self.node = None
        self.parents = None

class evidence:
    def __init__(self, data):
        tmp = data.split('\n')
        self.date = tmp[1].lstrip('<instantiation date="').rstrip('">')
        #print self.date
        self.nodes = dict()
        for i in tmp[2:len(tmp)-1]:
            i = i.lstrip('<inst id="').rstrip('"/>').split('" value = "')
            self.nodes[i[0]] = i[1]
        #print self.nodes

[nodes, potentials, pseudonym, name] = p.load(open(data_file, 'r'))

#pseudonym is a dict from actual node names to their psudonyms (integer node numbers)
#name is the opposite

pseudonyms_of_hidden_vars = set()
for i in vars_to_hide:
    if not (i in pseudonym):
        print "The key: ", i, "is not in the set of known pseduonyms"
    else:
        pseudonyms_of_hidden_vars.add(pseudonym[i])

print pseudonyms_of_hidden_vars

#for now, assume all pseudonyms are listed from 0 to k, in order, 
#everything is easy, everything is observed
indeces_of_hidden_vars = sorted(list(pseudonyms_of_hidden_vars))
infile = open(infile, 'r')
outfile = open(outfile, 'w')

l=0
for line in infile:
    if l!=1:
        line_data = line.rstrip().split('\t')
        for i in range(len(indeces_of_hidden_vars)):
            line_data.pop(indeces_of_hidden_vars[i]-i)
        outfile.write('\t'.join(line_data))
        outfile.write('\n')
    else:
        outfile.write('\n')
    l+=1

