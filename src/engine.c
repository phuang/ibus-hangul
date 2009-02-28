/* vim:set et sts=4: */

#include <ibus.h>
#include <hangul.h>
#include <string.h>
#include "engine.h"

typedef struct _IBusHangulEngine IBusHangulEngine;
typedef struct _IBusHangulEngineClass IBusHangulEngineClass;

struct _IBusHangulEngine {
	IBusEngine parent;

    /* members */
    HangulInputContext *context;
    gboolean hangul_mode;
    HanjaList* hanja_list;

    IBusLookupTable *table;
    IBusProperty    *hangul_mode_prop;
    IBusPropList    *prop_list;
};

struct _IBusHangulEngineClass {
	IBusEngineClass parent;
};

/* functions prototype */
static void	ibus_hangul_engine_class_init   (IBusHangulEngineClass  *klass);
static void	ibus_hangul_engine_init		    (IBusHangulEngine		*hangul);
static GObject*
            ibus_hangul_engine_constructor  (GType                   type,
                                             guint                   n_construct_params,
                                             GObjectConstructParam  *construct_params);
static void	ibus_hangul_engine_destroy		(IBusHangulEngine		*hangul);
static gboolean
			ibus_hangul_engine_process_key_event
                                            (IBusEngine             *engine,
                                             guint               	 keyval,
                                             guint               	 modifiers);
static void ibus_hangul_engine_focus_in     (IBusEngine             *engine);
static void ibus_hangul_engine_focus_out    (IBusEngine             *engine);
static void ibus_hangul_engine_reset        (IBusEngine             *engine);
static void ibus_hangul_engine_enable       (IBusEngine             *engine);
static void ibus_hangul_engine_disable      (IBusEngine             *engine);
#if 0
static void ibus_engine_set_cursor_location (IBusEngine             *engine,
                                             gint                    x,
                                             gint                    y,
                                             gint                    w,
                                             gint                    h);
static void ibus_hangul_engine_set_capabilities
                                            (IBusEngine             *engine,
                                             guint                   caps);
#endif
static void ibus_hangul_engine_page_up      (IBusEngine             *engine);
static void ibus_hangul_engine_page_down    (IBusEngine             *engine);
static void ibus_hangul_engine_cursor_up    (IBusEngine             *engine);
static void ibus_hangul_engine_cursor_down  (IBusEngine             *engine);
static void ibus_hangul_engine_toggle_hangul_mode
                                            (IBusHangulEngine       *hangul);
#if 0
static void ibus_hangul_property_activate   (IBusEngine             *engine,
                                             const gchar            *prop_name,
                                             gint                    prop_state);
static void ibus_hangul_engine_property_show
											(IBusEngine             *engine,
                                             const gchar            *prop_name);
static void ibus_hangul_engine_property_hide
											(IBusEngine             *engine,
                                             const gchar            *prop_name);
#endif

static void ibus_hangul_engine_flush        (IBusHangulEngine       *hangul);
static void ibus_hangul_engine_update_preedit_text
                                            (IBusHangulEngine       *hangul);

static IBusEngineClass *parent_class = NULL;
static HanjaTable *hanja_table = NULL;

GType
ibus_hangul_engine_get_type (void)
{
	static GType type = 0;

	static const GTypeInfo type_info = {
		sizeof (IBusHangulEngineClass),
		(GBaseInitFunc)		NULL,
		(GBaseFinalizeFunc) NULL,
		(GClassInitFunc)	ibus_hangul_engine_class_init,
		NULL,
		NULL,
		sizeof (IBusHangulEngine),
		0,
		(GInstanceInitFunc)	ibus_hangul_engine_init,
	};

	if (type == 0) {
		type = g_type_register_static (IBUS_TYPE_ENGINE,
									   "IBusHangulEngine",
									   &type_info,
									   (GTypeFlags) 0);
	}

	return type;
}

void
ibus_hangul_init (void)
{
    hanja_table = hanja_table_load (NULL);
}

void
ibus_hangul_exit (void)
{
    hanja_table_delete (hanja_table);
}

