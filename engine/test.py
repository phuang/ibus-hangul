#!/usr/bin/env python
# -*- coding: utf-8 -*-
import hangul
import sys
sys.path.append(".libs")

ctx = hangul.HangulInputContext("2")
ctx.process(ord('a'))
ctx.process(ord('b'))
ctx.process(ord('c'))
print ctx.get_preedit_string()
print ctx.get_commit_string()
print ctx.flush()
print ctx.get_preedit_string()
print ctx.get_commit_string()
print ctx.flush()
ctx = None

table = hangul.HanjaTable("/usr/share/libhangul/hanja/hanja.txt")
v = table.match_prefix("가례")
if v:
	k, l = v
	print k
	for v, c in l:
		print v, c
