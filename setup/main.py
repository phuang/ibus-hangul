# vim:set et sts=4 sw=4:
#
# ibus-hangul - The Hangul Engine For IBus
#
# Copyright (c) 2009 Choe Hwanjin <choe.hwanjin@gmail.com>
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

import sys
import os
import gobject
import gtk
import gtk.glade as glade
import ibus
import gettext
import config

_ = lambda a : gettext.dgettext("ibus-hangul", a)

class Setup ():
    def __init__ (self):
        self.__bus = ibus.Bus()
        self.__config = self.__bus.get_config()
	self.__config.connect("value-changed", self.on_value_changed, None)

	glade.bindtextdomain("ibus-hangul", config.localedir)
	glade_file = os.path.join(os.path.dirname(__file__), "setup.glade")
	self.__xml = glade.XML(glade_file, domain="ibus-hangul")

	self.__hangul_keyboard = self.__xml.get_widget("HangulKeyboard")
	model = gtk.ListStore(str, str, int)
	model.append([_("Dubeolsik"), "2", 0])
	model.append([_("Sebeolsik Final"), "3f", 1])
	model.append([_("Sebeolsik 390"), "39", 2])
	model.append([_("Sebeolsik No-shift"), "3s", 3])
	model.append([_("Sebeolsik 2 set"), "32", 4])
	model.append([_("Romaja"), "ro", 5])
	self.__hangul_keyboard.set_model(model)

	current = self.__read("HangulKeyboard", "2")
	for i in model:
	    if i[1] == current:
		self.__hangul_keyboard.set_active(i[2])
		break

	self.__window = self.__xml.get_widget("dialog")
	icon_file = os.path.join(config.datadir, "ibus-hangul", "icons", "ibus-hangul.svg")
	self.__window.set_icon_from_file(icon_file)
	self.__window.connect("response", self.on_response, None)
	self.__window.show()

	ok_button = self.__xml.get_widget("button_ok")
	ok_button.grab_focus()

    def run(self):
	res = self.__window.run()
	if (res == gtk.RESPONSE_OK):
	    self.on_ok()
	self.__window.destroy()

    def apply(self):
	model = self.__hangul_keyboard.get_model()
	i = self.__hangul_keyboard.get_active()
	self.__write("HangulKeyboard", model[i][1])

    def on_response(self, widget, id, data = None):
	if (id == gtk.RESPONSE_APPLY):
	    self.apply()
	    widget.emit_stop_by_name("response")

    def on_ok(self):
	self.apply()

    def on_value_changed(self, config, section, name, value, data):
	if section == "engine/Hangul":
	    if name == "HangulKeyboard":
		model = self.__hangul_keyboard.get_model()
		for i in model:
		    if i[1] == value:
			self.__hangul_keyboard.set_active(i[2])
			break

    def __read(self, name, v):
        return self.__config.get_value("engine/Hangul", name, v)

    def __write(self, name, v):
        return self.__config.set_value("engine/Hangul", name, v)

if __name__ == "__main__":
    gettext.bindtextdomain("ibus-hangul", config.localedir)
    Setup().run()