static void
ibus_hangul_engine_class_init (IBusHangulEngineClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
	IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS (klass);
	IBusEngineClass *engine_class = IBUS_ENGINE_CLASS (klass);

	parent_class = (IBusEngineClass *) g_type_class_peek_parent (klass);

    object_class->constructor = ibus_hangul_engine_constructor;
	ibus_object_class->destroy = (IBusObjectDestroyFunc) ibus_hangul_engine_destroy;

    engine_class->process_key_event = ibus_hangul_engine_process_key_event;

    engine_class->reset = ibus_hangul_engine_reset;
    engine_class->enable = ibus_hangul_engine_enable;
    engine_class->disable = ibus_hangul_engine_disable;

    engine_class->focus_in = ibus_hangul_engine_focus_in;
    engine_class->focus_out = ibus_hangul_engine_focus_out;

    engine_class->page_up = ibus_hangul_engine_page_up;
    engine_class->page_down = ibus_hangul_engine_page_down;

    engine_class->cursor_up = ibus_hangul_engine_cursor_up;
    engine_class->cursor_down = ibus_hangul_engine_cursor_down;
}

static void
ibus_hangul_engine_init (IBusHangulEngine *hangul)
{
    hangul->context = hangul_ic_new ("2");
    hangul->hanja_list = NULL;
    hangul->hangul_mode = TRUE;
    hangul->hangul_mode_prop = ibus_property_new ("hangul_mode_prop",
                                           PROP_TYPE_NORMAL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           TRUE,
                                           FALSE,
                                           0,
                                           NULL);

    hangul->prop_list = ibus_prop_list_new ();
    ibus_prop_list_append (hangul->prop_list,  hangul->hangul_mode_prop);

    hangul->table = ibus_lookup_table_new (9, 0, TRUE, FALSE);
}

static GObject*
ibus_hangul_engine_constructor (GType                   type,
                                guint                   n_construct_params,
                                GObjectConstructParam  *construct_params)
{
    IBusHangulEngine *hangul;

    hangul = (IBusHangulEngine *) G_OBJECT_CLASS (parent_class)->constructor (type,
                                                       n_construct_params,
                                                       construct_params);

    return (GObject *)hangul;
}


static void
ibus_hangul_engine_destroy (IBusHangulEngine *hangul)
{
    if (hangul->prop_list) {
        g_object_unref (hangul->prop_list);
        hangul->prop_list = NULL;
    }

    if (hangul->hangul_mode_prop) {
        g_object_unref (hangul->hangul_mode_prop);
        hangul->hangul_mode_prop = NULL;
    }

    if (hangul->table) {
        g_object_unref (hangul->table);
        hangul->table = NULL;
    }

    if (hangul->context) {
        hangul_ic_delete (hangul->context);
        hangul->context = NULL;
    }

	IBUS_OBJECT_CLASS (parent_class)->destroy ((IBusObject *)hangul);
}

static void
ibus_hangul_engine_update_preedit_text (IBusHangulEngine *hangul)
{
    const gunichar *str;
    IBusText *text;

    str = hangul_ic_get_preedit_string (hangul->context);

    if (str != NULL && str[0] != 0) {
        text = ibus_text_new_from_ucs4 (str);
        ibus_text_append_attribute (text, IBUS_ATTR_TYPE_FOREGROUND, 0x00ffffff, 0, -1);
        ibus_text_append_attribute (text, IBUS_ATTR_TYPE_BACKGROUND, 0x00000000, 0, -1);
        ibus_engine_update_preedit_text ((IBusEngine *)hangul,
                                         text,
                                         ibus_text_get_length (text),
                                         TRUE);
        g_object_unref (text);
    }
    else {
        text = ibus_text_new_from_static_string ("");
        ibus_engine_update_preedit_text ((IBusEngine *)hangul, text, 0, FALSE);
        g_object_unref (text);
    }
}

