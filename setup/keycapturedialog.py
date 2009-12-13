import gtk
import gtk.gdk as gdk
import gettext

_ = lambda a : gettext.dgettext("ibus-hangul", a)

class KeyCaptureDialog ():
    def __init__ (self, title, parent):
	self.__key_str = ''
	self.__dialog = gtk.MessageDialog(parent,
			gtk.DIALOG_MODAL,
			gtk.MESSAGE_INFO,
			gtk.BUTTONS_OK_CANCEL,
			"")
	self.__dialog.set_markup(_("Press any key which you want to use as hanja key. "
                "The key you pressed is displayed below.\n"
                "If you want to use it, click \"Ok\" or click \"Cancel\""))

	self.__dialog.format_secondary_markup(" ")
	self.__dialog.connect("key-press-event", self.on_keypress, None)

    def destroy(self):
	self.__dialog.destroy()

    def run(self):
	return self.__dialog.run()

    def get_key_string(self):
	return self.__key_str

    def on_keypress(self, widget, event, data = None):
	self.__key_str = ""
	if event.state & gdk.CONTROL_MASK :
	    self.__key_str += "Control+"
	if event.state & gdk.MOD1_MASK :
	    self.__key_str += "Alt+"
	if event.state & gdk.SHIFT_MASK :
	    self.__key_str += "Shift+"

	self.__key_str += gdk.keyval_name(event.keyval)
	    
	self.__dialog.format_secondary_markup('<span size="large" weight="bold">%s</span>' % self.__key_str)
