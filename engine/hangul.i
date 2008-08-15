/* vim:set et ts=4: */
/*
 * ibus-hangul - The Hangul engine for IBus
 *
 * Copyright (c) 2007-2008 Huang Peng <shawn.p.huang@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

%module hangul
%{
 /* Put header files here or function declarations like below */
#include <wchar.h>
#include <hangul.h>
%}

%init %{
%}

%typemap (in) ucschar * {
    if (PyUnicode_Check ($input)) {
        Py_ssize_t size = PyUnicode_GetSize ($input);
        $1 = (ucschar *)malloc ((size + 1) * sizeof (ucschar));
        PyUnicode_AsWideChar ((PyUnicodeObject *)$input, (wchar_t *)$1, size);
        $1[size] = 0;
    }
    else {
        PyErr_SetString (PyExc_TypeError,
            "arg msut be unistr");
        return NULL;
    }
}

%typemap(freearg) ucschar * {
    free ($1);
}

%typemap (out) ucschar * {
    if ($1 != NULL) {
        $result = PyUnicode_FromWideChar ((const wchar_t *)$1,
                    wcslen ((const wchar_t *)$1));
    }
    else {
        Py_INCREF (Py_None);
        $result = Py_None;
    }
}

/* define exception */
%exception {
    $action
    if (PyErr_Occurred ()) {
        return NULL;
    }
}

typedef int ucschar;

/* define struct HangulKeyboard */
typedef struct {} HangulKeyboard;
%extend HangulKeyboard {
    HangulKeyboard () {
        return hangul_keyboard_new ();
    }

    ~HangulKeyboard () {
        hangul_keyboard_delete (self);
    }

    void set_value (int key, ucschar value) {
        hangul_keyboard_set_value (self, key, value);
    }

    void set_type (int type) {
        hangul_keyboard_set_type (self, type);
    }
};

/* define struct HangulCombination */
typedef struct {} HangulCombination;
%extend HangulCombination {
    HangulCombination () {
        return hangul_combination_new ();
    }

    ~HangulCombination () {
        hangul_combination_delete (self);
    }

    bool set_data (ucschar *first, ucschar *second, ucschar *result, int n) {
        return hangul_combination_set_data (self, first, second, result, n);
    }
};

/* define struct HangulInputContext */
typedef struct {} HangulInputContext;
%extend HangulInputContext {
    HangulInputContext (char *keyboard) {
        return hangul_ic_new (keyboard);
    }

    ~HangulInputContext() {
        hangul_ic_delete (self);
    }

    bool process (int ascii) {
        return hangul_ic_process (self, ascii);
    }

    void reset () {
        hangul_ic_reset (self);
    }

    bool backspace () {
        return hangul_ic_backspace (self);
    }

    bool is_empty () {
        return hangul_ic_is_empty (self);
    }

    bool has_choseong () {
        return hangul_ic_has_choseong (self);
    }

    bool has_jungseong () {
        return hangul_ic_has_jungseong (self);
    }

    bool has_jongseong () {
        return hangul_ic_has_jongseong (self);
    }

    int dvorak_to_qwerty (int qwerty) {
        return hangul_ic_dvorak_to_qwerty (qwerty);
    }

    void set_output_mode (int mode) {
        hangul_ic_set_output_mode (self, mode);
    }

    void set_keyboard (const HangulKeyboard *keyboard) {
        hangul_ic_set_keyboard (self, keyboard);
    }

    void select_keyboard (const char *id) {
        hangul_ic_select_keyboard (self, id);
    }

    void set_combination (const HangulCombination *combination) {
        hangul_ic_set_combination (self, combination);
    }

    void connect_callback (void *event, void *callback, void *user_data) {
        hangul_ic_connect_callback (self, event, callback, user_data);
    }

    const ucschar *get_preedit_string () {
        return hangul_ic_get_preedit_string (self);
    }

    const ucschar *get_commit_string () {
        return hangul_ic_get_commit_string (self);
    }

    const ucschar *flush () {
        return hangul_ic_flush (self);
    }
};

/*
 Translate HanjaList to (key, [(v1, c1), (v2, c2), ...])
 */
%typemap (out) HanjaList * {
    if ($1 != NULL) {
        int size = hanja_list_get_size ($1);
        PyObject *key = PyString_FromString (hanja_list_get_key ($1));
        PyObject *list = PyList_New (size);
        int i;
        for (i = 0; i < size; i++) {
            const Hanja *hanja = hanja_list_get_nth ($1, i);
            PyObject *value = PyString_FromString (hanja_get_value (hanja));
            PyObject *comment = PyString_FromString (hanja_get_comment (hanja));
            PyList_SetItem (list, i, PyTuple_Pack (2, value, comment));
        }
        hanja_list_delete ($1);
        $result = PyTuple_Pack (2, key, list);
    }
    else {
        Py_INCREF (Py_None);
        $result = Py_None;
    }
}

/* define HanjaTable */
typedef struct {} HanjaTable;
%extend HanjaTable {
    HanjaTable (const char *name) {
        HanjaTable *table = hanja_table_load (name);
        if (table == NULL) {
            PyErr_Format (PyExc_IOError,
                            "Can not load HanjaTabel from %s.", name);
            return NULL;
        }
        return table;
    }

    ~HanjaTable () {
        hanja_table_delete (self);
    }

    HanjaList *match_prefix (const char *key) {
        return hanja_table_match_prefix (self, key);
    }

    HanjaList *match_suffix (const char *key) {
        return hanja_table_match_suffix (self, key);
    }
}

bool hangul_is_choseong(ucschar c);
bool hangul_is_jungseong(ucschar c);
bool hangul_is_jongseong(ucschar c);
bool hangul_is_choseong_conjoinable(ucschar c);
bool hangul_is_jungseong_conjoinable(ucschar c);
bool hangul_is_jongseong_conjoinable(ucschar c);
bool hangul_is_syllable(ucschar c);
bool hangul_is_jaso(ucschar c);
bool hangul_is_jamo(ucschar c);

/*
ucschar hangul_jaso_to_jamo(ucschar ch);
ucschar hangul_choseong_to_jamo(ucschar ch);
ucschar hangul_jungseong_to_jamo(ucschar ch);
ucschar hangul_jongseong_to_jamo(ucschar ch);
*/

ucschar hangul_choseong_to_jongseong(ucschar ch);
ucschar hangul_jongseong_to_choseong(ucschar ch);

/*
void    hangul_jongseong_dicompose(ucschar ch, ucschar* jong, ucschar* cho);
*/
ucschar hangul_jaso_to_syllable(ucschar choseong,
                ucschar jungseong,
                ucschar jongseong);
/*
void    hangul_syllable_to_jaso(ucschar syllable,
                ucschar* choseong,
                ucschar* jungseong,
                ucschar* jongseong);
*/
enum {
    HANGUL_CHOSEONG_FILLER  = 0x115f,   /* hangul choseong filler */
    HANGUL_JUNGSEONG_FILLER = 0x1160    /* hangul jungseong filler */
};

enum {
    HANGUL_OUTPUT_SYLLABLE,
    HANGUL_OUTPUT_JAMO
};

enum {
    HANGUL_KEYBOARD_TYPE_JAMO,
    HANGUL_KEYBOARD_TYPE_JASO
};