static void
ibus_hangul_engine_update_auxiliary_text (IBusHangulEngine *hangul)
{
    guint cursor_pos;
    const char* comment;
    IBusText* text;

    cursor_pos = ibus_lookup_table_get_cursor_pos (hangul->table);
    comment = hanja_list_get_nth_comment (hangul->hanja_list, cursor_pos);

    text = ibus_text_new_from_string (comment);
    ibus_engine_update_auxiliary_text ((IBusEngine *)hangul, text, TRUE);
    g_object_unref (text);
}

static void
ibus_hangul_engine_update_lookup_table (IBusHangulEngine *hangul)
{
    ibus_engine_update_lookup_table ((IBusEngine *)hangul, hangul->table, TRUE);
}

static void
ibus_hangul_engine_commit_current_candidate (IBusHangulEngine *hangul)
{
    guint cursor_pos;
    const char* value;
    IBusText* text;

    hangul_ic_reset (hangul->context);
    ibus_hangul_engine_update_preedit_text (hangul);

    cursor_pos = ibus_lookup_table_get_cursor_pos (hangul->table);
    value = hanja_list_get_nth_value (hangul->hanja_list, cursor_pos);

    text = ibus_text_new_from_string (value);
    ibus_engine_commit_text ((IBusEngine *)hangul, text);
    g_object_unref (text);
}

static void
ibus_hangul_engine_open_lookup_table (IBusHangulEngine *hangul)
{
    char* utf8;
    const gunichar* str;

    str = hangul_ic_get_preedit_string (hangul->context);
    utf8 = g_ucs4_to_utf8 (str, -1, NULL, NULL, NULL);
    if (utf8 != NULL) {
        HanjaList* list = hanja_table_match_suffix (hanja_table, utf8);
        if (list != NULL) {
            int i, n;
            n = hanja_list_get_size (list);

            ibus_lookup_table_clear (hangul->table);
            for (i = 0; i < n; i++) {
                const char* value = hanja_list_get_nth_value (list, i);
                IBusText* text = ibus_text_new_from_string (value);
                ibus_lookup_table_append_candidate (hangul->table, text);
                g_object_unref (text);
            }

            hanja_list_delete (hangul->hanja_list);
            hangul->hanja_list = list;

            ibus_lookup_table_set_cursor_pos (hangul->table, 0);
            ibus_hangul_engine_update_auxiliary_text (hangul);
            ibus_hangul_engine_update_lookup_table (hangul);
        }
        g_free (utf8);
    }
}

static void
ibus_hangul_engine_close_lookup_table (IBusHangulEngine *hangul)
{
    ibus_engine_hide_lookup_table ((IBusEngine *)hangul);
    ibus_engine_hide_auxiliary_text ((IBusEngine *)hangul);
    hanja_list_delete (hangul->hanja_list);
    hangul->hanja_list = NULL;
}

static void
ibus_hangul_engine_toggle_lookup_table (IBusHangulEngine *hangul)
{
    if (hangul->hanja_list != NULL) {
        ibus_hangul_engine_close_lookup_table (hangul);
    } else {
        ibus_hangul_engine_open_lookup_table (hangul);
    }
}

