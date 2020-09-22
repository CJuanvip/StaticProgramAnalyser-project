import os
import sys
from os import listdir
from os.path import isfile, join

class Arg:
  def __init__(self, source, queries):
    self.source = source
    self.queries = queries
  def dump(self):
    print(self.source + ";")
    print(self.queries)

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
  for i, _ in enumerate(sources):
    arg = Arg(join(dir, sources[i]), [join(dir, testcase) for testcase in testcases[i]], )
    ret.append(arg)
  return ret

ROOT_DIR = "../Tests19/"
OUT_DIR = "../Tests19/out/"

subdirs = [o for o in os.listdir(ROOT_DIR)
        if os.path.isdir(os.path.join(ROOT_DIR,o))]
args = []
for subdir in subdirs:
  args.extend(buildArgs(ROOT_DIR + subdir))
for i in range(len(args)):
  out_src = join(OUT_DIR, str(i) + "_source.txt")
  out_qry = join(OUT_DIR, str(i) + "_queries.txt")
  arg = args[i]
  with open(arg.source, 'r') as f_src:
    content = f_src.read()
    split = content.split("\n")
    content = ""
    for s in split:
      content += s.split("//")[0].split("\\")[0] + "\n"
  with open(out_src, "w+") as f_out_src:
    f_out_src.write(content)
  content = ""
  for qry in arg.queries:
    with open(qry, "r") as f_qry:
      read = f_qry.read()
      while(read[-1] == "\n" and read[-2] == "\n"):
        read = read[:-1]
      if read[-1] != "\n":
        read = read + "\n"
      content += read
  with open(out_qry, "w+") as f_out_qry:
    f_out_qry.write(content)
  content = ""
