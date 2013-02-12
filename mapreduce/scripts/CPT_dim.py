#!/usr/bin/python
#CPT_dim.py
#prints the number of entries in a node's CPT

import sys
import re
import pickle as p

if len(sys.argv) < 3:
    print "\nUsage:\n python CPT_dim.py .dat_file node_1 node_2 ..."
    exit()

dat_file = sys.argv[1]
node_names = sys.argv[2:len(sys.argv)]

class node:
    def __init__(self):
        self.properties = None
        self.states = None
        self.name = None
    def theRightStuff(self, stuff):
        if not(eval(self.properties['adaptvartype']) == stuff[0]):
            return False
        if not(eval(self.properties['adaptcomponenttype']) == stuff[1]):
            return False
        return True

class potential:
    def __init__(self):
        self.data = None
        self.node = None
        self.parents = None
    def theRightStuff(self, stuff, nodes):
        return nodes[self.node].theRightStuff(stuff)

[nodes, potentials, pseudonym, name] = p.load(open(dat_file, 'r'))
#pseudonym is a dict from actual node names to their psudonyms (integer node numbers)
#name is the opposite

p_dict = dict()
for p in potentials:
    tmp = str(p.data)
    nest_level=0
    for i in tmp:
        if i != '(':
            break
        else:
            nest_level+=1
        
    tmp=re.sub('\(|\)', '', tmp)
    p_dict[p.node] = str(len(eval('['+tmp+']')))+'/'+str(nest_level)


ans=""
for n in node_names:
    ans = ans+" "+p_dict[n]

print ans