static gboolean
ibus_hangul_engine_process_candidate_key_event (IBusHangulEngine    *hangul,
                                                guint                keyval,
                                                guint                modifiers)
{
    if (keyval == IBUS_Escape) {
        ibus_hangul_engine_close_lookup_table (hangul);
    } else if (keyval == IBUS_Return) {
	ibus_hangul_engine_commit_current_candidate (hangul);
        ibus_hangul_engine_close_lookup_table (hangul);
    } else if (keyval >= IBUS_1 && keyval <= IBUS_9) {
	guint page_no;
	guint page_size;
	guint cursor_pos;

	page_size = ibus_lookup_table_get_page_size (hangul->table);
	cursor_pos = ibus_lookup_table_get_cursor_pos (hangul->table);
	page_no = cursor_pos / page_size;

	cursor_pos = page_no * page_size + (keyval - IBUS_1);
	ibus_lookup_table_set_cursor_pos (hangul->table, cursor_pos);

	ibus_hangul_engine_commit_current_candidate (hangul);
        ibus_hangul_engine_close_lookup_table (hangul);
    } else if (keyval == IBUS_Left) {
        ibus_lookup_table_cursor_up (hangul->table);
        ibus_hangul_engine_update_lookup_table (hangul);
        ibus_hangul_engine_update_auxiliary_text (hangul);
    } else if (keyval == IBUS_Right) {
        ibus_lookup_table_cursor_down (hangul->table);
        ibus_hangul_engine_update_lookup_table (hangul);
        ibus_hangul_engine_update_auxiliary_text (hangul);
    } else if (keyval == IBUS_Up) {
        ibus_lookup_table_page_up (hangul->table);
        ibus_hangul_engine_update_lookup_table (hangul);
        ibus_hangul_engine_update_auxiliary_text (hangul);
    } else if (keyval == IBUS_Down) {
        ibus_lookup_table_page_down (hangul->table);
        ibus_hangul_engine_update_lookup_table (hangul);
        ibus_hangul_engine_update_auxiliary_text (hangul);
    } else if (keyval == IBUS_Page_Up) {
        ibus_lookup_table_page_up (hangul->table);
        ibus_hangul_engine_update_lookup_table (hangul);
        ibus_hangul_engine_update_auxiliary_text (hangul);
    } else if (keyval == IBUS_Page_Down) {
        ibus_lookup_table_page_down (hangul->table);
        ibus_hangul_engine_update_lookup_table (hangul);
        ibus_hangul_engine_update_auxiliary_text (hangul);
    } else if (keyval == IBUS_h) {
        ibus_lookup_table_cursor_up (hangul->table);
        ibus_hangul_engine_update_lookup_table (hangul);
        ibus_hangul_engine_update_auxiliary_text (hangul);
    } else if (keyval == IBUS_l) {
        ibus_lookup_table_cursor_down (hangul->table);
        ibus_hangul_engine_update_lookup_table (hangul);
        ibus_hangul_engine_update_auxiliary_text (hangul);
    } else if (keyval == IBUS_k) {
        ibus_lookup_table_page_up (hangul->table);
        ibus_hangul_engine_update_lookup_table (hangul);
        ibus_hangul_engine_update_auxiliary_text (hangul);
    } else if (keyval == IBUS_j) {
        ibus_lookup_table_page_down (hangul->table);
        ibus_hangul_engine_update_lookup_table (hangul);
        ibus_hangul_engine_update_auxiliary_text (hangul);
    }

    return TRUE;
}

static gboolean
ibus_hangul_engine_process_key_event (IBusEngine     *engine,
                                      guint           keyval,
                                      guint           modifiers)
{
    IBusHangulEngine *hangul = (IBusHangulEngine *) engine;

    gboolean retval;
    const gunichar *str;

    if (modifiers & IBUS_RELEASE_MASK)
        return FALSE;

    // if we don't ignore shift keys, shift key will make flush the preedit 
    // string. So you cannot input shift+key.
    // Let's think about these examples:
    //   dlTek (2 set)
    //   qhRdmaqkq (2 set)
    if (keyval == IBUS_Shift_L || keyval == IBUS_Shift_R)
        return FALSE;

    if (keyval == IBUS_F9 || keyval == IBUS_Hangul_Hanja) {
	ibus_hangul_engine_toggle_lookup_table (hangul);
	return TRUE;
    }

    if (modifiers & (IBUS_CONTROL_MASK | IBUS_MOD1_MASK))
        return FALSE;

    if (hangul->hanja_list != NULL) {
        return ibus_hangul_engine_process_candidate_key_event (hangul,
                     keyval, modifiers);
    }

    if (keyval == IBUS_BackSpace) {
        retval = hangul_ic_backspace (hangul->context);
    } else {
        retval = hangul_ic_process (hangul->context, keyval);
    }

    str = hangul_ic_get_commit_string (hangul->context);
    if (str != NULL && str[0] != 0) {
        IBusText *text = ibus_text_new_from_ucs4 (str);
        ibus_engine_commit_text ((IBusEngine *)hangul, text);
        g_object_unref (text);
    }

    ibus_hangul_engine_update_preedit_text (hangul);

    if (!retval)
        ibus_hangul_engine_flush (hangul);

    return retval;
}

