import os
import sys

def getRevision(path):
  cmd = os.popen("svn info " + path)

  while True:
    line = cmd.readline()
    if line == "":
      raise Exception("failed to obtain revision number")
    if line.startswith("Revision: "):
      revision = int(line[10:])
      return revision
      