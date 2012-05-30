#read_net.py

import sys
import pickle as p

infile = sys.argv[1]
if len(sys.argv) <= 1:
    print 'usage: [net file][optional: number of shared params]'
class chunk:
    def __init__(self, startline, name):
        self.startline = startline
        self.name = name
        self.text = []
    def insert_line(self, line):
        self.text.append(line)

class node:
    def __init__(self, chunk):
        lines = chunk.text
        tmp = [x.split(' = ') for x in lines]
        for i in tmp:
            i[0] = i[0].strip('\t').lstrip()
            i[1] = i[1].strip(';\n').strip('\t').lstrip()
        #self.properties = tmp
        self.properties = dict()
        for i in tmp:
            self.properties[i[0]] = i[1]
        tmp = self.properties['states'].replace('" "', '", "').replace('(,', '(').replace(',)', ')')
        self.states = list(eval(tmp))
        self.name = chunk.name[0].rstrip()

class potential:
    def __init__(self, chunk):
        tmp = chunk.text
        tmp2 = ''
        for i in tmp:
            tmp2 += ' '.join(i.split()).strip().lstrip('data =').rstrip(';').replace('\t', ',').replace('(,','(').replace(',)', ')')
        tmp2 = tmp2.replace(')(','),(').replace(' ',',').replace(',,', ',').replace('(,','(').replace(',)', ')')
        self.data = eval(tmp2)
        tmp = ' '.join(chunk.name)
        #print tmp
        tmp = tmp.replace('(','').replace(')','').replace('|',' ').split()
        #print tmp
        self.node = tmp[0]
        #print self.data
        #print self.node
        self.parents = [tmp[x] for x in range(1,len(tmp))]
        #print self.parents
        share(self)

#a map from node parameter tuple -> list of all node names that share identical parameters
sharable = dict()
def share(p):
    if p.data in sharable:
        sharable[p.data].append(p.node)
    else:
        sharable[p.data] = [p.node]

nodes = []
potentials = []

#Collect all nodes and potentials
lnum = 0
prevline = ''
chunk_active=False
thisChunk = None
chunkType = None
for line in open(infile, 'r'):
    if lnum==0 and line != "net\n":
        print "This file doesn't pass my very basic test to see if it's a net file. Fail."
        exit()
    elif chunk_active:
        if line == "}\n":
            chunk_active = False
            if thisChunk.startline == 1:
                pass
            elif chunkType == 'node':
                nodes.append(node(thisChunk))
            else:
                potentials.append(potential(thisChunk))
        else:
            thisChunk.insert_line(line.split('%')[0])
    elif line == "{\n":
        chunk_active=True
        chunkName = prevline.split(' ')[1:]
        thisChunk = chunk(lnum, chunkName)
        chunkType = prevline.split(' ')[0]
        #print "New chunk: ", chunkType, chunkName        
    prevline = line
    lnum +=1

#print 'We have', len(nodes), 'nodes and', len(potentials), 'potentials.'

#assign unique integer pseudonyms to the nodes
pseudonym = dict()
name = dict()
for i in range(len(nodes)):
    pseudonym[nodes[i].name] = i
    name[i] = nodes[i].name

#make nodes a dict where the name will access the informtion
tmp = dict()
for i in nodes:
    tmp[i.name] = i
nodes = tmp

#convert a disgusting tuple into a pretty list
#hopefully, this list will be in the right order
def n_dim_to_list(t):
    ans =[]
    if not(type(t) is tuple):
        ans.append(t)
    else:
        ans = []
        tmp = [n_dim_to_list(x) for x in t]
        for i in tmp:
            ans.extend(i)
    return ans

#print out factor block i
def print_potential(i, f):
    ans = ''
    n = potentials[i].node
    p = potentials[i].parents
    ans += str(1 + len(p)) + '\n'
    ans += str(pseudonym[n]) + ' '
    p.reverse()
    for j in p:
        ans += str(pseudonym[j]) + ' '
    ans += '\n'
    l = 1
    for j in [n] + p:
        tmp = len(nodes[j].states)
        ans += str(tmp) + ' '
        l *= tmp
    ans += '\n' + str(l) + '\n'
    data =  n_dim_to_list(potentials[i].data)
    assert (len(data) == l)
    for j in range(l):
        ans += str(j) + ' ' + str(data[j]) + '\n'
    ans += '\n'
    f.write(ans)
sharednums = set()
if len(sys.argv) == 3:
    numvars = sys.argv[2]
    for s in sharable.values():
        if str(len(s)) == numvars:
            print ' '.join(s)
            exit()
    raise Exception('no shared vars of length ' + str(numvars) + ' found...')
for s in sharable.values():
    sharednums.add(len(s))
    #print len(s)
    #print ' '.join(s)
out =''
for s in sharednums:
    if s > 1:
        out += str(s) + ' '
print out