static void
ibus_hangul_engine_flush (IBusHangulEngine *hangul)
{
    const gunichar *str;
    IBusText *text;

    str = hangul_ic_flush (hangul->context);

    if (str == NULL || str[0] == 0)
        return;

    text = ibus_text_new_from_ucs4 (str);

    ibus_engine_hide_preedit_text ((IBusEngine *) hangul);
    ibus_engine_commit_text ((IBusEngine *) hangul, text);

    g_object_unref (text);
}

static void
ibus_hangul_engine_toggle_hangul_mode (IBusHangulEngine *hangul)
{
    IBusText *text;
    hangul->hangul_mode = ! hangul->hangul_mode;

    ibus_hangul_engine_flush (hangul);

    if (hangul->hangul_mode) {
        text = ibus_text_new_from_static_string ("한");
    }
    else {
        text = ibus_text_new_from_static_string ("A");
    }

    ibus_property_set_label (hangul->hangul_mode_prop, text);
    ibus_engine_update_property ((IBusEngine *)hangul, hangul->hangul_mode_prop);
    g_object_unref (text);
}

static void
ibus_hangul_engine_focus_in (IBusEngine *engine)
{
    IBusHangulEngine *hangul = (IBusHangulEngine *) engine;

    ibus_engine_register_properties (engine, hangul->prop_list);

    if (hangul->hanja_list != NULL) {
        ibus_hangul_engine_update_lookup_table (hangul);
        ibus_hangul_engine_update_auxiliary_text (hangul);
    }

    parent_class->focus_in (engine);
}

static void
ibus_hangul_engine_focus_out (IBusEngine *engine)
{
    IBusHangulEngine *hangul = (IBusHangulEngine *) engine;

    if (hangul->hanja_list == NULL) {
	ibus_hangul_engine_flush (hangul);
    } else {
	ibus_engine_hide_lookup_table (engine);
	ibus_engine_hide_auxiliary_text (engine);
    }

    parent_class->focus_out ((IBusEngine *) hangul);
}

static void
ibus_hangul_engine_reset (IBusEngine *engine)
{
    IBusHangulEngine *hangul = (IBusHangulEngine *) engine;

    ibus_hangul_engine_flush (hangul);
    if (hangul->hanja_list != NULL) {
        ibus_hangul_engine_close_lookup_table (hangul);
    }
    parent_class->reset (engine);
}

static void
ibus_hangul_engine_enable (IBusEngine *engine)
{
    parent_class->enable (engine);
}

static void
ibus_hangul_engine_disable (IBusEngine *engine)
{
    ibus_hangul_engine_focus_out (engine);
    parent_class->disable (engine);
}

static void
ibus_hangul_engine_page_up (IBusEngine *engine)
{
    parent_class->page_up (engine);
}

static void
ibus_hangul_engine_page_down (IBusEngine *engine)
{
    parent_class->page_down (engine);
}

static void
ibus_hangul_engine_cursor_up (IBusEngine *engine)
{
    IBusHangulEngine *hangul = (IBusHangulEngine *) engine;

    if (hangul->hanja_list != NULL) {
	ibus_lookup_table_cursor_up (hangul->table);
	ibus_hangul_engine_update_lookup_table (hangul);
	ibus_hangul_engine_update_auxiliary_text (hangul);
    }

    parent_class->cursor_up (engine);
}

static void
ibus_hangul_engine_cursor_down (IBusEngine *engine)
{
    IBusHangulEngine *hangul = (IBusHangulEngine *) engine;

    if (hangul->hanja_list != NULL) {
	ibus_lookup_table_cursor_down (hangul->table);
	ibus_hangul_engine_update_lookup_table (hangul);
	ibus_hangul_engine_update_auxiliary_text (hangul);
    }

    parent_class->cursor_down (engine);
}

