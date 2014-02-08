# -*- coding: utf-8 -*-

import unittest

from subprocess import call
from tempfile import mkdtemp
from shutil import rmtree

from proctest import ProcTest
from subprocess import Popen, PIPE

class TrimProcTest(ProcTest):
    monodix = "data/minimal-mono.dix"
    monodir = "lr"
    bidix = "data/minimal-bi.dix"
    bidir = "lr"
    procflags = ["-z"]

    def runTest(self):
        tmpd = mkdtemp()
        try:
            self.assertEqual(0, call(["../lttoolbox/lt-comp",
                                      self.monodir,
                                      self.monodix,
                                      tmpd+"/mono.bin"],
                                     stdout=PIPE))
            self.assertEqual(0, call(["../lttoolbox/lt-comp",
                                      self.bidir,
                                      self.bidix,
                                      tmpd+"/bi.bin"],
                                     stdout=PIPE))
            self.assertEqual(0, call(["../lttoolbox/lt-trim",
                                      tmpd+"/mono.bin",
                                      tmpd+"/bi.bin",
                                      tmpd+"/trimmed.bin"],
                                     stdout=PIPE))

            self.proc = Popen(["../lttoolbox/lt-proc"] + self.procflags + [tmpd+"/trimmed.bin"],
                              stdin=PIPE,
                              stdout=PIPE,
                              stderr=PIPE)

            for inp,exp in zip(self.inputs, self.expectedOutputs):
                self.assertEqual( self.communicateFlush(inp+"[][\n]"),
                                  exp+"[][\n]" )

            self.proc.communicate() # let it terminate
            self.proc.stdin.close()
            self.proc.stdout.close()
            self.proc.stderr.close()
            self.assertEqual( self.proc.poll(),
                              self.expectedRetCode )


        finally:
            rmtree(tmpd)


class TrimNormalAndJoin(unittest.TestCase, TrimProcTest):
    inputs = ["abc", "ab", "y", "n", "jg", "jh", "kg"]
    expectedOutputs = ["^abc/ab<n><def>$", "^ab/ab<n><ind>$", "^y/y<n><ind>$", "^n/*n$", "^jg/j<pr>+g<n>$", "^jh/*jh$", "^kg/*kg$"]
    expectedRetCode = 0

class TrimCmp(unittest.TestCase, TrimProcTest):
    inputs = ["a", "b", "c", "d", "aa", "ab", "ac", "ad", "ba", "bb", "bc", "bd", "ca", "cb", "cc", "cd", "da", "db", "dc", "dd", ]
    expectedOutputs = ["^a/*a$", "^b/b<n>$", "^c/*c$", "^d/d<n>$", "^aa/*aa$", "^ab/a<n>+b<n>$", "^ac/*ac$", "^ad/a<n>+d<n>$", "^ba/*ba$", "^bb/*bb$", "^bc/*bc$", "^bd/*bd$", "^ca/*ca$", "^cb/d<n>+b<n>$", "^cc/*cc$", "^cd/d<n>+d<n>$", "^da/*da$", "^db/*db$", "^dc/*dc$", "^dd/*dd$"]
    expectedRetCode = 0
    monodix = "data/cmp-mono.dix"
    bidix = "data/cmp-bi.dix"
    procflags = ["-e", "-z"]

class TrimLongleft(unittest.TestCase, TrimProcTest):
    inputs = ["herdende"]
    expectedOutputs = ["^herdende/herde<adj><pprs>$"]
    expectedRetCode = 0
    monodix = "data/longleft-mono.dix"
    bidix = "data/longleft-bi.dix"

class BidixPardef(unittest.TestCase, TrimProcTest):
    inputs = ["c"]
    expectedOutputs = ["^c/c<vblex><inf>$"]
    expectedRetCode = 0
    monodix = "data/bidixpardef-mono.dix"
    bidir = "rl"
    bidix = "data/bidixpardef-bi.dix"

class UnbalancedEpsilons(unittest.TestCase, TrimProcTest):
    inputs = ["re", "rer", "res", "ret"]
    expectedOutputs = ["^re/re<vblex><inf>$", "^rer/re<vblex><pres>$", "^res/re<vblex><pres>$", "^ret/re<vblex><pret>$"]
    expectedRetCode = 0
    monodix = "data/unbalanced-epsilons-mono.dix"
    bidir = "rl"
    bidix = "data/unbalanced-epsilons-bi.dix"

class Group(unittest.TestCase, TrimProcTest):
    inputs = ["abc"]
    expectedOutputs = ["^abc/ab<n><ind>#c$"]
    expectedRetCode = 0
    monodix = "data/group-mono.dix"
    bidix = "data/group-bi.dix"


class Empty(unittest.TestCase, TrimProcTest):
    def runTest(self):
        tmpd = mkdtemp()
        try:
            self.assertEqual(0, call(["../lttoolbox/lt-comp",
                                      "lr",
                                      "data/empty-mono.dix",
                                      tmpd+"/empty-mono.bin"],
                                     stdout=PIPE))
            self.assertEqual(0, call(["../lttoolbox/lt-comp",
                                      "rl", # rl!
                                      "data/empty-bi.dix",
                                      tmpd+"/empty-bi.bin"],
                                     stdout=PIPE))
            self.assertEqual(1, call(["../lttoolbox/lt-trim",
                                      tmpd+"/empty-mono.bin",
                                      tmpd+"/empty-bi.bin",
                                      tmpd+"/empty-trimmed.bin"],
                                     stdout=PIPE,
                                     stderr=PIPE))

        finally:
            rmtree(tmpd)
