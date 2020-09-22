import os
import sys
from os import listdir
from os.path import isfile, join
import xml.etree.ElementTree as ET
import subprocess
from multiprocessing import Pool
from distutils.dir_util import copy_tree
import shutil

# GLOBAL_LOG_FILE (will be .gitignored)
LOG_FILE = "./.timing.log"
TAG_PREFIX = "benchmark"
LATEST = "__LATEST__"
shouldRecompile = True
# require any test case program to end with '_source.txt'
# require any PQL Q&A to have same prefix as the program file minus the '_source'
class Arg:
  def __init__(self, source, query, result):
    self.source = source
    self.query = query
    self.result = result
  def dump(self):
    print(self.source + "," + self.query + "," + self.result)

def buildArgs(dir):
  files = [f for f in listdir(dir) if isfile(join(dir, f))]
  sources = []
  sourcesName = []
  for file in files:
    if (file.endswith("_source.txt")):
      sources.append(file)
      sourcesName.append(file.split("_source.txt")[0])
  testcases = []
  for source in sources:
    testcases.append([])
  for i, _ in enumerate(sources):
    source = sources[i]
    sourceName = sourcesName[i]
    for file in files:
      if file != source and file.startswith(sourceName + "_"):
        testcases[i].append(file)
  ret = []
  index = 0
  for i, _ in enumerate(sources):
    for testcase in testcases[i]:
      arg = Arg(join(dir, sources[i]), join(dir, testcase), join(dir, str(index)))
      index = index + 1
      ret.append(arg)
  return ret

def toString(xmlElement):
  if xmlElement.text == None:
    return ''
  else:
    return xmlElement.text

def clearResult(arg):
  os.system("rm -f " + arg.result)
  os.system("rm -f " + arg.result + "_output.txt")

rootdir = "./../Tests19/"
copydir = "./../tmp/"

copy_tree(rootdir, copydir)

subdirs = [o for o in os.listdir(copydir)
        if os.path.isdir(os.path.join(copydir,o))]
args = []
for subdir in subdirs:
  dir = copydir + subdir
  autotester_dir = "./build/src/autotester/autotester"
  args.extend(buildArgs(dir))


def runSingleTest(arg):
  result = arg.result.split("/")[-1]
  os.system(autotester_dir + " " + arg.source + " " + arg.query + " " + arg.result + " > " + arg.result + "_output.txt  2>&1")
  xml = ET.parse(join(arg.source.rsplit("/", 1)[0], result))
  root = xml.getroot()
  times = []
  for queries in root.iter('queries'):
    for query in queries:
      if len(query) < 6:
        if query[2].tag == "timeout":
          raise Exception("SPA timed out while evaluating PQL: " + query[1].text)
        else:
          print("Source program: " + arg.source)
        raise Exception("SPA throws exception while evaluating PQL: " + query[1].text)
      if query[5].tag == "failed":
        print("Source program: " + arg.source)
        if ('comment' in query[0].attrib):
          print("Test case: " + query[0].attrib['comment'])
        print("PQL: " + query[1].text)
        print("Expected: " + toString(query[3]))
        print("Got: " + toString(query[2]))
        raise Exception("Tests failed")
      else:
        times.append(float(toString(query[4])))
  print("All tests passed at :" + arg.query)
  times.sort()
  print("Slowest: " + str(times[-1]) + ", fastest: " + str(times[0]) + ", average: " + str(sum(times)/len(times)))
  clearResult(arg)
  return sum(times)

def runTests(isSingleThreaded):
  totalTime = 0
  if isSingleThreaded:
    print("Running test cases single-threadedly")
    for arg in args:
      totalTime += runSingleTest(arg)
  else:
    threadPool = Pool(5)
    times = threadPool.map(runSingleTest, args)
    totalTime = sum(times)
  print ("Total time: " + str(totalTime))
  return totalTime

def getAllTags():
  proc = subprocess.Popen(["git tag"], stdout=subprocess.PIPE, shell=True)
  (out, err) = proc.communicate()
  tags = []
  for tag in out.strip("\n").split("\n"):
    tag = tag.strip("\n")
    if tag.startswith(TAG_PREFIX):
      tags.append(tag)
  return tags

# we assume that file has tags recorded in newest to oldest order, the same order that `git tag` would return
def getRanTags():
  ret = []
  if not os.path.isfile(LOG_FILE):
    return ret
  with open(LOG_FILE) as f:
    for line in f.readlines():
      [tag, time] = line.strip("\n").split(" ")
      ret.append([tag, float(time)])
  return ret
# run tests that have all tags prefixed with benchmark, and also this latest change
# if there is any file not committed, it will be stashed and popped

def runTag(tag, isSingleThreaded):
  if tag == LATEST:
    if shouldRecompile == True:
      os.system("bash build.sh refresh")
    return runTests(isSingleThreaded)

  # else we need to stash and checkout
  needsToPop = True
  proc = subprocess.Popen(["git stash"], stdout=subprocess.PIPE, shell=True)
  (out, err) = proc.communicate()
  if out.startswith("No local changes to save"):
    needsToPop = False
  os.system("git checkout " + tag)
  print("\nCurrent tag: " + tag + "\n\n")
  os.system("bash build.sh refresh")
  timing = runTests(isSingleThreaded)
  os.system("git checkout -")
  if needsToPop:
    os.system("git stash pop")
  return timing

def updateLog(updatedTags):
  file = open(LOG_FILE, 'w+')
  for tag in updatedTags:
    [t, time] = tag
    file.write(t + " " + str(time) + "\n")

def updateLogFirstLine(updatedTags):
  lines = []
  if os.path.isfile(LOG_FILE):
    with open(LOG_FILE) as file:
      lines = file.readlines()
      if len(lines) > 0 and lines[0].startswith(LATEST):
        lines[0] = updatedTags[0][0] + " " + str(updatedTags[0][1])
      else:
        lines.insert(0, updatedTags[0][0] + " " + str(updatedTags[0][1]))
  else:
    lines.append(updatedTags[0][0] + " " + str(updatedTags[0][1]))
  with open(LOG_FILE, "w+") as f:
    for line in lines:
      f.write(line.strip("\n") + "\n")

def driver():
  isSingleThreaded = False
  earlyTermination = False
  if len(sys.argv) > 1 and int(sys.argv[1]) == 0:
    isSingleThreaded = True
    earlyTermination = True
    print("Running using single thread")
    print("Only test latest change")
  if len(sys.argv) > 1 and int(sys.argv[1]) == 1:
    earlyTermination = True
    global shouldRecompile
    shouldRecompile = False
    print("Only test latest change")
  if len(sys.argv) > 1 and int(sys.argv[1]) == 2:
    isSingleThreaded = True
    print("Test latest change and all tagged benchmarks")
  allTags = getAllTags()
  ranTags = getRanTags()
  updatedTags = [[LATEST, runTag(LATEST, isSingleThreaded)]]
  if earlyTermination:
    # we only run single threadedly usually in travis
    # and we do not want to do retrospect build
    if isSingleThreaded:
      updateLogFirstLine(updatedTags)
    return
  index = 0
  for tag in allTags:
    timing = None
    if len(ranTags) > index and ranTags[index][0] == tag:
      timing = ranTags[index][1]
      index = index + 1
    else:
      timing = runTag(tag, isSingleThreaded)
    updatedTags.append([tag, timing])
  if isSingleThreaded:
    updateLog(updatedTags)


driver()
# cleanup
shutil.rmtree(copydir)
