#summarize_net.py

import sys
import pickle as p

if len(sys.argv) < 2:
    print "Usage: python summarize_net.py dat_file"
    exit()

dat_file = sys.argv[1]

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

[nodes, potentials, pseudonym, name] = p.load(open(dat_file, 'r'))
#pseudonym is a dict from actual node names to their psudonyms (integer node numbers)
#name is the opposite

#print "Node names:"
#print nodes.keys()
"""
for n in nodes.values():
    print "Node name:", n.name
    print "adaptvartype:", eval(n.properties['adaptvartype'])
    print "adaptcomponenttype:", eval(n.properties['adaptcomponenttype'])
    print "states:", n.states
"""

print pseudonym
