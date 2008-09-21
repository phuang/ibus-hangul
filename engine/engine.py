# vim:set et sts=4 sw=4:
# -*- coding: utf-8 -*-
#
# ibus-hangul - The Hangul engine for IBus
#
# Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

import gobject
import ibus
import hangul
from ibus import keysyms
from ibus import modifier

_ = lambda a: a

class Engine(ibus.EngineBase):

    # init hanja table
    __hanja_table = None

    def __init__(self, bus, object_path):
        super(Engine, self).__init__(bus, object_path)
        # if Engine.__hanja_table == None:
        #     table = "/usr/share/libhangul/hanja/hanja.txt"
        #     Engine.__hanja_table = \
        #         hangul.HanjaTable(table)

        # create anthy context
        self.__context = hangul.HangulInputContext("2")
        self.__reset()

    # reset values of engine
    def __reset(self):
        self.__context.reset()

    def page_up(self):
        return True

    def page_down(self):
        return True

    def cursor_up(self):
        return True

    def cursor_down(self):
        return True

    def __flush(self):
        text = self.__context.flush()
        self.hide_preedit()
        self.commit_string(text)

    def __update_preedit(self):
        preedit_string = self.__context.get_preedit_string()
        if preedit_string:
            attrs = ibus.AttrList()
            l = len(preedit_string)
            attrs.append(ibus.AttributeForeground(0xffffff, 0, l))
            attrs.append(ibus.AttributeBackground(0, 0, l))
            self.update_preedit(preedit_string, attrs, l, True)
        else:
            self.hide_preedit()

    def __commit_current(self):
        commit_string = self.__context.get_commit_string()
        if commit_string:
            self.commit_string(commit_string)

    def process_key_event(self, keyval, is_press, state):
        # ignore key release events
        if not is_press:
            return False

        if state & (modifier.CONTROL_MASK | modifier.MOD1_MASK):
            return False

        res = False
        if keyval == keysyms.BackSpace:
            res = self.__context.backspace()
            if res:
                self.__update_preedit()
        else:
            if state & modifier.LOCK_MASK:
                # toggle case
                c = unichr(keyval)
                if c.islower():
                    keyval = ord(c.upper())
                else:
                    keyval = ord(c.lower())

            res = self.__context.process(keyval)
            self.__update_preedit()
            self.__commit_current()
        return res

    def property_activate(self, prop_name, state):
        pass

    def focus_in(self):
        pass

    def focus_out(self):
        self.__flush()
