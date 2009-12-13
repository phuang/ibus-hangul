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
import ibus
import locale
import gettext
import config
from keycapturedialog import KeyCaptureDialog

_ = lambda a : gettext.dgettext("ibus-hangul", a)

class Setup ():
    def __init__ (self):
	locale.bindtextdomain("ibus-hangul", config.localedir)
	locale.bind_textdomain_codeset("ibus-hangul", "UTF-8")

        self.__bus = ibus.Bus()
        self.__config = self.__bus.get_config()
	self.__config.connect("value-changed", self.on_value_changed, None)

	ui_file = os.path.join(os.path.dirname(__file__), "setup.ui")
	self.__builder = gtk.Builder()
	self.__builder.set_translation_domain("ibus-hangul")
	self.__builder.add_from_file(ui_file)

	# Hangul tab
	self.__hangul_keyboard = self.__builder.get_object("HangulKeyboard")
	model = gtk.ListStore(str, str, int)
	model.append([_("Dubeolsik"), "2", 0])
	model.append([_("Sebeolsik Final"), "3f", 1])
	model.append([_("Sebeolsik 390"), "39", 2])
	model.append([_("Sebeolsik No-shift"), "3s", 3])
	model.append([_("Sebeolsik 2 set"), "32", 4])
	model.append([_("Romaja"), "ro", 5])

	self.__hangul_keyboard.set_model(model)
	renderer = gtk.CellRendererText()
	self.__hangul_keyboard.pack_start(renderer)
	self.__hangul_keyboard.add_attribute(renderer, "text", 0)

	current = self.__read("HangulKeyboard", "2")
	for i in model:
	    if i[1] == current:
		self.__hangul_keyboard.set_active(i[2])
		break


	# hanja tab
	button = self.__builder.get_object("HanjaKeyListAddButton")
	button.connect("clicked", self.on_hanja_key_add, None)

	button = self.__builder.get_object("HanjaKeyListRemoveButton")
	button.connect("clicked", self.on_hanja_key_remove, None)

	model = gtk.ListStore(str)

	keylist_str = self.__read("HanjaKeys", "Hangul_Hanja,F9")
	self.__hanja_key_list_str = keylist_str.split(',')
	for i in self.__hanja_key_list_str:
	    model.append([i])

	self.__hanja_key_list = self.__builder.get_object("HanjaKeyList")
	self.__hanja_key_list.set_model(model)
	column = gtk.TreeViewColumn()
	column.set_title("key")
	renderer = gtk.CellRendererText()
	column.pack_start(renderer)
	column.add_attribute(renderer, "text", 0)
	self.__hanja_key_list.append_column(column)


	# advanced tab
	notebook = self.__builder.get_object("SetupNotebook")
	notebook.remove_page(2)

	# setup dialog
	self.__window = self.__builder.get_object("SetupDialog")
	icon_file = os.path.join(config.datadir, "ibus-hangul", "icons", "ibus-hangul.svg")
	self.__window.set_icon_from_file(icon_file)
	self.__window.connect("response", self.on_response, None)
	self.__window.show()

	ok_button = self.__builder.get_object("button_cancel")
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

	model = self.__hanja_key_list.get_model()
	str = ""
	iter = model.get_iter_first()
	while iter:
	    if len(str) > 0:
		str += ","
		str += model.get_value(iter, 0)
	    else:
		str += model.get_value(iter, 0)
	    iter = model.iter_next(iter)
	self.__write("HanjaKeys", str)

    def on_response(self, widget, id, data = None):
	if id == gtk.RESPONSE_APPLY:
	    self.apply()
	    widget.emit_stop_by_name("response")
	if id == gtk.RESPONSE_NONE:
	    widget.emit_stop_by_name("response")

    def on_ok(self):
	self.apply()

    def on_hanja_key_add(self, widget, data = None):
	dialog = KeyCaptureDialog(_("Select Hanja key"), self.__window)
	res = dialog.run()
	if res == gtk.RESPONSE_OK:
	    key_str = dialog.get_key_string()
	    if len(key_str) > 0:
		model = self.__hanja_key_list.get_model()
		iter = model.get_iter_first()
		while iter:
		    str = model.get_value(iter, 0)
		    if str == key_str:
			model.remove(iter)
			break
		    iter = model.iter_next(iter)

		model.append([key_str])
	dialog.destroy()

    def on_hanja_key_remove(self, widget, data = None):
	selection = self.__hanja_key_list.get_selection()
	(model, iter) = selection.get_selected()
	if model and iter:
	    model.remove(iter)

    def on_value_changed(self, config, section, name, value, data):
	if section == "engine/Hangul":
	    if name == "HangulKeyboard":
		model = self.__hangul_keyboard.get_model()
		for i in model:
		    if i[1] == value:
			self.__hangul_keyboard.set_active(i[2])
			break
	    elif name == "HanjaKeys":
		self.__hanja_key_list_str = value.split(',')

    def __read(self, name, v):
        return self.__config.get_value("engine/Hangul", name, v)

    def __write(self, name, v):
        return self.__config.set_value("engine/Hangul", name, v)

if __name__ == "__main__":
    gettext.bindtextdomain("ibus-hangul", config.localedir)
    Setup().run()
