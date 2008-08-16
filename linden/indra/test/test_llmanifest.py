#!/usr/bin/python
# @file test_llmanifest.py
# @author Ryan Williams
# @brief Test cases for LLManifest library.
#
# Copyright (c) 2006-2007, Linden Research, Inc.
# 
# The source code in this file ("Source Code") is provided by Linden Lab
# to you under the terms of the GNU General Public License, version 2.0
# ("GPL"), unless you have obtained a separate licensing agreement
# ("Other License"), formally executed by you and Linden Lab.  Terms of
# the GPL can be found in doc/GPL-license.txt in this distribution, or
# online at http://secondlife.com/developers/opensource/gplv2
# 
# There are special exceptions to the terms and conditions of the GPL as
# it is applied to this Source Code. View the full text of the exception
# in the file doc/FLOSS-exception.txt in this software distribution, or
# online at http://secondlife.com/developers/opensource/flossexception
# 
# By copying, modifying or distributing this software, you acknowledge
# that you have read and understood your obligations described above,
# and agree to abide by those obligations.
# 
# ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
# WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
# COMPLETENESS OR PERFORMANCE.

from indra import llmanifest
import os.path
import os
import unittest

class DemoManifest(llmanifest.LLManifest):
    def construct(self):
        super(DemoManifest, self).construct()
        if self.prefix("dir_1"):
            self.path("test_a")
            self.path(src="test_b", dst="test_dst_b")
            self.path("*.test")
            self.path("*.tex", "*.jpg")
            if self.prefix("nested", dst=""):
                self.path("deep")
                self.end_prefix()
            self.end_prefix("dir_1")


class Demo_ArchManifest(llmanifest.LLManifest):
        pass

class TestLLManifest(unittest.TestCase):
    mode='static'
    def setUp(self):
        self.m = llmanifest.LLManifest("src", "dst", {'grid':'default', 'platform':'darwin', 'version':(1,2,3,4)})

    def testproperwindowspath(self):
        self.assertEqual(llmanifest.proper_windows_path("C:\Program Files", "cygwin"),"/cygdrive/c/Program Files")
        self.assertEqual(llmanifest.proper_windows_path("C:\Program Files", "windows"), "C:\Program Files")
        self.assertEqual(llmanifest.proper_windows_path("/cygdrive/c/Program Files/NSIS", "windows"), "C:\Program Files\NSIS")
        self.assertEqual(llmanifest.proper_windows_path("/cygdrive/c/Program Files/NSIS", "cygwin"), "/cygdrive/c/Program Files/NSIS")

    def testpathancestors(self):
        self.assertEqual(["dir"], [p for p in llmanifest.path_ancestors("dir")])
        self.assertEqual(["dir/sub", "dir"], [p for p in llmanifest.path_ancestors("dir/sub")])
        self.assertEqual(["dir/sub", "dir"], [p for p in llmanifest.path_ancestors("dir/sub/")])
        self.assertEqual(["dir/sub/two", "dir/sub", "dir"], [p for p in llmanifest.path_ancestors("dir/sub/two")])


    def testforplatform(self):
        self.assertEqual(llmanifest.LLManifest.for_platform('demo'), DemoManifest)
        def tmp_test():
            return llmanifest.LLManifest.for_platform('extant')
        self.assertRaises(KeyError, tmp_test)
        ExtantManifest = llmanifest.LLManifestRegistry('ExtantManifest', (llmanifest.LLManifest,), {})
        self.assertEqual(llmanifest.LLManifest.for_platform('extant'), ExtantManifest)
        self.assertEqual(llmanifest.LLManifest.for_platform('demo', 'Arch'), Demo_ArchManifest)


    def testprefix(self):
        self.assertEqual(self.m.get_src_prefix(), "src")
        self.assertEqual(self.m.get_dst_prefix(), "dst")
        self.m.prefix("level1")
        self.assertEqual(self.m.get_src_prefix(), "src/level1")
        self.assertEqual(self.m.get_dst_prefix(), "dst/level1")
        self.m.end_prefix()
        self.m.prefix(src="src", dst="dst")
        self.assertEqual(self.m.get_src_prefix(), "src/src")
        self.assertEqual(self.m.get_dst_prefix(), "dst/dst")
        self.m.end_prefix()

    def testendprefix(self):
        self.assertEqual(self.m.get_src_prefix(), "src")
        self.assertEqual(self.m.get_dst_prefix(), "dst")
        self.m.prefix("levela")
        self.m.end_prefix()
        self.assertEqual(self.m.get_src_prefix(), "src")
        self.assertEqual(self.m.get_dst_prefix(), "dst")
        self.m.prefix("level1")
        self.m.end_prefix("level1")
        self.assertEqual(self.m.get_src_prefix(), "src")
        self.assertEqual(self.m.get_dst_prefix(), "dst")
        self.m.prefix("level1")
        def tmp_test():
            self.m.end_prefix("mismatch")
        self.assertRaises(ValueError, tmp_test)

    def testruncommand(self):
        self.assertEqual("Hello\n", self.m.run_command("echo Hello"))
        def tmp_test():
            self.m.run_command("fff_garbage")
        self.assertRaises(RuntimeError, tmp_test)

    def testpathof(self):
        self.assertEqual(self.m.src_path_of("a"), "src/a")
        self.assertEqual(self.m.dst_path_of("a"), "dst/a")
        self.m.prefix("tmp")
        self.assertEqual(self.m.src_path_of("b/c"), "src/tmp/b/c")
        self.assertEqual(self.m.dst_path_of("b/c"), "dst/tmp/b/c")

    def testcmakedirs(self):
        self.m.cmakedirs("test_dir_DELETE/nested/dir")
        self.assert_(os.path.exists("test_dir_DELETE/nested/dir"))
        self.assert_(os.path.isdir("test_dir_DELETE"))
        self.assert_(os.path.isdir("test_dir_DELETE/nested"))
        self.assert_(os.path.isdir("test_dir_DELETE/nested/dir"))
        os.removedirs("test_dir_DELETE/nested/dir")

if __name__ == '__main__':
    unittest.main()
