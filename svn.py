import os
import sys

def getRevision(path):
  cmd = os.popen("svn info " + path)

  while True:
    line = cmd.readline()
    if line == "":
      raise Exception("failed to obtain revision number")
    for word in line.split():
      try:
        return int(word)
      except:
        pass

