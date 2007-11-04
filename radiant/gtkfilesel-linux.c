/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */
#define LEO

#ifdef LEO
#define _(a) a

static char * back_xpm[] = {
"14 14 33 1",
" 	c None",
".	c #000000",
"+	c #C6D7C6",
"@	c #E7EBE7",
"#	c #FFFFFF",
"$	c #DEEBDE",
"%	c #F7F7F7",
"&	c #DEE7DE",
"*	c #EFF3EF",
"=	c #101810",
"-	c #B5C7AD",
";	c #EFEFEF",
">	c #D6E3D6",
",	c #213021",
"'	c #315931",
")	c #52824A",
"!	c #739A6B",
"~	c #84A673",
"{	c #7BA673",
"]	c #84AA73",
"^	c #84AA7B",
"/	c #84AE7B",
"(	c #63925A",
"_	c #526D4A",
":	c #4A7D42",
"<	c #739E6B",
"[	c #739A63",
"}	c #4A7539",
"|	c #638E52",
"1	c #427139",
"2	c #6BA663",
"3	c #5A8A52",
"4	c #315929",
"            ..",
"          ..+.",
"        ..@#+.",
"      ..$#%%+.",
"    ..&#%*%%+.",
"  .=&#******+.",
"..-#;>&@****+,",
"..')!~{]^/^/(.",
"  .._:<^~^/^(.",
"    ..':[]~/(.",
"      ..}:[~|.",
"        ..123.",
"          ..4.",
"            .."};

static char * up_xpm[] = {
"14 14 36 1",
" 	c None",
".	c #000000",
"+	c #181C18",
"@	c #D6DBD6",
"#	c #94AA8C",
"$	c #000400",
"%	c #DEDFDE",
"&	c #94AA84",
"*	c #E7E3E7",
"=	c #94B28C",
"-	c #6B865A",
";	c #EFEBEF",
">	c #9CB694",
",	c #8CA684",
"'	c #EFEFEF",
")	c #F7EFF7",
"!	c #9CB68C",
"~	c #63865A",
"{	c #94B684",
"]	c #94AE84",
"^	c #739263",
"/	c #F7F3F7",
"(	c #94B284",
"_	c #849E73",
":	c #8CAE7B",
"<	c #8CAA84",
"[	c #7B966B",
"}	c #8CA67B",
"|	c #DEDBD6",
"1	c #E7E7E7",
"2	c #8CAE84",
"3	c #8CAA7B",
"4	c #738E63",
"5	c #BDBEB5",
"6	c #BDC3BD",
"7	c #637D52",
"      ..      ",
"      ..      ",
"     +@#$     ",
"     .%&.     ",
"    .**=-.    ",
"    .;;>,.    ",
"   .*')!&~.   ",
"   .;)){]^.   ",
"  .*')/(]_-.  ",
"  .;)//::<[.  ",
" .*')//:::}-. ",
" .|1;;12]3}4. ",
".556666^^^^-7.",
".............."};

static char * forward_xpm[] = {
"14 14 36 1",
" 	c None",
".	c #000000",
"+	c #E7EBDE",
"@	c #FFFFFF",
"#	c #F7F7EF",
"$	c #D6E3D6",
"%	c #F7F7F7",
"&	c #EFF3EF",
"*	c #CEDFCE",
"=	c #CEDBC6",
"-	c #E7EFE7",
";	c #181818",
">	c #292829",
",	c #E7EBE7",
"'	c #DEE7DE",
")	c #B5C7AD",
"!	c #9CBA94",
"~	c #8CAE84",
"{	c #84AA7B",
"]	c #7BA673",
"^	c #84A67B",
"/	c #739A6B",
"(	c #5A824A",
"_	c #395931",
":	c #9CBA8C",
"<	c #84AE7B",
"[	c #739E6B",
"}	c #527D4A",
"|	c #425942",
"1	c #84A673",
"2	c #4A7142",
"3	c #94B284",
"4	c #395D31",
"5	c #5A8652",
"6	c #315929",
"7	c #396531",
"..            ",
".+..          ",
".@#$..        ",
".@%&#*..      ",
".@%%&&%=..    ",
".@&&&&&-#=;.  ",
">@&&&&,'$'&)..",
".!~{~{{]^/(_..",
".:{<{^{[}|..  ",
".:<1{/}2..    ",
".31/}4..      ",
".{56..        ",
".7..          ",
"..            "};

static char * refresh_xpm[] = {
"16 16 11 1",
" 	c None",
".	c #000000",
"+	c #526942",
"@	c #4A6139",
"#	c #526542",
"$	c #5A7142",
"%	c #425531",
"&	c #314529",
"*	c #425131",
"=	c #425931",
"-	c #5A754A",
"       .        ",
"      ..        ",
"     .+@...     ",
"    .#$##@%..   ",
"     .+#...%%.  ",
"   .  ..   .&.  ",
"  .    .    .&. ",
" ..          .. ",
" ..          .. ",
" .*.    .    .  ",
"  .*.   ..  .   ",
"  .%@...#=.     ",
"   ..##-#@#.    ",
"     ...@%.     ",
"        ..      ",
"        .       "};

#endif

#ifndef LEO
#include "config.h"
#endif

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

#include "fnmatch.h"

#if (defined TORRIE_DEBUG || defined LEO)
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkentry.h>
#include "gtkfilesel-linux.h"
#include <gtk/gtkhbox.h>
#include <gtk/gtkhbbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtklist.h>
#include <gtk/gtklistitem.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkclist.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkcombo.h>
#include <gtk/gtkframe.h>
#include <gtk/gtkhpaned.h>
#include <gtk/gtktable.h>
#include <gtk/gtkpixmap.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkhseparator.h>
#include <gtk/gtktogglebutton.h>
#else
#include "gdk/gdkkeysyms.h"
#include "gtkbutton.h"
#include "gtkentry.h"
#include "gtkfilesel.h"
#include "gtkhbox.h"
#include "gtkhbbox.h"
#include "gtklabel.h"
#include "gtklist.h"
#include "gtklistitem.h"
#include "gtkmain.h"
#include "gtkscrolledwindow.h"
#include "gtksignal.h"
#include "gtkvbox.h"
#include "gtkmenu.h"
#include "gtkmenuitem.h"
#include "gtkoptionmenu.h"
#include "gtkclist.h"
#include "gtkdialog.h"
#include "gtkcombo.h"
#include "gtkframe.h"
#include "gtkhpaned.h"
#include "gtktable.h"
#include "gtkpixmap.h"
#include "gtknotebook.h"
#include "gtkhseparator.h"
#include "gtktogglebutton.h"
#endif

#ifndef LEO
#include "gtkintl.h"

#include "back.xpm"
#include "up.xpm"
#include "forward.xpm"
#include "refresh.xpm"
#endif

#define DIR_LIST_WIDTH   180
#define DIR_LIST_HEIGHT  180
#define FILE_LIST_WIDTH  180
#define FILE_LIST_HEIGHT 180
#define BOOKMARK_FILE "/.gtkfilesel_bookmarks"
#define MASK_FILE "/.gtkfilesel_masks"
#define TIME_STRING_BUF 50

/* I've put this here so it doesn't get confused with the 
 * file completion interface */
typedef struct _HistoryCallbackArg HistoryCallbackArg;

struct _HistoryCallbackArg
{
  gchar *directory;
  GtkWidget *menu_item;
};


typedef struct _BookmarkMenuStruct BookmarkMenuStruct;
struct _BookmarkMenuStruct {
  GtkWidget *menu_item;
  gchar     *desc;
  gchar     *path;
};

typedef struct _CompletionState    CompletionState;
typedef struct _CompletionDir      CompletionDir;
typedef struct _CompletionDirSent  CompletionDirSent;
typedef struct _CompletionDirEntry CompletionDirEntry;
typedef struct _CompletionUserDir  CompletionUserDir;
typedef struct _PossibleCompletion PossibleCompletion;

/* Non-external file completion decls and structures */

/* A contant telling PRCS how many directories to cache.  Its actually
 * kept in a list, so the geometry isn't important. */
#define CMPL_DIRECTORY_CACHE_SIZE 10

/* A constant used to determine whether a substring was an exact
 * match by first_diff_index()
 */
#define PATTERN_MATCH -1
/* The arguments used by all fnmatch() calls below
 */
#define FNMATCH_FLAGS (FNM_PATHNAME | FNM_PERIOD)

#define CMPL_ERRNO_TOO_LONG ((1<<16)-1)

/* This structure contains all the useful information about a directory
 * for the purposes of filename completion.  These structures are cached
 * in the CompletionState struct.  CompletionDir's are reference counted.
 */
struct _CompletionDirSent
{
  ino_t inode;
  time_t mtime;
  dev_t device;

  gint entry_count;
  gchar *name_buffer; /* memory segment containing names of all entries */

  struct _CompletionDirEntry *entries;
};

struct _CompletionDir
{
  CompletionDirSent *sent;

  gchar *fullname;
  gint fullname_len;

  struct _CompletionDir *cmpl_parent;
  gint cmpl_index;
  gchar *cmpl_text;
};

/* This structure contains pairs of directory entry names with a flag saying
 * whether or not they are a valid directory.  NOTE: This information is used
 * to provide the caller with information about whether to update its completions
 * or try to open a file.  Since directories are cached by the directory mtime,
 * a symlink which points to an invalid file (which will not be a directory),
 * will not be reevaluated if that file is created, unless the containing
 * directory is touched.  I consider this case to be worth ignoring (josh).
 */
struct _CompletionDirEntry
{
  gint is_dir;
  gchar *entry_name;
};

struct _CompletionUserDir
{
  gchar *login;
  gchar *homedir;
};

struct _PossibleCompletion
{
  /* accessible fields, all are accessed externally by functions
   * declared above
   */
  gchar *text;
  gint is_a_completion;
  gint is_directory;

  gint file_size;
  gint file_time;
  gint uid;
  gint gid;
  /* Private fields
   */
  gint text_alloc;
};

struct _CompletionState
{
  gint last_valid_char;
  gchar *updated_text;
  gint updated_text_len;
  gint updated_text_alloc;
  gint re_complete;

  gchar *user_dir_name_buffer;
  gint user_directories_len;

  gchar *last_completion_text;

  gint user_completion_index; /* if >= 0, currently completing ~user */

  struct _CompletionDir *completion_dir; /* directory completing from */
  struct _CompletionDir *active_completion_dir;

  struct _PossibleCompletion the_completion;

  struct _CompletionDir *reference_dir; /* initial directory */

  GList* directory_storage;
  GList* directory_sent_storage;

  struct _CompletionUserDir *user_directories;
};

/* Widgets from the Properties Dialog */
typedef struct _PropertiesPrivate PropertiesPrivate;

struct _PropertiesPrivate
{
  GtkWidget *mode_label;
  GtkWidget *mode_buttons[12];
};

/* pixmap creation function */
GtkWidget*                 create_pixmap          (GtkWidget *widget, 
						   const gchar *pixmap_char);

/* File completion functions which would be external, were they used
 * outside of this file.
 */

static CompletionState*    cmpl_init_state        (void);
static void                cmpl_free_state        (CompletionState *cmpl_state);
static gint                cmpl_state_okay        (CompletionState* cmpl_state);
static gchar*              cmpl_strerror          (gint);

static PossibleCompletion* cmpl_completion_matches(gchar           *text_to_complete,
						   gchar          **remaining_text,
						   CompletionState *cmpl_state);

/* Returns a name for consideration, possibly a completion, this name
 * will be invalid after the next call to cmpl_next_completion.
 */
static char*               cmpl_this_completion   (PossibleCompletion*);

/* True if this completion matches the given text.  Otherwise, this
 * output can be used to have a list of non-completions.
 */
static gint                cmpl_is_a_completion   (PossibleCompletion*);

/* True if the completion is a directory
 */
static gint                cmpl_is_directory      (PossibleCompletion*);

/* Obtains the next completion, or NULL
 */
static PossibleCompletion* cmpl_next_completion   (CompletionState*);

/* Updating completions: the return value of cmpl_updated_text() will
 * be text_to_complete completed as much as possible after the most
 * recent call to cmpl_completion_matches.  For the present
 * application, this is the suggested replacement for the user's input
 * string.  You must CALL THIS AFTER ALL cmpl_text_completions have
 * been received.
 */
static gchar*              cmpl_updated_text       (CompletionState* cmpl_state);

/* After updating, to see if the completion was a directory, call
 * this.  If it was, you should consider re-calling completion_matches.
 */
static gint                cmpl_updated_dir        (CompletionState* cmpl_state);

/* Current location: if using file completion, return the current
 * directory, from which file completion begins.  More specifically,
 * the cwd concatenated with all exact completions up to the last
 * directory delimiter('/').
 */
static gchar*              cmpl_reference_position (CompletionState* cmpl_state);

/* backing up: if cmpl_completion_matches returns NULL, you may query
 * the index of the last completable character into cmpl_updated_text.
 */
static gint                cmpl_last_valid_char    (CompletionState* cmpl_state);

/* When the user selects a non-directory, call cmpl_completion_fullname
 * to get the full name of the selected file.
 */
static gchar*              cmpl_completion_fullname (gchar*, CompletionState* cmpl_state);


/* Directory operations. */
static CompletionDir* open_ref_dir         (gchar* text_to_complete,
					    gchar** remaining_text,
					    CompletionState* cmpl_state);
static gboolean       check_dir            (gchar *dir_name, 
					    struct stat *result, 
					    gboolean *stat_subdirs);
static CompletionDir* open_dir             (gchar* dir_name,
					    CompletionState* cmpl_state);
static CompletionDir* open_user_dir        (gchar* text_to_complete,
					    CompletionState *cmpl_state);
static CompletionDir* open_relative_dir    (gchar* dir_name, CompletionDir* dir,
					    CompletionState *cmpl_state);
static CompletionDirSent* open_new_dir     (gchar* dir_name, 
					    struct stat* sbuf,
					    gboolean stat_subdirs);
static gint           correct_dir_fullname (CompletionDir* cmpl_dir);
static gint           correct_parent       (CompletionDir* cmpl_dir,
					    struct stat *sbuf);
static gchar*         find_parent_dir_fullname    (gchar* dirname);
static CompletionDir* attach_dir           (CompletionDirSent* sent,
					    gchar* dir_name,
					    CompletionState *cmpl_state);
static void           free_dir_sent (CompletionDirSent* sent);
static void           free_dir      (CompletionDir  *dir);
static void           prune_memory_usage(CompletionState *cmpl_state);

/* Completion operations */
static PossibleCompletion* attempt_homedir_completion(gchar* text_to_complete,
						      CompletionState *cmpl_state);
static PossibleCompletion* attempt_file_completion(CompletionState *cmpl_state);
static CompletionDir* find_completion_dir(gchar* text_to_complete,
					  gchar** remaining_text,
					  CompletionState* cmpl_state);
static PossibleCompletion* append_completion_text(gchar* text,
						  CompletionState* cmpl_state);
static gint get_pwdb(CompletionState* cmpl_state);
static gint first_diff_index(gchar* pat, gchar* text);
static gint compare_user_dir(const void* a, const void* b);
static gint compare_cmpl_dir(const void* a, const void* b);
static void update_cmpl(PossibleCompletion* poss,
			CompletionState* cmpl_state);

static void gtk_file_selection_class_init    (GtkFileSelectionClass *klass);
static void gtk_file_selection_init          (GtkFileSelection      *filesel);
static void gtk_file_selection_realize       (GtkWidget             *widget);
static void gtk_file_selection_destroy       (GtkObject             *object);
static gint gtk_file_selection_key_press     (GtkWidget             *widget,
					      GdkEventKey           *event,
					      gpointer               user_data);

static void gtk_file_selection_file_button (GtkWidget *widget,
					    gint row, 
					    gint column, 
					    GdkEventButton *bevent,
					    gpointer user_data);

static void gtk_file_selection_dir_button (GtkWidget *widget,
					   gint row,
					   gint column,
					   GdkEventButton *bevent,
					   gpointer data);

static void gtk_file_selection_undir_button (GtkWidget *widget,
					     gint row,
					     gint column,
					     GdkEventButton *bevent,
					     gpointer data);

static void gtk_file_selection_populate      (GtkFileSelection      *fs,
					      gchar                 *rel_path,
					      gint                   try_complete);
static void gtk_file_selection_abort         (GtkFileSelection      *fs);

static void gtk_file_selection_update_history_menu (GtkFileSelection       *fs,
						    gchar                  *current_dir);

static void gtk_file_selection_create_dir (gpointer data);
static void gtk_file_selection_delete_file (gpointer data);
static void gtk_file_selection_rename_file (gpointer data);
static void gtk_file_selection_properties (gpointer data);
static void gtk_file_selection_properties_update_mode (GtkWidget *widget, gpointer data);
static mode_t gtk_file_selection_properties_get_mode (PropertiesPrivate* private);

static gboolean gtk_file_selection_history_combo_callback (GtkWidget *widget, GdkEventKey *event, gpointer data);
static gboolean gtk_file_selection_history_combo_list_key_handler(GtkWidget *widget,
																							GdkEventKey *event,
																							gpointer user_data);
static gboolean gtk_file_selection_history_combo_list_callback (GtkWidget *thelist,
																							GdkEventButton *event,

																							gpointer user_data);
static void gtk_file_selection_bookmark_callback (GtkWidget *widget, gpointer data);
static void gtk_file_selection_mask_entry_callback (GtkWidget *widget, gpointer data);
static gint gtk_file_selection_mask_entry_key_callback (GtkWidget *widget, GdkEventKey *event, gpointer data);
static gint gtk_file_selection_mask_entry_button_callback (GtkWidget *widget, GdkEventButton *event, gpointer data);

//static void gtk_file_selection_home_button (GtkWidget *widget, gpointer data);
static void gtk_file_selection_bookmark_button (GtkWidget *widget, 
	      	                   GtkFileSelection *fs);

static void gtk_file_selection_up_button (GtkWidget *widget, gpointer data);
static void gtk_file_selection_prev_button (GtkWidget *widget, gpointer data);
static void gtk_file_selection_next_button (GtkWidget *widget, gpointer data);
static void gtk_file_selection_refresh_button (GtkWidget *widget, gpointer data);

static gint gtk_file_selection_files_list_key_callback (GtkWidget *widget, GdkEventKey *event, gpointer data);


static gint gtk_file_selection_match_char (gchar, gchar *mask);
static gint gtk_file_selection_match_mask (gchar *,gchar *);

static void gtk_file_selection_load_bookmarks(GtkFileSelection *fs);
static void gtk_file_selection_add_bookmark (GtkFileSelection *fs, gchar *desc, gchar *path);
gint gtk_file_selection_save_bookmarks (GtkFileSelection *fs);

static void gtk_file_selection_load_masks(GtkFileSelection *fs);

static gint gtk_file_selection_show_fileop_menu (GtkCList *clist, 
						 GdkEvent *event, 
						 GtkFileSelection *fs);

						 
static GtkWindowClass *parent_class = NULL;

/* Saves errno when something cmpl does fails. */
static gint cmpl_errno;

#ifdef G_WITH_CYGWIN
/*
 * Take the path currently in the file selection
 * entry field and translate as necessary from
 * a WIN32 style to CYGWIN32 style path.  For
 * instance translate:
 * x:\somepath\file.jpg
 * to:
 * //x/somepath/file.jpg
 *
 * Replace the path in the selection text field.
 * Return a boolean value concerning whether a
 * translation had to be made.
 */
int
translate_win32_path (GtkFileSelection *filesel)
{
  int updated = 0;
  gchar *path;

  /*
   * Retrieve the current path
   */
  path = gtk_entry_get_text (GTK_ENTRY (filesel->selection_entry));

  /*
   * Translate only if this looks like a DOS-ish
   * path... First handle any drive letters.
   */
  if (isalpha (path[0]) && (path[1] == ':')) {
    /*
     * This part kind of stinks... It isn't possible
     * to know if there is enough space in the current
     * string for the extra character required in this
     * conversion.  Assume that there isn't enough space
     * and use the set function on the text field to
     * set the newly created string.
     */
    gchar *newPath = g_strdup_printf ("//%c/%s", path[0], (path + 3));
    gtk_entry_set_text (GTK_ENTRY (filesel->selection_entry), newPath);

    path = newPath;
    updated = 1;
  }

  /*
   * Now, replace backslashes with forward slashes 
   * if necessary.
   */
  if (strchr (path, '\\'))
    {
      int index;
      for (index = 0; path[index] != '\0'; index++)
	if (path[index] == '\\')
	  path[index] = '/';
      
      updated = 1;
    }
    
  return updated;
}
#endif

/* General notes:
 * Make prev and next inactive if their respective *
 *   histories are empty.
 * Add facilities for handling hidden files and    *
 * directories                                     *
 * Add an api to access the mask, and hidden files *
 * check box?  (prob not in 1.2.x series)          *
 */

/* Routine for applying mask to filenames         *
 *   Need to be optimized to minimize recursion   *
 *     help the for loop by looking for the next  *
 *     instance of the mask character following   *
 *     the '*'.  ei *.c -- look for '.'           *
 *     Also, swap all *? pairs (-> ?*), as that   *
 *     will make it possible to look ahead (?     *
 *     makes it very nondeterministic as in *?.c  *
 *     which really is ?*.c                       *
 *                                                *
 */
static gint gtk_file_selection_match_char (gchar text, gchar *mask)
{
  gchar *maskc;
  gint x;
  gint s;
  gchar lastc;
  gchar nextc;

  if (mask[0] == '[')
    {
      if (!strchr (mask,']')) return 0;
      lastc = 0;

      maskc = g_strdup(mask + 1); /* get the portion of mask inside []*/
      (*(strchr (maskc + 1,']'))) = 0;
      s = strlen ((char *)maskc);

      for (x = 0 ; x < s ; x ++){
        if (maskc[x] == '-')
          {
            if (x == s) return 1;
            nextc = maskc[x + 1];

            if (nextc > lastc)
            {
              if ((lastc <= text) && (nextc >= text))
              {
                g_free (maskc);
                return s + 2;
              }
            }
            else if ((lastc >= text) && (nextc <= text))
              {
                g_free (maskc);
                return s + 2;
              }
          }
        else if (text == maskc[x])
          {
            g_free (maskc);
            return s + 2;
          }
        lastc = maskc[x];
      }
      g_free (maskc);

      return 0;
    }

  if (mask[0] == '?') return 1;
  if (mask[0] == text) return 1;

  return 0;
}


static gint gtk_file_selection_match_mask1 (gchar *text, gchar *mask)
{

  int mc;
  int tc;

  tc = 0; mc = 0;
	
  if (mask[0] == 0 && text[0] == 0) return 1;
	
  if (mask[0] == '*')
    {
      for (tc = 0; tc <= strlen(text); tc++)
	{
	  if (gtk_file_selection_match_mask1 (text + tc, mask + 1))
	    return 1;
	}
      return 0;
    }
  mc = gtk_file_selection_match_char (text[0], mask);

  if(mc)
    return gtk_file_selection_match_mask1 (text + 1, mask + mc);
  else
    return 0;
}

static gint gtk_file_selection_match_mask (gchar *text, gchar *mask)
{
  gchar *masks;
  gchar *bmask;
  gchar *emask;

  masks=g_strdup(mask);

  emask=strchr(masks,'<');
  if(emask){
    bmask=emask+1;
    emask=strchr(bmask,'>');
    if(emask){
      *emask=0;
    }
  }else{
    bmask=masks;
  }

  do{
    if((emask=strchr(bmask,',')) || (emask=strchr(bmask,';'))){
      *emask=0;
      if (gtk_file_selection_match_mask1 (text, bmask)){
      	g_free(masks);
	return 1;
      }

      bmask=emask+1;
    }
  }while(emask);

  if(gtk_file_selection_match_mask1 (text, bmask)){
    g_free(masks);
    return 1;
  }
  g_free(masks);
  return 0;
}

static void
gtk_file_selection_load_bookmarks(GtkFileSelection *fs)
{
  GList *list;
  gchar *bookmark_file;
  gchar *bookmark_data;
  struct stat file_info;
  gint   file;
  gint   lp;
  gint   cp;
  BookmarkMenuStruct *item;


  if(fs->bookmark_list){  //erase
    list=fs->bookmark_list;
    while(list){
      item=list->data;
      g_free(item->desc);
      g_free(item->path);
      g_free(item);
      list=list->next;
    }
    g_list_free (fs->bookmark_list);
    fs->bookmark_list = NULL;
    gtk_widget_destroy (fs->bookmark_menu);
  }

  fs->bookmark_menu=gtk_menu_new();

  /* spacer */
  item=g_malloc(sizeof(item));
  item->menu_item = gtk_menu_item_new();
  gtk_widget_show(item->menu_item);
  gtk_menu_append (GTK_MENU(fs->bookmark_menu), item->menu_item);

  item=g_malloc(sizeof(item));
  item->desc=g_strdup("Add bookmark");
  item->path=g_strdup(".");
  item->menu_item=gtk_menu_item_new_with_label (item->desc);
  gtk_widget_show(item->menu_item);
  //fs->bookmark_list=g_list_append(fs->bookmark_list,item);
  //set signal here!!
  gtk_menu_append (GTK_MENU(fs->bookmark_menu), item->menu_item);

  item=g_malloc(sizeof(item));
  item->desc=g_strdup("Edit bookmark");
  item->path=g_strdup(".");
  item->menu_item=gtk_menu_item_new_with_label (item->desc);
  gtk_widget_show(item->menu_item);
  //fs->bookmark_list=g_list_append(fs->bookmark_list,item);
  //set signal here!!
  gtk_menu_append (GTK_MENU(fs->bookmark_menu), item->menu_item);

  bookmark_file=g_strconcat(g_get_home_dir(), BOOKMARK_FILE ,NULL);
  if(!stat(bookmark_file,&file_info) && (file = open(bookmark_file,  O_RDONLY )) > 0)
  {
    if(file_info.st_size <65536 )
    {
      bookmark_data=g_malloc(file_info.st_size);

      if(file && read(file, bookmark_data, file_info.st_size))
      {
        cp=lp=0;

        while (cp < file_info.st_size)
        {
          while (cp < file_info.st_size && bookmark_data[cp] != '<' )
            cp++;
	  bookmark_data[cp]=0;
	  item=g_malloc(sizeof(BookmarkMenuStruct));
	  item->desc=g_strdup(bookmark_data+lp);
	  lp=++cp;
	  
          while (cp < file_info.st_size && bookmark_data[cp] != '>' )
            cp++;

          bookmark_data[cp]=0;
          //create menu items
	  item->path=g_strdup(bookmark_data+lp);
	  gtk_file_selection_add_bookmark ((gpointer) fs, (gpointer) item->desc, (gpointer) item->path);

	  cp++;

          while(cp < file_info.st_size && bookmark_data[cp] < 33 )
            cp++;
          lp=cp;
        }
      }

      close(file);
    }
  } else {

    /* Add some default items, then save off to bookmarks file */

    gtk_file_selection_add_bookmark ((gpointer) fs, "Home", "~/");
    gtk_file_selection_add_bookmark ((gpointer) fs, "Root", "/");

    gtk_file_selection_save_bookmarks ((gpointer) fs);
  }
}

static void 
gtk_file_selection_add_bookmark (GtkFileSelection *fs, gchar *desc, gchar *path) 
{
  /* Add item to menu */
  BookmarkMenuStruct *item;
  item=g_malloc(sizeof(item));
  item->desc = (gpointer) desc;
  item->path = (gpointer) path;
  item->menu_item=gtk_menu_item_new_with_label (item->desc);
  gtk_widget_show(item->menu_item);
  fs->bookmark_list=g_list_append(fs->bookmark_list,item);
  gtk_signal_connect (GTK_OBJECT(item->menu_item), "activate",
		      (GtkSignalFunc) gtk_file_selection_bookmark_callback,
		      (gpointer) fs);
  gtk_menu_insert (GTK_MENU(fs->bookmark_menu), item->menu_item, g_list_length(fs->bookmark_list) -1);
}

gint 
gtk_file_selection_save_bookmarks (GtkFileSelection *fs)
{
  BookmarkMenuStruct *item;
  gchar *bookmark_file;
  gchar *item_data;
  gint   file;
  GList *list;

  bookmark_file=g_strconcat(g_get_home_dir(), BOOKMARK_FILE ,NULL);

  if ((file = open(bookmark_file, O_CREAT | O_WRONLY | O_TRUNC, 0600)) > 0)
  {
    for (list = g_list_first (fs->bookmark_list); list != NULL; list = g_list_next(list)) {
      item = list->data;
      item_data = g_strconcat(item->desc, " <", item->path, ">\n", NULL);
      if (write (file, item_data, strlen(item_data)) != strlen(item_data)) {
	return TRUE;
      }
      g_free(item_data);
    }
    
    close(file);
  } else {
    return TRUE;
  }

  return FALSE;
}

static void
gtk_file_selection_load_masks(GtkFileSelection *fs)
{
  /*
  GList *list;
  gchar *masks_file;
  gchar *masks_data;
  struct stat file_info;
  gint   file;
  gint   lp;
  gint   cp;

  if(fs->masks){
    list=fs->masks;
    while(list){
      g_free(list->data);
      list=list->next;
    }
    fs->masks = NULL;
  }

  masks_file=g_strconcat(g_get_home_dir(), MASK_FILE,NULL); //put in #define
  if(!stat(masks_file,&file_info))
  {
    if(file_info.st_size <65536 )
    {
      masks_data=g_malloc(file_info.st_size);

      file = open(masks_file,  O_RDONLY );

      if(file && read(file, masks_data, file_info.st_size))
      {
        cp=lp=0;

        while (cp < file_info.st_size)
        {
          while (cp < file_info.st_size && masks_data[cp] != '>' )
            cp++;

          masks_data[++cp]=0;
          if (masks_data[lp]=='<') { //if there was no description, strip off brackets
            lp++;
            masks_data[cp-1]=0;
          }
//          g_print("%s\n",masks_data+lp);
          fs->masks = g_list_append(fs->masks, g_strdup(masks_data+lp));

          while(cp < file_info.st_size && masks_data[cp] < 33 )
            cp++;
          lp=cp;
        }
      }

      close(file);
    }
  }
  */
  if (!fs->masks) {
    /* masks is still null, fill it with default data... */
    /*
    fs->masks = g_list_append(fs->masks, "all files <*>");
    fs->masks = g_list_append(fs->masks, "mp3s/playlists <*.mp3,*.m3u>");
    fs->masks = g_list_append(fs->masks, "src/hdr <*.[CcHh],*.[Cc][Cc],*.[Hh][Hh],*.cpp>");
    fs->masks = g_list_append(fs->masks, "html docs <*.html,*.htm,*.HTM,*.php*,*.inc>");
    fs->masks = g_list_append(fs->masks, "images <*.png,*.jpg,*.jpeg,*.gif,*.xpm,*.tiff>");
    fs->masks = g_list_append(fs->masks, "package <*.rpm,*.deb>");
    fs->masks = g_list_append(fs->masks, "archive <*.tgz,*.tb2,*.tar*,*.zip,*.rar>");
    fs->masks = g_list_append(fs->masks, "compressed <*.Z,*.gz,*.bz2>");
    */
  }
}

void gtk_file_selection_clear_masks (GtkFileSelection *filesel)
{
  GList *list;

  g_return_if_fail (filesel != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (filesel));

  list = filesel->masks;
  while (list)
    {
      g_free (list->data);
      list = list->next;
    }
  filesel->masks = NULL;

  gtk_list_clear_items (GTK_LIST (GTK_COMBO (filesel->mask_entry)->list), 0, -1);
}

void gtk_file_selection_set_masks (GtkFileSelection *filesel, const gchar **masks)
{
  g_return_if_fail (filesel != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (filesel));

  while (*masks)
    {
      filesel->masks = g_list_append (filesel->masks, (gpointer)*masks);
      masks++;
    }

  if (filesel->masks)
    gtk_combo_set_popdown_strings (GTK_COMBO (filesel->mask_entry), filesel->masks);
}

GtkType
gtk_file_selection_get_type (void)
{
  static GtkType file_selection_type = 0;

  if (!file_selection_type)
    {
      static const GtkTypeInfo filesel_info =
      {
	"GtkFileSelection",
	sizeof (GtkFileSelection),
	sizeof (GtkFileSelectionClass),
	(GtkClassInitFunc) gtk_file_selection_class_init,
	(GtkObjectInitFunc) gtk_file_selection_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };

      file_selection_type = gtk_type_unique (GTK_TYPE_WINDOW, &filesel_info);
    }

  return file_selection_type;
}

static void
gtk_file_selection_class_init (GtkFileSelectionClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GtkObjectClass*) class;

  parent_class = gtk_type_class (GTK_TYPE_WINDOW);

  widget_class = GTK_WIDGET_CLASS (class);

  widget_class->realize = gtk_file_selection_realize;
  object_class->destroy = gtk_file_selection_destroy;
}

static void
gtk_file_selection_init (GtkFileSelection *filesel)
{
  GtkWidget *entry_vbox;
  GtkWidget *label;
  GtkWidget *list_vbox;
  GtkWidget *confirm_area;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *hbox2;
  GtkWidget *table;
  GtkWidget *pulldown_hbox;
  GtkWidget *scrolled_win;
  GtkWidget *mask_label;
  GtkWidget *bigframe;
  GtkWidget *button;
  GtkWidget *hpaned;
  GtkWidget *menu_item;
  GtkWidget *pixmap;

  char *dir_title [2];
  char *file_title [2];
  
  filesel->cmpl_state = cmpl_init_state ();

  filesel->mask=NULL;
  filesel->prev_history=NULL;
  filesel->next_history=NULL;
  filesel->saved_entry=NULL;
  filesel->bookmark_list=NULL;
  filesel->masks=NULL;
  filesel->selection_text = NULL;
  filesel->fileop_data = NULL;

  gtk_file_selection_load_masks(filesel);
  gtk_file_selection_load_bookmarks(filesel);

  /* The dialog-sized vertical box  */
  filesel->main_vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (filesel), 0);
  gtk_container_add (GTK_CONTAINER (filesel), filesel->main_vbox);
  gtk_widget_show (filesel->main_vbox);

  /* hbox for pulldown menu */
  pulldown_hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (filesel->main_vbox), pulldown_hbox, FALSE, FALSE, 0);
  gtk_widget_show (pulldown_hbox);
  
  /* The horizontal box containing create, rename etc. buttons */

/*
  filesel->button_area = gtk_hbutton_box_new ();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(filesel->button_area), GTK_BUTTONBOX_START);
  gtk_button_box_set_spacing(GTK_BUTTON_BOX(filesel->button_area), 0);
  gtk_box_pack_start (GTK_BOX (pulldown_hbox), filesel->button_area,
		      FALSE, FALSE, 0);
  gtk_button_box_set_child_size(GTK_BUTTON_BOX(filesel->button_area),0,0);
  gtk_button_box_set_child_ipadding(GTK_BUTTON_BOX(filesel->button_area),0,0);
  */

  filesel->button_area = gtk_hbox_new (TRUE,0);
  //gtk_box_pack_start (GTK_BOX (pulldown_hbox), filesel->button_area,
  //		      FALSE, FALSE, 0);

  //gtk_widget_show (filesel->button_area);

  gtk_file_selection_show_fileop_buttons(filesel);
  /*  frame to put the following hbox in  */
  bigframe = gtk_frame_new (NULL);
  gtk_box_pack_start (GTK_BOX (filesel->main_vbox), bigframe, TRUE, TRUE, 0);
  gtk_frame_set_shadow_type (GTK_FRAME (bigframe), GTK_SHADOW_OUT);
  gtk_widget_show (bigframe);


  list_vbox = gtk_vbox_new (FALSE,3);
  gtk_widget_show(list_vbox);
  gtk_container_add (GTK_CONTAINER(bigframe), list_vbox);
  gtk_container_set_border_width (GTK_CONTAINER (list_vbox),2);
  gtk_widget_show (list_vbox);
  
  /*  The horizontal box containing the directory and file listboxes  */
//  list_hbox = gtk_hbox_new (FALSE, 3);
  //gtk_container_add (GTK_CONTAINER(bigframe), list_hbox);
  //gtk_container_set_border_width (GTK_CONTAINER (list_hbox), 3);
//  gtk_box_pack_start(GTK_BOX(list_vbox), list_hbox, FALSE,FALSE,0);
//  gtk_widget_show (list_hbox);

  hpaned=gtk_hpaned_new();
  gtk_widget_show(hpaned);
  gtk_container_set_border_width (GTK_CONTAINER (hpaned), 1);
  gtk_paned_set_gutter_size (GTK_PANED (hpaned), 10);
  gtk_box_pack_start (GTK_BOX(list_vbox), hpaned,TRUE,TRUE,0);

  /* vbox to put the buttons and directory listing in  */
  vbox = gtk_vbox_new (FALSE, 3);
  gtk_widget_show (vbox);
  gtk_container_add(GTK_CONTAINER(hpaned),vbox);
  //gtk_box_pack_start (GTK_BOX (hpaned), vbox, FALSE, FALSE, 0);

  hbox = gtk_hbox_new (FALSE, 4);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

//  home_button = gtk_button_new_with_label (_("Home"));
//  gtk_widget_show (home_button);
//  gtk_signal_connect (GTK_OBJECT (home_button), "clicked",
//		      (GtkSignalFunc) gtk_file_selection_home_button,
//		      (gpointer) filesel);
//  gtk_box_pack_start (GTK_BOX (hbox), home_button, TRUE,TRUE, 0);

  /* Here we add the bookmark menu button */
  #define If we're going to make bookmark a menu, we don't need 
  #define   to keep it in the filesel structure
  button=gtk_button_new_with_label(_("Bookmarks"));
  gtk_widget_show(button);
  gtk_box_pack_start (GTK_BOX(hbox), button, FALSE,FALSE,0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) gtk_file_selection_bookmark_button,
		      (gpointer) filesel);
  
  hbox2 = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), hbox2, FALSE, FALSE, 0);
  gtk_widget_show(hbox2);

  /* Prev button */
  button = gtk_button_new ();
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) gtk_file_selection_prev_button,
		      (gpointer) filesel);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE,FALSE, 0);
  pixmap = create_pixmap (filesel->main_vbox, (gpointer) back_xpm);
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (button), pixmap);

  /* Up button */
  button = gtk_button_new ();
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) gtk_file_selection_up_button,
		      (gpointer) filesel);
  gtk_widget_show (button);
  gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE,FALSE, 0);
  pixmap = create_pixmap (filesel->main_vbox, (gpointer) up_xpm);
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (button), pixmap);

  /* next button */
  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) gtk_file_selection_next_button,
		      (gpointer) filesel);
  gtk_box_pack_start (GTK_BOX (hbox2), button, FALSE,FALSE, 0);
  pixmap = create_pixmap (filesel->main_vbox, (gpointer) forward_xpm);
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (button), pixmap);

  /* refresh button */
  button = gtk_button_new ();
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) gtk_file_selection_refresh_button,
		      (gpointer) filesel);
  gtk_box_pack_end (GTK_BOX (hbox), button, FALSE,FALSE, 0);
  pixmap = create_pixmap (filesel->main_vbox, (gpointer) refresh_xpm);
  gtk_widget_show (pixmap);
  gtk_container_add (GTK_CONTAINER (button), pixmap);

  /* menu for right click file operations */
  filesel->fileop_menu = gtk_menu_new();
  
  menu_item = gtk_menu_item_new_with_label ("Rename...");
  gtk_widget_show(menu_item);
  gtk_signal_connect_object (GTK_OBJECT (menu_item), "activate",
			     (GtkSignalFunc) gtk_file_selection_rename_file,
			     (gpointer) filesel);
  gtk_menu_append (GTK_MENU (filesel->fileop_menu), menu_item);

  menu_item = gtk_menu_item_new_with_label ("Delete");
  gtk_widget_show(menu_item);
  gtk_menu_append (GTK_MENU (filesel->fileop_menu), menu_item);
  gtk_signal_connect_object (GTK_OBJECT (menu_item), "activate",
			     (GtkSignalFunc) gtk_file_selection_delete_file,
			     (gpointer) filesel);

  menu_item = gtk_menu_item_new ();
  gtk_widget_show(menu_item);
  gtk_menu_append (GTK_MENU (filesel->fileop_menu), menu_item);

  menu_item = gtk_menu_item_new_with_label ("Create Directory...");
  gtk_signal_connect_object (GTK_OBJECT (menu_item), "activate",
			     (GtkSignalFunc) gtk_file_selection_create_dir, 
			     (gpointer) filesel);
  gtk_widget_show(menu_item);
  gtk_menu_append (GTK_MENU (filesel->fileop_menu), menu_item);

  menu_item = gtk_menu_item_new ();
  gtk_menu_append (GTK_MENU (filesel->fileop_menu), menu_item);
  gtk_widget_show(menu_item);

  menu_item = gtk_menu_item_new_with_label ("Properties...");
  gtk_signal_connect_object (GTK_OBJECT (menu_item), "activate",
			     (GtkSignalFunc) gtk_file_selection_properties,
			     (gpointer) filesel);
  gtk_menu_append (GTK_MENU (filesel->fileop_menu), menu_item);
  gtk_widget_show(menu_item);

  /* The directories clist */
  dir_title[0] = _("Directories");
  dir_title[1] = NULL;
  filesel->dir_list = gtk_clist_new_with_titles (1, (gchar**) dir_title);
  gtk_widget_set_usize (filesel->dir_list, DIR_LIST_WIDTH, DIR_LIST_HEIGHT);
  gtk_signal_connect (GTK_OBJECT (filesel->dir_list), "select_row",
		      (GtkSignalFunc) gtk_file_selection_dir_button,
		      (gpointer) filesel);
  gtk_signal_connect (GTK_OBJECT (filesel->dir_list), "unselect_row",
		      (GtkSignalFunc) gtk_file_selection_undir_button,
		      (gpointer) filesel);
  gtk_signal_connect (GTK_OBJECT (filesel->dir_list), "button_press_event",
		      GTK_SIGNAL_FUNC(gtk_file_selection_show_fileop_menu), 
		      (gpointer) filesel);

  gtk_clist_column_titles_passive (GTK_CLIST (filesel->dir_list));

  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrolled_win), filesel->dir_list);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox), scrolled_win, TRUE,TRUE, 0);
  //gtk_container_add(GTK_CONTAINER(hpaned), scrolled_win);
  
  gtk_widget_show (filesel->dir_list);
  gtk_widget_show (scrolled_win);

  vbox = gtk_vbox_new (FALSE, 3);
  gtk_widget_show (vbox);
  gtk_container_add(GTK_CONTAINER(hpaned),vbox);
  /* vbox area for mask entry and files clist  */

  hbox = gtk_hbox_new (FALSE, 2);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);

  mask_label = gtk_label_new (_("Mask:"));
  gtk_widget_show (mask_label);
  gtk_box_pack_start (GTK_BOX (hbox), mask_label, FALSE, FALSE, 2);

/*
  filesel->mask_entry = gtk_entry_new ();
  gtk_widget_show (filesel->mask_entry);
  gtk_signal_connect(GTK_OBJECT(filesel->mask_entry),"activate",
		     (GtkSignalFunc) gtk_file_4_mask_entry_callback,
		     (gpointer) filesel);
  gtk_box_pack_start (GTK_BOX (hbox),filesel->mask_entry, TRUE, TRUE, 0);
  */

  filesel->mask_entry = gtk_combo_new ();
  gtk_widget_show (filesel->mask_entry);
  gtk_combo_set_value_in_list(GTK_COMBO(filesel->mask_entry),FALSE,FALSE);
  gtk_signal_connect(GTK_OBJECT(GTK_COMBO(filesel->mask_entry)->entry),"activate",
		     (GtkSignalFunc) gtk_file_selection_mask_entry_callback,
		     (gpointer) filesel);
  gtk_signal_connect(GTK_OBJECT(((GtkCombo *)filesel->mask_entry)->entry),"key-press-event",				
		     (GtkSignalFunc) gtk_file_selection_mask_entry_key_callback,
		     (gpointer) filesel);

  gtk_signal_connect(GTK_OBJECT(((GtkCombo *)filesel->mask_entry)->list),"button-release-event",
		     (GtkSignalFunc) gtk_file_selection_mask_entry_button_callback,
		     (gpointer) filesel);
  gtk_box_pack_start (GTK_BOX (hbox),filesel->mask_entry, TRUE, TRUE, 0);

  if (filesel->masks)
    gtk_combo_set_popdown_strings (GTK_COMBO (filesel->mask_entry), filesel->masks);


  /* The files clist */
  file_title[0] = _("Files");
  file_title[1] = NULL;
  filesel->file_list = gtk_clist_new_with_titles (1, (gchar**) file_title);
  gtk_widget_set_usize (filesel->file_list, FILE_LIST_WIDTH, FILE_LIST_HEIGHT);
  gtk_signal_connect (GTK_OBJECT (filesel->file_list), "select_row",
		      (GtkSignalFunc) gtk_file_selection_file_button, 
		      (gpointer) filesel);
  gtk_signal_connect (GTK_OBJECT (filesel->file_list), "key-press-event",
		      (GtkSignalFunc) gtk_file_selection_files_list_key_callback,
		      (gpointer) filesel);
  
  gtk_signal_connect (GTK_OBJECT (filesel->file_list), "button_press_event",
		      GTK_SIGNAL_FUNC(gtk_file_selection_show_fileop_menu), 
		      (gpointer) filesel);

  gtk_clist_column_titles_passive (GTK_CLIST (filesel->file_list));

  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrolled_win), filesel->file_list);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_box_pack_start (GTK_BOX (vbox), scrolled_win, TRUE, TRUE, 0);
  gtk_widget_show (filesel->file_list);
  gtk_widget_show (scrolled_win);

  /* action area for packing buttons into. */
  filesel->action_area = gtk_hbox_new (TRUE, 0);
  gtk_box_pack_start (GTK_BOX (filesel->main_vbox), filesel->action_area, 
		      FALSE, FALSE, 2);
  gtk_widget_show (filesel->action_area);

  /*
  hbox=gtk_hbox_new(FALSE,0);
  gtk_box_pack_end (GTK_BOX (filesel->main_vbox), hbox, FALSE,FALSE, 0);
  gtk_widget_show (hbox);
  */

  /*  The selection entry widget  */
  
  entry_vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_end (GTK_BOX (filesel->main_vbox), entry_vbox, FALSE, FALSE, 0);
  gtk_widget_show (entry_vbox);

  table = gtk_table_new ( 2, 2, FALSE );
  gtk_box_pack_start (GTK_BOX (entry_vbox), table, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table), 2);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);


  label = gtk_label_new (_("Selection:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (label);


  filesel->selection_entry = gtk_entry_new ();
  gtk_signal_connect (GTK_OBJECT (filesel->selection_entry), "key_press_event",
		      (GtkSignalFunc) gtk_file_selection_key_press, filesel);
  gtk_table_attach (GTK_TABLE (table), filesel->selection_entry, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (filesel->selection_entry);


  label = gtk_label_new (_("Directory:"));
  gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (label);
  

  filesel->history_combo = gtk_combo_new();
  gtk_combo_set_value_in_list(GTK_COMBO(filesel->history_combo),FALSE,FALSE);
  gtk_table_attach (GTK_TABLE (table), filesel->history_combo, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show(filesel->history_combo);

  gtk_signal_connect(GTK_OBJECT(((GtkCombo *)filesel->history_combo)->entry),"key-press-event",				    
		     (GtkSignalFunc) gtk_file_selection_history_combo_callback,
		     (gpointer) filesel);

  gtk_signal_connect(GTK_OBJECT(((GtkCombo *)filesel->history_combo)->list),"button-press-event",
		     (GtkSignalFunc) gtk_file_selection_history_combo_list_callback,
		     (gpointer) filesel);

  gtk_signal_connect(GTK_OBJECT(((GtkCombo *)filesel->history_combo)->list),"key-press-event",
		     (GtkSignalFunc) gtk_file_selection_history_combo_list_key_handler,
		     (gpointer) filesel);

  filesel->selection_text = NULL;


  /*  The OK/Cancel button area */
  confirm_area = gtk_hbutton_box_new ();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(confirm_area), GTK_BUTTONBOX_END);
  gtk_button_box_set_spacing(GTK_BUTTON_BOX(confirm_area), 5);
  gtk_box_pack_end (GTK_BOX (entry_vbox), confirm_area, FALSE, FALSE, 0);
  gtk_widget_show (confirm_area);

  /*  The OK button  */
  filesel->ok_button = gtk_button_new_with_label (_("OK"));
  GTK_WIDGET_SET_FLAGS (filesel->ok_button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX (confirm_area), filesel->ok_button, TRUE, TRUE, 0);
  gtk_signal_connect_object (GTK_OBJECT (filesel->selection_entry), "focus_in_event",
			     (GtkSignalFunc) gtk_widget_grab_default,
			     GTK_OBJECT (filesel->ok_button));
  gtk_signal_connect_object (GTK_OBJECT (filesel->selection_entry), "activate",
                             (GtkSignalFunc) gtk_button_clicked,
                             GTK_OBJECT (filesel->ok_button));
  gtk_widget_grab_default (filesel->ok_button);
  gtk_widget_show (filesel->ok_button);

  /*  The Cancel button  */
  filesel->cancel_button = gtk_button_new_with_label (_("Cancel"));
  GTK_WIDGET_SET_FLAGS (filesel->cancel_button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX (confirm_area), filesel->cancel_button, TRUE, TRUE, 0);
  gtk_widget_show (filesel->cancel_button);

  gtk_widget_show(table);


  /*
  filesel->selection_text = label = gtk_label_new ("");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (entry_vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  */


  if (!cmpl_state_okay (filesel->cmpl_state))
    {
      gchar err_buf[256];

      sprintf (err_buf, _("Directory unreadable: %s"), cmpl_strerror (cmpl_errno));

      /*
      gtk_label_set_text (GTK_LABEL (filesel->selection_text), err_buf);
      */
    }
  else
    {
      gtk_file_selection_populate (filesel, "", FALSE);
    }

  gtk_widget_grab_focus (filesel->selection_entry);
}

GtkWidget*
gtk_file_selection_new (const gchar *title)
{
  GtkFileSelection *filesel;

  filesel = gtk_type_new (GTK_TYPE_FILE_SELECTION);
  gtk_window_set_title (GTK_WINDOW (filesel), title);
  /* !!! put check here to figure out if screen > 640x480, if true
     We need to make the file selection dialog bigger. much bigger.. 
     or maybe we should keep it at a certan percentage of the screen
     size? */

  gtk_window_set_default_size(GTK_WINDOW (filesel), 520, 420);
  return GTK_WIDGET (filesel);
}

void
gtk_file_selection_show_fileop_buttons (GtkFileSelection *filesel)
{
  g_return_if_fail (filesel != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (filesel));

  return;
	
  /* delete, create directory, and rename */
/*
  if (!filesel->fileop_c_dir) 
    {
      filesel->fileop_c_dir = gtk_button_new_with_label (_("MkDir"));
      gtk_signal_connect (GTK_OBJECT (filesel->fileop_c_dir), "clicked",
			  (GtkSignalFunc) gtk_file_selection_create_dir, 
			  (gpointer) filesel);
      gtk_box_pack_start (GTK_BOX (filesel->button_area), 
			  filesel->fileop_c_dir, TRUE,TRUE, 0);
      gtk_widget_show (filesel->fileop_c_dir);
    }
	
  if (!filesel->fileop_del_file) 
    {
      filesel->fileop_del_file = gtk_button_new_with_label (_("Delete"));
      gtk_signal_connect (GTK_OBJECT (filesel->fileop_del_file), "clicked",
			  (GtkSignalFunc) gtk_file_selection_delete_file, 
			  (gpointer) filesel);
      gtk_box_pack_start (GTK_BOX (filesel->button_area), 
			  filesel->fileop_del_file, TRUE,TRUE, 0);
      gtk_widget_show (filesel->fileop_del_file);
    }

  if (!filesel->fileop_ren_file)
    {
      filesel->fileop_ren_file = gtk_button_new_with_label (_("Rename"));
      gtk_signal_connect (GTK_OBJECT (filesel->fileop_ren_file), "clicked",
			  (GtkSignalFunc) gtk_file_selection_rename_file, 
			  (gpointer) filesel);
      gtk_box_pack_start (GTK_BOX (filesel->button_area), 
			  filesel->fileop_ren_file, TRUE,TRUE, 0);
      gtk_widget_show (filesel->fileop_ren_file);
    }

  gtk_widget_queue_resize(GTK_WIDGET(filesel));
  */
}

void       
gtk_file_selection_hide_fileop_buttons (GtkFileSelection *filesel)
{
  g_return_if_fail (filesel != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (filesel));
  
  return;
  /*
  if (filesel->fileop_ren_file) 
    {
      gtk_widget_destroy (filesel->fileop_ren_file);
      filesel->fileop_ren_file = NULL;
    }

  if (filesel->fileop_del_file)
    {
      gtk_widget_destroy (filesel->fileop_del_file);
      filesel->fileop_del_file = NULL;
    }

  if (filesel->fileop_c_dir)
    {
      gtk_widget_destroy (filesel->fileop_c_dir);
      filesel->fileop_c_dir = NULL;
    }
  */
}



void
gtk_file_selection_set_filename (GtkFileSelection *filesel,
				 const gchar      *filename)
{
  char  buf[MAXPATHLEN];
  const char *name, *last_slash;

  g_return_if_fail (filesel != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (filesel));
  g_return_if_fail (filename != NULL);

  last_slash = strrchr (filename, '/');

  if (!last_slash)
    {
      buf[0] = 0;
      name = filename;
    }
  else
    {
      gint len = MIN (MAXPATHLEN - 1, last_slash - filename + 1);

      strncpy (buf, filename, len);
      buf[len] = 0;

      name = last_slash + 1;
    }

  gtk_file_selection_populate (filesel, buf, FALSE);

  if (filesel->selection_entry)
    gtk_entry_set_text (GTK_ENTRY (filesel->selection_entry), name);
}

gchar*
gtk_file_selection_get_filename (GtkFileSelection *filesel)
{
  static char nothing[2] = "";
  char *text;
  char *filename;

  g_return_val_if_fail (filesel != NULL, nothing);
  g_return_val_if_fail (GTK_IS_FILE_SELECTION (filesel), nothing);

  text = gtk_entry_get_text (GTK_ENTRY (filesel->selection_entry));
  if (text)
    {
      filename = cmpl_completion_fullname (text, filesel->cmpl_state);
      return filename;
    }

  return nothing;
}

void
gtk_file_selection_complete (GtkFileSelection *filesel,
			     const gchar      *pattern)
{
  gchar *new_pattern;
  gint x;
	
  g_return_if_fail (filesel != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (filesel));
  g_return_if_fail (pattern != NULL);

  if (filesel->selection_entry)
    gtk_entry_set_text (GTK_ENTRY (filesel->selection_entry), pattern);
	
  if(strchr(pattern,'*') || strchr(pattern,'?'))
    {
      for(x=strlen(pattern);x>=0;x--)
	{
	  if(pattern[x]=='/') break;
	}
      gtk_entry_set_text(GTK_ENTRY(filesel->mask_entry),g_strdup(pattern+x+1));
      
      if(filesel->mask) g_free(filesel->mask);
      
      filesel->mask=g_strdup(pattern+x+1);
      new_pattern=g_strdup(pattern);
      new_pattern[x+1]=0;
      gtk_file_selection_populate (filesel, (gchar*) new_pattern, TRUE);
      g_free(new_pattern);
    }
  else
    {
      gtk_file_selection_populate (filesel, (gchar*) pattern, TRUE);
    }
}

static void
gtk_file_selection_realize (GtkWidget *widget)
{
  GtkFileSelection *filesel;
  const gchar *masks[] = { "All Files <*>", NULL };

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (widget));

  filesel = GTK_FILE_SELECTION (widget);

  /* make sure that we have at least one mask */
  if (!filesel->masks)
    gtk_file_selection_set_masks (filesel, masks);

  filesel->mask = g_strdup ((gchar*) filesel->masks->data);
  gtk_file_selection_populate (filesel, "", FALSE);


  if (GTK_WIDGET_CLASS (parent_class)->realize)
    (* GTK_WIDGET_CLASS (parent_class)->realize) (widget);
}

static void
gtk_file_selection_destroy (GtkObject *object)
{
  GtkFileSelection *filesel;
  GList *list;

  g_return_if_fail (object != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (object));

  filesel = GTK_FILE_SELECTION (object);
  
  if (filesel->fileop_dialog)
    gtk_widget_destroy (filesel->fileop_dialog);
  
  if (filesel->next_history)
    {
      list = filesel->next_history;
      while (list)
  	{
	  g_free (list->data);
	  list = list->next;
  	}
    }
  g_list_free (filesel->next_history);
  filesel->next_history = NULL;

  if (filesel->prev_history)
    {
      list = filesel->prev_history;
      while (list)
  	{
	  g_free (list->data);
	  list = list->next;
  	}
    }
  g_list_free (filesel->prev_history);
  filesel->prev_history = NULL;

  if (filesel->mask)
    {
      g_free (filesel->mask);
      filesel->mask = NULL;
    }
  
  cmpl_free_state (filesel->cmpl_state);
  filesel->cmpl_state = NULL;

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

/* Begin file operations callbacks */

static gint
gtk_file_selection_show_fileop_menu (GtkCList *clist, GdkEvent *event, GtkFileSelection *fs)
{
  GdkEventButton *event_button;

  g_return_val_if_fail (clist != NULL, FALSE);
  g_return_val_if_fail (GTK_IS_CLIST (clist), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);
  g_return_val_if_fail (fs != NULL, FALSE);
  g_return_val_if_fail (GTK_FILE_SELECTION (fs), FALSE);
  
  if (event->type == GDK_BUTTON_PRESS)
    {
      event_button = (GdkEventButton *) event;
      if (event_button->button == 3)
	{

	  gtk_menu_popup (GTK_MENU (fs->fileop_menu), NULL, NULL, NULL, NULL, 
			  event_button->button, event_button->time);
	  return TRUE;
	}
    }
  
  return FALSE;
}

static void
gtk_file_selection_fileop_error (GtkFileSelection *fs, gchar *error_message)
{
  GtkWidget *label;
  GtkWidget *vbox;
  GtkWidget *button;
  GtkWidget *dialog;
  
  g_return_if_fail (error_message != NULL);
  
  /* main dialog */
  dialog = gtk_dialog_new ();
  /*
  gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
		      (GtkSignalFunc) gtk_file_selection_fileop_destroy, 
		      (gpointer) fs);
  */
  gtk_window_set_title (GTK_WINDOW (dialog), _("Error"));
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
  
  /* If file dialog is grabbed, make this dialog modal too */
  /* When error dialog is closed, file dialog will be grabbed again */
  if (GTK_WINDOW(fs)->modal)
      gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), vbox,
		     FALSE, FALSE, 0);
  gtk_widget_show(vbox);

  label = gtk_label_new(error_message);
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.0);
  gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
  gtk_widget_show(label);

  /* yes, we free it */
  g_free (error_message);
  
  /* close button */
  button = gtk_button_new_with_label (_("Close"));
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy, 
			     (gpointer) dialog);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button, TRUE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);

  gtk_widget_show (dialog);
}

static void
gtk_file_selection_fileop_destroy (GtkWidget *widget, gpointer data)
{
  GtkFileSelection *fs = data;

  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));
  
  fs->fileop_dialog = NULL;
  g_free (fs->fileop_data);
  fs->fileop_data = NULL;
}


static void
gtk_file_selection_create_dir_confirmed (GtkWidget *widget, gpointer data)
{
  GtkFileSelection *fs = data;
  gchar *dirname;
  gchar *path;
  gchar *full_path;
  gchar *buf;
  CompletionState *cmpl_state;
  
  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));

  dirname = gtk_entry_get_text (GTK_ENTRY (fs->fileop_entry));
  cmpl_state = (CompletionState*) fs->cmpl_state;
  path = cmpl_reference_position (cmpl_state);
  
  full_path = g_strconcat (path, "/", dirname, NULL);
  if ( (mkdir (full_path, 0755) < 0) ) 
    {
      buf = g_strconcat ("Error creating directory \"", dirname, "\":  ", 
			 g_strerror(errno), NULL);
      gtk_file_selection_fileop_error (fs, buf);
    }
  g_free (full_path);
  
  gtk_widget_destroy (fs->fileop_dialog);
  gtk_file_selection_populate (fs, "", FALSE);
}
  
static void
gtk_file_selection_create_dir (gpointer data)
{
  GtkFileSelection *fs = data;
  GtkWidget *label;
  GtkWidget *dialog;
  GtkWidget *vbox;
  GtkWidget *button;

  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));

  if (fs->fileop_dialog)
	  return;
  
  /* main dialog */
  fs->fileop_dialog = dialog = gtk_dialog_new ();
  gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
		      (GtkSignalFunc) gtk_file_selection_fileop_destroy, 
		      (gpointer) fs);
  gtk_window_set_title (GTK_WINDOW (dialog), _("Create Directory"));
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);

  /* If file dialog is grabbed, grab option dialog */
  /* When option dialog is closed, file dialog will be grabbed again */
  if (GTK_WINDOW(fs)->modal)
      gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);

  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), vbox,
		     FALSE, FALSE, 0);
  gtk_widget_show(vbox);
  
  label = gtk_label_new(_("Directory name:"));
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.0);
  gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
  gtk_widget_show(label);

  /*  The directory entry widget  */
  fs->fileop_entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (vbox), fs->fileop_entry, 
		      TRUE, TRUE, 5);
  GTK_WIDGET_SET_FLAGS(fs->fileop_entry, GTK_CAN_DEFAULT);
  gtk_widget_show (fs->fileop_entry);
  
  /* buttons */
  button = gtk_button_new_with_label (_("Create"));
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) gtk_file_selection_create_dir_confirmed, 
		      (gpointer) fs);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button, TRUE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_show(button);
  
  button = gtk_button_new_with_label (_("Cancel"));
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy, 
			     (gpointer) dialog);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button, TRUE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);

  gtk_widget_show (dialog);
}

static void
gtk_file_selection_delete_file_confirmed (GtkWidget *widget, gpointer data)
{
  GtkFileSelection *fs = data;
  CompletionState *cmpl_state;
  gchar *path;
  gchar *full_path;
  gchar *buf;
  
  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));

  cmpl_state = (CompletionState*) fs->cmpl_state;
  path = cmpl_reference_position (cmpl_state);
  
  full_path = g_strconcat (path, "/", fs->fileop_file, NULL);
  if ( (unlink (full_path) < 0) ) 
    {
      buf = g_strconcat ("Error deleting file \"", fs->fileop_file, "\":  ", 
			 g_strerror(errno), NULL);
      gtk_file_selection_fileop_error (fs, buf);
    }
  g_free (full_path);
  
  gtk_widget_destroy (fs->fileop_dialog);
  gtk_file_selection_populate (fs, "", FALSE);
}

static void
gtk_file_selection_delete_file (gpointer data)
{
  GtkFileSelection *fs = data;
  GtkWidget *label;
  GtkWidget *vbox;
  GtkWidget *button;
  GtkWidget *dialog;
  gchar *filename;
  gchar *buf;
  
  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));

  if (fs->fileop_dialog)
	  return;

  filename = gtk_entry_get_text (GTK_ENTRY (fs->selection_entry));
  if (strlen(filename) < 1)
	  return;

  fs->fileop_file = filename;
  
  /* main dialog */
  fs->fileop_dialog = dialog = gtk_dialog_new ();
  gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
		      (GtkSignalFunc) gtk_file_selection_fileop_destroy, 
		      (gpointer) fs);
  gtk_window_set_title (GTK_WINDOW (dialog), _("Delete File"));
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);

  /* If file dialog is grabbed, grab option dialog */
  /* When option dialog is closed, file dialog will be grabbed again */
  if (GTK_WINDOW(fs)->modal)
      gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);
  
  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 8);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), vbox,
		     FALSE, FALSE, 0);
  gtk_widget_show(vbox);

  buf = g_strconcat ("Really delete file \"", filename, "\" ?", NULL);
  label = gtk_label_new(buf);
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.0);
  gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
  gtk_widget_show(label);
  g_free(buf);
  
  /* buttons */
  button = gtk_button_new_with_label (_("Delete"));
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) gtk_file_selection_delete_file_confirmed, 
		      (gpointer) fs);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button, TRUE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_show(button);
  
  button = gtk_button_new_with_label (_("Cancel"));
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy, 
			     (gpointer) dialog);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button, TRUE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);

  gtk_widget_show (dialog);

}

static void
gtk_file_selection_rename_file_confirmed (GtkWidget *widget, gpointer data)
{
  GtkFileSelection *fs = data;
  gchar *buf;
  gchar *file;
  gchar *path;
  gchar *new_filename;
  gchar *old_filename;
  CompletionState *cmpl_state;
  
  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));

  file = gtk_entry_get_text (GTK_ENTRY (fs->fileop_entry));
  cmpl_state = (CompletionState*) fs->cmpl_state;
  path = cmpl_reference_position (cmpl_state);
  
  new_filename = g_strconcat (path, "/", file, NULL);
  old_filename = g_strconcat (path, "/", fs->fileop_file, NULL);

  if (strcmp (new_filename, old_filename))
    if ((rename (old_filename, new_filename)) < 0) 
      {
        buf = g_strconcat ("Error renaming file \"", file, "\":  ", 
                           g_strerror(errno), NULL);
        gtk_file_selection_fileop_error (fs, buf);
      }
  g_free (new_filename);
  g_free (old_filename);
  
  gtk_widget_destroy (fs->fileop_dialog);
  gtk_file_selection_populate (fs, "", FALSE);
}
  
static void
gtk_file_selection_file_mode_confirmed (GtkWidget *widget, gpointer data)
{
  GtkFileSelection *fs = data;
  PropertiesPrivate *priv = fs->fileop_data;
  CompletionState *cmpl_state;
  gchar *filename, *file, *path;
  mode_t mode;

  mode = gtk_file_selection_properties_get_mode (priv);

  file = gtk_entry_get_text (GTK_ENTRY (fs->fileop_entry));
  cmpl_state = (CompletionState*) fs->cmpl_state;
  path = cmpl_reference_position (cmpl_state);

  filename = g_strconcat (path, "/", file, NULL);
  if (chmod (filename, mode) == -1)
    {
      gchar *buf = g_strconcat ("Error changing file mode of \"", filename, "\":  ", 
                                g_strerror (errno), NULL);
      gtk_file_selection_fileop_error (fs, buf);
      gtk_widget_destroy (fs->fileop_dialog);
      gtk_file_selection_populate (fs, "", FALSE);
    }
  else
    gtk_file_selection_rename_file_confirmed (widget, data);

  g_free (filename);
}

static void
gtk_file_selection_rename_file (gpointer data)
{
  GtkFileSelection *fs = data;
  GtkWidget *label;
  GtkWidget *dialog;
  GtkWidget *vbox;
  GtkWidget *button;
  gchar *buf;
  
  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));

  if (fs->fileop_dialog)
	  return;

  fs->fileop_file = gtk_entry_get_text (GTK_ENTRY (fs->selection_entry));
  if (strlen(fs->fileop_file) < 1)
	  return;
  
  /* main dialog */
  fs->fileop_dialog = dialog = gtk_dialog_new ();
  gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
		      (GtkSignalFunc) gtk_file_selection_fileop_destroy, 
		      (gpointer) fs);
  gtk_window_set_title (GTK_WINDOW (dialog), _("Rename File"));
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);

  /* If file dialog is grabbed, grab option dialog */
  /* When option dialog  closed, file dialog will be grabbed again */
  if (GTK_WINDOW(fs)->modal)
    gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);
  
  vbox = gtk_vbox_new(FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER(vbox), 8);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), vbox,
		     FALSE, FALSE, 0);
  gtk_widget_show(vbox);
  
  buf = g_strconcat ("Rename file \"", fs->fileop_file, "\" to:", NULL);
  label = gtk_label_new(buf);
  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.0);
  gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 5);
  gtk_widget_show(label);
  g_free(buf);

  /* New filename entry */
  fs->fileop_entry = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (vbox), fs->fileop_entry, 
		      TRUE, TRUE, 5);
  GTK_WIDGET_SET_FLAGS(fs->fileop_entry, GTK_CAN_DEFAULT);
  gtk_widget_show (fs->fileop_entry);
  
  gtk_entry_set_text (GTK_ENTRY (fs->fileop_entry), fs->fileop_file);
  gtk_editable_select_region (GTK_EDITABLE (fs->fileop_entry),
			      0, strlen (fs->fileop_file));

  /* buttons */
  button = gtk_button_new_with_label (_("Rename"));
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) gtk_file_selection_rename_file_confirmed, 
		      (gpointer) fs);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button, TRUE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_show(button);
  
  button = gtk_button_new_with_label (_("Cancel"));
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy, 
			     (gpointer) dialog);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button, TRUE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);

  gtk_widget_show (dialog);
}

static mode_t
gtk_file_selection_properties_get_mode (PropertiesPrivate* priv)
{
  mode_t mode = 0;

  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->mode_buttons[0])))
    mode |= S_IRUSR;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->mode_buttons[1])))
    mode |= S_IWUSR;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->mode_buttons[2])))
    mode |= S_IXUSR;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->mode_buttons[3])))
    mode |= S_ISUID;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->mode_buttons[4])))
    mode |= S_IRGRP;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->mode_buttons[5])))
    mode |= S_IWGRP;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->mode_buttons[6])))
    mode |= S_IXGRP;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->mode_buttons[7])))
    mode |= S_ISGID;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->mode_buttons[8])))
    mode |= S_IROTH;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->mode_buttons[9])))
    mode |= S_IWOTH;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->mode_buttons[10])))
    mode |= S_IXOTH;
  if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (priv->mode_buttons[11])))
    mode |= S_ISVTX;

  return mode;
}

static void
gtk_file_selection_properties_update_mode (GtkWidget *widget, gpointer data)
{
  GtkFileSelection *fs = data;
  PropertiesPrivate *priv = fs->fileop_data;
  gchar str[8];

  sprintf (str, "(%.4o)", gtk_file_selection_properties_get_mode (priv));
  gtk_label_set (GTK_LABEL (priv->mode_label), str);
}

static void
gtk_file_selection_properties (gpointer data)
{
  GtkFileSelection *fs = data;
  GtkWidget *label;
  GtkWidget *dialog;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget *notebook;
  GtkWidget *table;
  GtkWidget *hseparator;
  GtkWidget *entry;
  GtkWidget *togglebutton;
  struct stat statbuf;
  struct passwd *pw;
  struct group *gp;
  gchar *buf;
  gchar *path;
  gchar *filename;
  gchar timeBuf[TIME_STRING_BUF];
  gint pagenum = 0;
  PropertiesPrivate *priv;
  int i;

  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));

  if (fs->fileop_dialog)
	  return;
  
  /* main dialog */
  fs->fileop_dialog = dialog = gtk_dialog_new ();
  gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
		      (GtkSignalFunc) gtk_file_selection_fileop_destroy, 
		      (gpointer) fs);
  priv = fs->fileop_data = g_malloc (sizeof (PropertiesPrivate));

  gtk_window_set_title (GTK_WINDOW (dialog), ("Properties"));
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);

  /* If file dialog is grabbed, grab option dialog */
  /* When option dialog  closed, file dialog will be grabbed again */
  if (GTK_WINDOW(fs)->modal)
    gtk_window_set_modal (GTK_WINDOW(dialog), TRUE);
  
  /* Dialog guts go here */
  notebook = gtk_notebook_new ();
  gtk_widget_show (notebook);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG(dialog)->vbox), notebook, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (notebook), 8);

  path = cmpl_reference_position(fs->cmpl_state);
  fs->fileop_file = gtk_entry_get_text (GTK_ENTRY (fs->selection_entry));
  filename = g_strconcat(path, "/", fs->fileop_file, NULL);
  if (strlen(fs->fileop_file) > 0 && !(stat(filename, &statbuf))) 
  {
    /* stats page */
    table = gtk_table_new (9, 2, FALSE);
    gtk_widget_show (table);
    gtk_container_add (GTK_CONTAINER (notebook), table);
    gtk_container_set_border_width (GTK_CONTAINER (table), 5);
    gtk_table_set_row_spacings (GTK_TABLE (table), 4);
    gtk_table_set_col_spacings (GTK_TABLE (table), 6);
    
    label = gtk_label_new (_("Statistics"));
    gtk_widget_show (label);
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), pagenum), label);
    pagenum++;
    /* path and filename */
    label = gtk_label_new (_("Path:"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    
    label = gtk_label_new (_(path));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    
    label = gtk_label_new (_("File Name:"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    
    fs->fileop_entry = entry = gtk_entry_new ();
    gtk_widget_show (entry);
    gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_entry_set_text (GTK_ENTRY (entry), fs->fileop_file);
    if (access (filename, W_OK))
      gtk_widget_set_sensitive( GTK_WIDGET (entry), FALSE);
    
    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_table_attach (GTK_TABLE (table), hseparator, 0, 2, 2, 3,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    
    /* file type and size */
    label = gtk_label_new (_("Type:"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    switch (statbuf.st_mode & S_IFMT)
    {
    case S_IFSOCK:
      buf = g_strdup ("Socket");
      break;
    case S_IFLNK:
      buf = g_strdup ("Symbolic link");
      break;
    case S_IFREG:
      buf = g_strdup ("File");
      break;
    case S_IFBLK:
      buf = g_strdup ("Block device");
      break;
    case S_IFDIR:
      buf = g_strdup ("Directory");
      break;
    case S_IFCHR:
      buf = g_strdup ("Character device");
      break;
    case S_IFIFO:
      buf = g_strdup ("First-in/first-out pipe");
      break;
    default:
      buf = g_strdup ("Unknown");
    }


    label = gtk_label_new (buf);
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 1, 2, 3, 4,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    
    label = gtk_label_new (_("Size:"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 4, 5,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    
    label = gtk_label_new (_(g_strdup_printf ("%ld bytes", statbuf.st_size)));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 1, 2, 4, 5,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    
    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_table_attach (GTK_TABLE (table), hseparator, 0, 2, 5, 6,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    
    /* file dates */
    label = gtk_label_new (_("Created:"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 6, 7,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    
    strftime (timeBuf, TIME_STRING_BUF, "%a %b %d %X %Y", localtime(&statbuf.st_mtime));
    label = gtk_label_new (_(timeBuf));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 1, 2, 6, 7,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    
    
    
    label = gtk_label_new (_("Modified:"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 7, 8,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    
    strftime (timeBuf, TIME_STRING_BUF, "%a %b %d %X %Y", localtime(&statbuf.st_mtime));
    label = gtk_label_new (_(timeBuf));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 1, 2, 7, 8,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    
    
    label = gtk_label_new (_("Accessed:"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 8, 9,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    
    strftime (timeBuf, TIME_STRING_BUF, "%a %b %d %X %Y", localtime(&statbuf.st_atime));
    label = gtk_label_new (_(timeBuf));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 1, 2, 8, 9,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);  
    
    /* permissions page */
    vbox = gtk_vbox_new (FALSE, 4);
    gtk_widget_show (vbox);
    gtk_container_add (GTK_CONTAINER (notebook), vbox);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
    
    label = gtk_label_new (_("Permissions"));
    gtk_widget_show (label);
    gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), pagenum), label);
    pagenum++;

    /* owner / group */
    table = gtk_table_new (2, 2, FALSE);
    gtk_widget_show (table);
    gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, TRUE, 0);
    gtk_table_set_row_spacings (GTK_TABLE (table), 2);
    gtk_table_set_col_spacings (GTK_TABLE (table), 8);
    
    label = gtk_label_new (_("Owner:"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);

    entry = gtk_entry_new();
    gtk_widget_show (entry);
    gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 0, 1,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    if ((pw = getpwuid(statbuf.st_uid)))
      gtk_entry_set_text(GTK_ENTRY (entry), pw->pw_name);
    else
      gtk_entry_set_text(GTK_ENTRY (entry), (gpointer) statbuf.st_uid);
    if (access (filename, W_OK) || (getuid() != 0))
      gtk_widget_set_sensitive( GTK_WIDGET (entry), FALSE);


    label = gtk_label_new (_("Group:"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    
    entry = gtk_entry_new();
    gtk_widget_show (entry);
    gtk_table_attach (GTK_TABLE (table), entry, 1, 2, 1, 2,
		      (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    if ((gp = getgrgid(statbuf.st_gid)))
      gtk_entry_set_text(GTK_ENTRY (entry), gp->gr_name);
    else
      gtk_entry_set_text(GTK_ENTRY (entry), (gpointer) statbuf.st_gid);
    if (access (filename, W_OK) || (getuid() != 0))
      gtk_widget_set_sensitive( GTK_WIDGET (entry), FALSE);

    
    hseparator = gtk_hseparator_new ();
    gtk_widget_show (hseparator);
    gtk_box_pack_start (GTK_BOX (vbox), hseparator, FALSE, TRUE, 0);
    
    /* permissions */
    table = gtk_table_new (4, 5, TRUE);
    gtk_widget_show (table);
    gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);
    gtk_table_set_row_spacings (GTK_TABLE (table), 2);
    gtk_table_set_col_spacings (GTK_TABLE (table), 4);
    if (access (filename, W_OK) || ((getuid() != statbuf.st_uid) && getuid() != 0))
      gtk_widget_set_sensitive (GTK_WIDGET (table), FALSE);
    
    hbox = gtk_hbox_new (FALSE, 1);
    gtk_widget_show (hbox);
    gtk_table_attach (GTK_TABLE (table), hbox, 0, 1, 0, 1,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (GTK_FILL), 0, 0);
    
    priv->mode_label = label = gtk_label_new ("(0000)");
    gtk_widget_show (label);
    gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

    label = gtk_label_new (_("Read"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 1, 2, 0, 1,
		      (GtkAttachOptions) (0),
		      (GtkAttachOptions) (0), 0, 0);
    
    label = gtk_label_new (_("Write"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 2, 3, 0, 1,
		      (GtkAttachOptions) (0),
		      (GtkAttachOptions) (0), 0, 0);
    
    label = gtk_label_new (_("Exec"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 3, 4, 0, 1,
		      (GtkAttachOptions) (0),
		      (GtkAttachOptions) (0), 0, 0);
    
    label = gtk_label_new (_("Special"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 4, 5, 0, 1,
		      (GtkAttachOptions) (0),
		      (GtkAttachOptions) (0), 0, 0);
    
    
    label = gtk_label_new (_("User:"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    
    priv->mode_buttons[0] = togglebutton = gtk_toggle_button_new_with_label ("");
    gtk_widget_show (togglebutton);
    gtk_table_attach (GTK_TABLE (table), togglebutton, 1, 2, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    if ((statbuf.st_mode & ~(S_IFMT)) & S_IRUSR)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (togglebutton), TRUE);

    priv->mode_buttons[1] = togglebutton = gtk_toggle_button_new_with_label ("");
    gtk_widget_show (togglebutton);
    gtk_table_attach (GTK_TABLE (table), togglebutton, 2, 3, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    if ((statbuf.st_mode & ~(S_IFMT)) & S_IWUSR)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (togglebutton), TRUE);
    
    priv->mode_buttons[2] = togglebutton = gtk_toggle_button_new_with_label ("");
    gtk_widget_show (togglebutton);
    gtk_table_attach (GTK_TABLE (table), togglebutton, 3, 4, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    if ((statbuf.st_mode & ~(S_IFMT)) & S_IXUSR)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (togglebutton), TRUE);
    
    priv->mode_buttons[3] = togglebutton = gtk_toggle_button_new_with_label ("");
    gtk_widget_show (togglebutton);
    gtk_table_attach (GTK_TABLE (table), togglebutton, 4, 5, 1, 2,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    if ((statbuf.st_mode & ~(S_IFMT)) & S_ISUID)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (togglebutton), TRUE);


    
    label = gtk_label_new (_("Group:"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 2, 3,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    
    priv->mode_buttons[4] = togglebutton = gtk_toggle_button_new_with_label ("");
    gtk_widget_show (togglebutton);
    gtk_table_attach (GTK_TABLE (table), togglebutton, 1, 2, 2, 3,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    if ((statbuf.st_mode & ~(S_IFMT)) & S_IRGRP)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (togglebutton), TRUE);
    
    priv->mode_buttons[5] = togglebutton = gtk_toggle_button_new_with_label ("");
    gtk_widget_show (togglebutton);
    gtk_table_attach (GTK_TABLE (table), togglebutton, 2, 3, 2, 3,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    if ((statbuf.st_mode & ~(S_IFMT)) & S_IWGRP)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (togglebutton), TRUE);
    
    priv->mode_buttons[6] = togglebutton = gtk_toggle_button_new_with_label ("");
    gtk_widget_show (togglebutton);
    gtk_table_attach (GTK_TABLE (table), togglebutton, 3, 4, 2, 3,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    if ((statbuf.st_mode & ~(S_IFMT)) & S_IXGRP)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (togglebutton), TRUE);
    
    priv->mode_buttons[7] = togglebutton = gtk_toggle_button_new_with_label ("");
    gtk_widget_show (togglebutton);
    gtk_table_attach (GTK_TABLE (table), togglebutton, 4, 5, 2, 3,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    if ((statbuf.st_mode & ~(S_IFMT)) & S_ISGID)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (togglebutton), TRUE);
    
    label = gtk_label_new (_("Other:"));
    gtk_widget_show (label);
    gtk_table_attach (GTK_TABLE (table), label, 0, 1, 3, 4,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
    
    priv->mode_buttons[8] = togglebutton = gtk_toggle_button_new_with_label ("");
    gtk_widget_show (togglebutton);
    gtk_table_attach (GTK_TABLE (table), togglebutton, 1, 2, 3, 4,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    if ((statbuf.st_mode & ~(S_IFMT)) & S_IROTH)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (togglebutton), TRUE);

    priv->mode_buttons[9] = togglebutton = gtk_toggle_button_new_with_label ("");
    gtk_widget_show (togglebutton);
    gtk_table_attach (GTK_TABLE (table), togglebutton, 2, 3, 3, 4,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    if ((statbuf.st_mode & ~(S_IFMT)) & S_IWOTH)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (togglebutton), TRUE);
    
    priv->mode_buttons[10] = togglebutton = gtk_toggle_button_new_with_label ("");
    gtk_widget_show (togglebutton);
    gtk_table_attach (GTK_TABLE (table), togglebutton, 3, 4, 3, 4,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    if ((statbuf.st_mode & ~(S_IFMT)) & S_IXOTH)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (togglebutton), TRUE);
    
    priv->mode_buttons[11] = togglebutton = gtk_toggle_button_new_with_label ("");
    gtk_widget_show (togglebutton);
    gtk_table_attach (GTK_TABLE (table), togglebutton, 4, 5, 3, 4,
		      (GtkAttachOptions) (GTK_FILL),
		      (GtkAttachOptions) (0), 0, 0);
    if ((statbuf.st_mode & ~(S_IFMT)) & S_ISVTX)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (togglebutton), TRUE);

    for (i = 0; i < 12; i++)
      gtk_signal_connect (GTK_OBJECT (priv->mode_buttons[i]), "toggled",
                          GTK_SIGNAL_FUNC (gtk_file_selection_properties_update_mode), fs);
    gtk_toggle_button_toggled (GTK_TOGGLE_BUTTON (priv->mode_buttons[0]));
  }
  /* global page */
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (notebook), vbox);

  label = gtk_label_new (_("Global"));
  gtk_widget_show (label);
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), pagenum), label);
  pagenum++;

  label = gtk_label_new (_("dialog preferances will go here"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);

  /* end of dialog guts */

  /* buttons */
  button = gtk_button_new_with_label (_("OK"));
  //  gtk_signal_connect (GTK_OBJECT (button), "clicked",
  //		      (GtkSignalFunc) gtk_file_selection_rename_file_confirmed, 
  //		      (gpointer) fs);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      (GtkSignalFunc) gtk_file_selection_file_mode_confirmed, 
		      (gpointer) fs);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button, TRUE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_show(button);
  
  button = gtk_button_new_with_label (_("Cancel"));
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
			     (GtkSignalFunc) gtk_widget_destroy, 
			     (gpointer) dialog);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area),
		     button, TRUE, TRUE, 0);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);

  g_free (filename);
  gtk_widget_show (dialog);
}

static gint
gtk_file_selection_key_press (GtkWidget   *widget,
			      GdkEventKey *event,
			      gpointer     user_data)
{
	
  GtkFileSelection *fs;
  char *text;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  fs = GTK_FILE_SELECTION (user_data);

  if (fs->saved_entry)
    {
      gtk_clist_unselect_all ((GtkCList *) (fs->dir_list));
      gtk_entry_set_text(GTK_ENTRY(fs->selection_entry),fs->saved_entry);
      g_free (fs->saved_entry);
      fs->saved_entry = NULL;
    }
  if (event->keyval == GDK_Tab)
    {
      text = gtk_entry_get_text (GTK_ENTRY (fs->selection_entry));

      text = g_strdup (text);

      gtk_file_selection_populate (fs, text, TRUE);

      g_free (text);

      gtk_signal_emit_stop_by_name (GTK_OBJECT (widget), "key_press_event");

      return TRUE;
    }


  return FALSE;
}

/*
static void
gtk_file_selection_home_button (GtkWidget *widget, gpointer data){
  GList *list;
	
  GtkFileSelection *fs=data;

  list = fs->next_history;
  if (list)
    {
      g_free (list->data);
      list = list->next;
    }
  g_list_free (fs->next_history);
  fs->next_history = NULL;
		
  gtk_file_selection_populate (fs,"~/",FALSE);
}
*/
static void
gtk_file_selection_bookmark_button (GtkWidget *widget, 
                                    GtkFileSelection *fs)
{

  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_FILE_SELECTION (fs));
	 
  gtk_menu_popup (GTK_MENU (fs->bookmark_menu), NULL, NULL, NULL, NULL, 
		  0, 0);
  
}

static void
gtk_file_selection_up_button (GtkWidget *widget, gpointer data){
  GtkFileSelection *fs = data;
  GList *list;

  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));

  list = fs->next_history;
  if (list)
    {
      g_free (list->data);
      list = list->next;
    }
  g_list_free (fs->next_history);
  fs->next_history = NULL;

  gtk_file_selection_populate (fs, "../", FALSE); /*change directories. */
		
}

static void
gtk_file_selection_prev_button (GtkWidget *widget, gpointer data){
  GtkFileSelection *fs = data;
  GList *list;
  GList *first;
  gchar *path;

  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));

  list = fs->prev_history;

  if (list && g_list_length(list) > 1)
    {
      first = list;            /* get first element */
      list = list->next;       /* pop off current directory */

      list->prev = NULL;       /* make this the new head. */
  	
      fs->prev_history = list; /* update prev_history list */
      fs->next_history = g_list_prepend(fs->next_history,first->data); /* put it on next_history */
  	
      first->next = NULL;      /* orphan the old first node */
      g_list_free (first);     /* free the node (data is now in use by next_history) */


  	
      path = g_malloc(strlen(list->data)+4); /* plenty of space */
      strcpy(path,list->data);               /* get the 2nd path in the history */
      strcat(path,"/");                      /* append a '/' */
      gtk_file_selection_populate (fs, path, FALSE); /* change directories. */
      g_free (path);
    }
}	

static void
gtk_file_selection_next_button (GtkWidget *widget, gpointer data){
  GtkFileSelection *fs = data;
  GList *list;
  GList *first;
  gchar *path;

  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));

  list = fs->next_history;

  if (list && g_list_length(list) > 0)
    {
      first = list;            /*get first element*/
      list = list->next;       /*pop off current directory*/
      
      if (list)
	list->prev = NULL;
      
      fs->next_history = list;                       /*update prev_history list*/
  	
      path = g_malloc(strlen(first->data)+4);        /*plenty of space*/
      strcpy(path,first->data);
      strcat(path,"/");                              /*append a /   */
      gtk_file_selection_populate (fs, path, FALSE); /*change directories.*/
      g_free(path);
  	
      first->next = NULL;     /* orphan the old first node */
      g_list_free (first);    /* free the node (data is now in use by next_history) */
      
    }
}	

void static
gtk_file_selection_refresh_button (GtkWidget *widget, gpointer data){
  GtkFileSelection *fs = data;

  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));

  gtk_file_selection_populate (fs,"",FALSE);
}

static void
gtk_file_selection_mask_entry_callback (GtkWidget *widget, gpointer data){
  GtkFileSelection *fs = data;

  if(fs->mask)
    g_free (fs->mask);
		
  fs->mask = g_strdup(gtk_entry_get_text (GTK_ENTRY(GTK_COMBO(fs->mask_entry)->entry)));
	
  if (strlen(fs->mask) == 0)
    {
      g_free (fs->mask);
      fs->mask = NULL;
    }
	
  gtk_file_selection_refresh_button (widget,data);
}

static gint gtk_file_selection_files_list_key_callback (GtkWidget *widget, GdkEventKey *event, gpointer data){
  GtkFileSelection *fs=data;
  gchar *saved;
  gchar key[2];

//  g_print("Key event: %d\n",event->keyval);
  //we need some sort of timeout.

  //if the key is a normal character then
  //add to our saved_entry1
  //if it's backspace then remove one character
  //otherwise let it through (and erase our buffer.

  if(event->keyval > GDK_space && event->keyval <=  GDK_Korean_Won) {
    key[1]=0;
    key[0]=event->keyval;
    saved=fs->saved_entry1;
    if(fs->saved_entry1){
      fs->saved_entry1=g_strconcat(saved,key,NULL);
      g_free(saved);
    }else{
      fs->saved_entry1=g_strdup(key);
    }
    g_print("complete: %s\n",fs->saved_entry1);
    /*gtk_label_set_text(GTK_LABEL(fs->completion_label), fs->saved_entry1); */

    saved=g_strdup(gtk_entry_get_text(GTK_ENTRY(fs->selection_entry)));
    gtk_file_selection_complete(fs,fs->saved_entry1);
    gtk_entry_set_text(GTK_ENTRY(fs->selection_entry),saved);
    g_free(saved);
  }else if (event->keyval ==  GDK_BackSpace) {
    if(strlen(fs->saved_entry1)){
      fs->saved_entry1[strlen(fs->saved_entry1)-1]=0;
      g_print("complete: %s\n",fs->saved_entry1);
      /*gtk_label_set_text(GTK_LABEL(fs->completion_label),fs->saved_entry1); */
      saved=g_strdup(gtk_entry_get_text(GTK_ENTRY(fs->selection_entry)));
      gtk_file_selection_complete(fs,fs->saved_entry1);
      gtk_entry_set_text(GTK_ENTRY(fs->selection_entry),saved);
      g_free(saved);
    }
  }else if (event->keyval == GDK_Tab) {
    saved=g_strdup(gtk_entry_get_text(GTK_ENTRY(fs->selection_entry)));
    gtk_file_selection_populate(fs,fs->saved_entry1,TRUE);
    g_free(fs->saved_entry1);
    fs->saved_entry1=gtk_entry_get_text(GTK_ENTRY(fs->selection_entry));
    gtk_entry_set_text(GTK_ENTRY(fs->selection_entry),saved);
    g_free(saved);

    g_print("complete: %s\n",fs->saved_entry1);
    /* gtk_label_set_text(GTK_LABEL(fs->completion_label),fs->saved_entry1);*/

    gtk_signal_emit_stop_by_name (GTK_OBJECT (widget), "key_press_event");
  }else {
    if(fs->saved_entry1){
      g_free(fs->saved_entry1);
      fs->saved_entry1=NULL;
    }
    /* gtk_label_set_text(GTK_LABEL(fs->completion_label)," "); */
  }

  return TRUE;
}


static gint gtk_file_selection_mask_entry_key_callback (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  GtkEntry *entry=(GtkEntry *)widget;
  GtkFileSelection *fs=data;
	
  g_return_val_if_fail (fs != NULL,FALSE);
  g_return_val_if_fail (GTK_IS_FILE_SELECTION (fs),FALSE);
	

  if (event->keyval == GDK_Return || event->keyval == GDK_Tab)
    {
      if(fs->mask)
        g_free(fs->mask);

      fs->mask=g_strdup(gtk_entry_get_text(entry));
      gtk_file_selection_refresh_button(widget,fs);

      if (event->keyval == GDK_Return)
        gtk_signal_emit_stop_by_name (GTK_OBJECT (widget), "key_press_event");
      return TRUE;
    }
  else
    {
      return FALSE;
    }
}

static gint gtk_file_selection_mask_entry_button_callback (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  GtkFileSelection *fs = data;
		
  if(fs->mask)
    g_free(fs->mask);

  fs->mask=g_strdup(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(fs->mask_entry)->entry)));
  gtk_file_selection_refresh_button(widget,fs);

  return TRUE;

}

static gboolean gtk_file_selection_history_combo_list_key_handler(GtkWidget *widget,
								  GdkEventKey *event,
								  gpointer user_data)
{
  /*
  g_print("Key pressed! \n");
  */
	
  return TRUE;
}

static gboolean gtk_file_selection_history_combo_list_callback (GtkWidget *thelist,
								GdkEventButton *event,
								gpointer user_data)
{

  GtkFileSelection *fs = user_data;
  GList *list;
  gchar *path;
		
  list = fs->next_history;
  if(list)
    {
      g_free (list->data);
      list = list->next;
    }
  g_list_free (fs->next_history);
  fs->next_history = NULL;
			
  path = g_malloc(strlen(gtk_entry_get_text(GTK_ENTRY (((GtkCombo *)fs->history_combo)->entry)))+4);
  strcpy (path,gtk_entry_get_text(GTK_ENTRY( ((GtkCombo *)fs->history_combo)->entry)));
  strcat (path,"/");
	
  gtk_file_selection_populate (fs,path,TRUE);
	
  g_free (path);

  return TRUE;
}

static gboolean
gtk_file_selection_history_combo_callback (GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  GtkEntry *entry=(GtkEntry *)widget;
  GtkFileSelection *fs=data;
  GList *list;
  gchar *path;
	
  g_return_val_if_fail (fs != NULL,FALSE);
  g_return_val_if_fail (GTK_IS_FILE_SELECTION (fs),FALSE);
	

  if (event->keyval == GDK_Return)
    {
      list = fs->next_history;
      if (list)
	{
	  g_free (list->data);
	  list = list->next;
	}
      g_list_free (fs->next_history);
      fs->next_history = NULL;
      
      path = g_malloc(strlen(gtk_entry_get_text(entry))+4);
      strcpy (path,gtk_entry_get_text(entry));
      strcat (path,"/");
      gtk_file_selection_populate (fs,path,TRUE);
      g_free (path);
      gtk_signal_emit_stop_by_name (GTK_OBJECT (widget), "key_press_event");
      return TRUE;
    }
  else
    {
      return FALSE;
    }

}


static void gtk_file_selection_bookmark_callback (GtkWidget *widget, gpointer data)
{
  GtkFileSelection *fs = data;
  BookmarkMenuStruct *item;
  GList *list;
	
  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));

//g_print ("Callback\n");
  list = fs->bookmark_list;
  while(list) {
    item = list->data;
    if (item->menu_item == widget) {
      if(strcmp(item->path,"./")) {
        gtk_file_selection_populate (fs, item->path, FALSE);
      }
      break;
    }
    list=list->next;
  } 
}

static void
gtk_file_selection_update_history_menu (GtkFileSelection *fs,
					gchar *current_directory)
{
  gchar *current_dir;

  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));
  g_return_if_fail (current_directory != NULL);
  
  current_dir = g_strdup (current_directory);

  if(fs->prev_history)
    {
      if (strcmp((fs->prev_history)->data,current_dir))
	{ /*if this item isn't on the top of the list */
	  fs->prev_history = g_list_prepend(fs->prev_history,g_strdup(current_dir));
	}
    } else {
      fs->prev_history = g_list_prepend(fs->prev_history,g_strdup(current_dir));
    }
  
  gtk_combo_set_popdown_strings (GTK_COMBO (fs->history_combo),fs->prev_history);
  
  g_free (current_dir);
}

static void
gtk_file_selection_file_button (GtkWidget *widget,
			       gint row, 
			       gint column, 
			       GdkEventButton *bevent,
			       gpointer user_data)
{
  GtkFileSelection *fs = NULL;
  gchar *filename, *temp = NULL;
  
  g_return_if_fail (GTK_IS_CLIST (widget));

  fs = user_data;
  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));
  
  gtk_clist_get_text (GTK_CLIST (fs->file_list), row, 0, &temp);
  filename = g_strdup (temp);

  if (filename)
    {
      if (fs->saved_entry)
      {
        gtk_clist_unselect_all ((GtkCList *) (fs->dir_list));
        gtk_entry_set_text(GTK_ENTRY(fs->selection_entry),fs->saved_entry);
        g_free (fs->saved_entry);
        fs->saved_entry = NULL;
      }
      if(fs->saved_entry1){
        g_free(fs->saved_entry1);
        fs->saved_entry1=NULL;
      }
      /* gtk_label_set_text(GTK_LABEL(fs->completion_label)," "); */


      if (bevent)
	switch (bevent->type)
	  {
	  case GDK_2BUTTON_PRESS:
	    gtk_button_clicked (GTK_BUTTON (fs->ok_button));
	    break;
	    
	  default:
/*	
	    if (bevent->button && GDK_BUTTON2_MASK)
	      {
	        g_print("Right click! -- %d\n",bevent->button);
	      }
	      else
	      {
	      */
	
	    gtk_entry_set_text (GTK_ENTRY (fs->selection_entry), filename);
	      /*}*/
	    break;
	  }
      else
	gtk_entry_set_text (GTK_ENTRY (fs->selection_entry), filename);

      g_free (filename);
    }
}

static void
gtk_file_selection_dir_button (GtkWidget *widget,
			       gint row, 
			       gint column, 
			       GdkEventButton *bevent,
			       gpointer user_data)
{
  GList *list;
  GtkFileSelection *fs = NULL;
  gchar *filename, *temp = NULL;

  g_return_if_fail (GTK_IS_CLIST (widget));

  fs = GTK_FILE_SELECTION (user_data);
  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));

  gtk_clist_get_text (GTK_CLIST (fs->dir_list), row, 0, &temp);
  filename = g_strdup (temp);

  if (filename)
    {
      if (bevent)
	switch (bevent->type)
	  {
	  case GDK_2BUTTON_PRESS:
	    list = fs->next_history;
	    if (list)
	      {
		g_free (list->data);
		list = list->next;
	      }
	    g_list_free (fs->next_history);
	    fs->next_history = NULL;
	
	    gtk_file_selection_populate (fs, filename, FALSE);
	    gtk_entry_set_text(GTK_ENTRY(fs->selection_entry),fs->saved_entry);
	    g_free (fs->saved_entry);
	    fs->saved_entry = NULL;
	    break;
	    
	  default:
	    /* here we need to add the "filename" to the beginning of what's already
	       in the entry.  Save what's in the entry, then restore it on the double click
	    */
	    if (fs->saved_entry) g_free (fs->saved_entry);
	    fs->saved_entry=g_strdup(gtk_entry_get_text(GTK_ENTRY (fs->selection_entry)));
	
	    temp=g_strconcat(filename,fs->saved_entry,NULL);
	    gtk_entry_set_text (GTK_ENTRY (fs->selection_entry), temp);
	    g_free (temp);
	
	    break;
	  }
      else
	gtk_entry_set_text (GTK_ENTRY (fs->selection_entry), filename);
      
      g_free (filename);
    }
}

static void
gtk_file_selection_undir_button (GtkWidget *widget,
			       gint row,
			       gint column,
			       GdkEventButton *bevent,
			       gpointer user_data)
{
  GtkFileSelection *fs = NULL;
  gchar *filename, *temp = NULL;

  g_return_if_fail (GTK_IS_CLIST (widget));

  fs = GTK_FILE_SELECTION (user_data);
  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));

  gtk_clist_get_text (GTK_CLIST (fs->dir_list), row, 0, &temp);
  filename = g_strdup (temp);

  if (filename)
    {
      if (bevent)
	switch (bevent->type)
	  {
	  default:
	    /* here we need to add the "filename" to the beginning of what's already
	       in the entry.  Save what's in the entry, then restore it on the double click
	    */
	    if (fs->saved_entry)
	      {
	        gtk_entry_set_text (GTK_ENTRY (fs->selection_entry),fs->saved_entry);
		g_free (fs->saved_entry);
		fs->saved_entry = NULL;
	      }
	    break;
	  }
      else
	gtk_entry_set_text (GTK_ENTRY (fs->selection_entry), filename); //?????

      g_free (filename);
    }
}

static void
gtk_file_selection_populate (GtkFileSelection *fs,
			     gchar            *rel_path,
			     gint              try_complete)
{
  CompletionState *cmpl_state;
  PossibleCompletion* poss;
  gchar* filename;
  gint row;
  gchar* rem_path = rel_path;
  gchar* sel_text;
  gchar* text[2];
  gint did_recurse = FALSE;
  gint possible_count = 0;
  gint selection_index = -1;
  gint file_list_width;
  gint dir_list_width;
  
  g_return_if_fail (fs != NULL);
  g_return_if_fail (GTK_IS_FILE_SELECTION (fs));
  
  cmpl_state = (CompletionState*) fs->cmpl_state;
  poss = cmpl_completion_matches (rel_path, &rem_path, cmpl_state);

  if (!cmpl_state_okay (cmpl_state))
    {
      /* Something went wrong. */
      gtk_file_selection_abort (fs);
      return;
    }

  g_assert (cmpl_state->reference_dir);

  gtk_clist_freeze (GTK_CLIST (fs->dir_list));
  gtk_clist_clear (GTK_CLIST (fs->dir_list));
  gtk_clist_freeze (GTK_CLIST (fs->file_list));
  gtk_clist_clear (GTK_CLIST (fs->file_list));

  /* Set the dir_list to include ./ and ../ */
  /* Actually, no let's not.
  text[1] = NULL;
  text[0] = "./";
  row = gtk_clist_append (GTK_CLIST (fs->dir_list), text);
  */

  text[0] = "../";  //Do we need ..?
  row = gtk_clist_append (GTK_CLIST (fs->dir_list), text);

  /*reset the max widths of the lists*/
  dir_list_width = gdk_string_width(fs->dir_list->style->font,"../");
  gtk_clist_set_column_width(GTK_CLIST(fs->dir_list),0,dir_list_width);
  file_list_width = 1;
  gtk_clist_set_column_width(GTK_CLIST(fs->file_list),0,file_list_width);

  while (poss)
    {
      if (cmpl_is_a_completion (poss))
  	{
	  possible_count += 1;
	  
	  filename = cmpl_this_completion (poss);

	  text[0] = filename;
	  
	  if (cmpl_is_directory (poss))
	    {
	      if (strcmp (filename, "./") != 0 &&
		  strcmp (filename, "../") != 0)
		{
		  int width = gdk_string_width(fs->dir_list->style->font,
					       filename);
		  row = gtk_clist_append (GTK_CLIST (fs->dir_list), text);
		  if(width > dir_list_width)
		    {
		      dir_list_width = width;
		      gtk_clist_set_column_width(GTK_CLIST(fs->dir_list),0,
						 width);
		    }
		}
	    }
	  else
	    {
	      if(fs->mask)
		{
		  if (gtk_file_selection_match_mask(filename,fs->mask))
		    {
		      int width = gdk_string_width(fs->file_list->style->font,
						   filename);
		      row = gtk_clist_append (GTK_CLIST (fs->file_list), text);
		      if(width > file_list_width)
		        {
		          file_list_width = width;
		          gtk_clist_set_column_width(GTK_CLIST(fs->file_list),0,
						     width);
		        }
		    }
		}
	      else
		{
		  int width = gdk_string_width(fs->file_list->style->font,
					       filename);
		  row = gtk_clist_append (GTK_CLIST (fs->file_list), text);
		  if(width > file_list_width)
		    {
		      file_list_width = width;
		      gtk_clist_set_column_width(GTK_CLIST(fs->file_list),0,
						 width);
		    }
		}
	    }
	}

      poss = cmpl_next_completion (cmpl_state);
    }

  gtk_clist_thaw (GTK_CLIST (fs->dir_list));
  gtk_clist_thaw (GTK_CLIST (fs->file_list));

  /* File lists are set. */

  g_assert (cmpl_state->reference_dir);

  if (try_complete)
    {

      /* User is trying to complete filenames, so advance the user's input
       * string to the updated_text, which is the common leading substring
       * of all possible completions, and if its a directory attempt
       * attempt completions in it. */

      if (cmpl_updated_text (cmpl_state)[0])
        {

          if (cmpl_updated_dir (cmpl_state))
            {
	      gchar* dir_name = g_strdup (cmpl_updated_text (cmpl_state));

              did_recurse = TRUE;

              gtk_file_selection_populate (fs, dir_name, TRUE);

              g_free (dir_name);
            }
          else
            {
	      if (fs->selection_entry)
		      gtk_entry_set_text (GTK_ENTRY (fs->selection_entry),
					  cmpl_updated_text (cmpl_state));
            }
        }
      else
        {
          selection_index = cmpl_last_valid_char (cmpl_state) -
                            (strlen (rel_path) - strlen (rem_path));
	  if (fs->selection_entry)
	    gtk_entry_set_text (GTK_ENTRY (fs->selection_entry), rem_path);
        }
    }
  else
    {
      if (fs->selection_entry)
      /* Here we need to take the old filename and keep it!*/
	/*gtk_entry_set_text (GTK_ENTRY (fs->selection_entry), "");*/
	;
    }

  if (!did_recurse)
    {
      if (fs->selection_entry)
	gtk_entry_set_position (GTK_ENTRY (fs->selection_entry), selection_index);

      if (fs->selection_entry)
	{
	  sel_text = g_strconcat (_("Selection: "),
				  cmpl_reference_position (cmpl_state),
				  NULL);

/*
		gtk_label_set_text (GTK_LABEL (fs->selection_text), sel_text);
*/
		g_free (sel_text);
	}

  gtk_file_selection_update_history_menu (fs, cmpl_reference_position (cmpl_state));

    }
}

static void
gtk_file_selection_abort (GtkFileSelection *fs)
{
  gchar err_buf[256];

  sprintf (err_buf, _("Directory unreadable: %s"), cmpl_strerror (cmpl_errno));

  /*  BEEP gdk_beep();  */

/*
  if (fs->selection_entry)
    gtk_label_set_text (GTK_LABEL (fs->selection_text), err_buf);
*/
}

/**********************************************************************/
/*			  External Interface                          */
/**********************************************************************/

/* The four completion state selectors
 */
static gchar*
cmpl_updated_text (CompletionState* cmpl_state)
{
  return cmpl_state->updated_text;
}

static gint
cmpl_updated_dir (CompletionState* cmpl_state)
{
  return cmpl_state->re_complete;
}

static gchar*
cmpl_reference_position (CompletionState* cmpl_state)
{
  return cmpl_state->reference_dir->fullname;
}

static gint
cmpl_last_valid_char (CompletionState* cmpl_state)
{
  return cmpl_state->last_valid_char;
}

static gchar*
cmpl_completion_fullname (gchar* text, CompletionState* cmpl_state)
{
  static char nothing[2] = "";

  if (!cmpl_state_okay (cmpl_state))
    {
      return nothing;
    }
  else if (text[0] == '/')
    {
      strcpy (cmpl_state->updated_text, text);
    }
  else if (text[0] == '~')
    {
      CompletionDir* dir;
      char* slash;

      dir = open_user_dir (text, cmpl_state);

      if (!dir)
	{
	  /* spencer says just return ~something, so
	   * for now just do it. */
	  strcpy (cmpl_state->updated_text, text);
	}
      else
	{

	  strcpy (cmpl_state->updated_text, dir->fullname);

	  slash = strchr (text, '/');

	  if (slash)
	    strcat (cmpl_state->updated_text, slash);
	}
    }
  else
    {
      strcpy (cmpl_state->updated_text, cmpl_state->reference_dir->fullname);
      if (strcmp (cmpl_state->reference_dir->fullname, "/") != 0)
	strcat (cmpl_state->updated_text, "/");
      strcat (cmpl_state->updated_text, text);
    }

  return cmpl_state->updated_text;
}

/* The three completion selectors
 */
static gchar*
cmpl_this_completion (PossibleCompletion* pc)
{
  return pc->text;
}

static gint
cmpl_is_directory (PossibleCompletion* pc)
{
  return pc->is_directory;
}

static gint
cmpl_is_a_completion (PossibleCompletion* pc)
{
  return pc->is_a_completion;
}

/**********************************************************************/
/*	                 Construction, deletion                       */
/**********************************************************************/

static CompletionState*
cmpl_init_state (void)
{
  gchar getcwd_buf[2*MAXPATHLEN];
  CompletionState *new_state;

  new_state = g_new (CompletionState, 1);

  /* We don't use getcwd() on SUNOS, because, it does a popen("pwd")
   * and, if that wasn't bad enough, hangs in doing so.
   */
#if defined(sun) && !defined(__SVR4)
  if (!getwd (getcwd_buf))
#else    
  if (!getcwd (getcwd_buf, MAXPATHLEN))
#endif    
    {
      /* Oh joy, we can't get the current directory. Um..., we should have
       * a root directory, right? Right? (Probably not portable to non-Unix)
       */
      strcpy (getcwd_buf, "/");
    }

tryagain:

  new_state->reference_dir = NULL;
  new_state->completion_dir = NULL;
  new_state->active_completion_dir = NULL;
  new_state->directory_storage = NULL;
  new_state->directory_sent_storage = NULL;
  new_state->last_valid_char = 0;
  new_state->updated_text = g_new (gchar, MAXPATHLEN);
  new_state->updated_text_alloc = MAXPATHLEN;
  new_state->the_completion.text = g_new (gchar, MAXPATHLEN);
  new_state->the_completion.text_alloc = MAXPATHLEN;
  new_state->user_dir_name_buffer = NULL;
  new_state->user_directories = NULL;

  new_state->reference_dir =  open_dir (getcwd_buf, new_state);

  if (!new_state->reference_dir)
    {
      /* Directories changing from underneath us, grumble */
      strcpy (getcwd_buf, "/");
      goto tryagain;
    }

  return new_state;
}

static void
cmpl_free_dir_list(GList* dp0)
{
  GList *dp = dp0;

  while (dp) {
    free_dir (dp->data);
    dp = dp->next;
  }

  g_list_free(dp0);
}

static void
cmpl_free_dir_sent_list(GList* dp0)
{
  GList *dp = dp0;

  while (dp) {
    free_dir_sent (dp->data);
    dp = dp->next;
  }

  g_list_free(dp0);
}

static void
cmpl_free_state (CompletionState* cmpl_state)
{
  cmpl_free_dir_list (cmpl_state->directory_storage);
  cmpl_free_dir_sent_list (cmpl_state->directory_sent_storage);

  if (cmpl_state->user_dir_name_buffer)
    g_free (cmpl_state->user_dir_name_buffer);
  if (cmpl_state->user_directories)
    g_free (cmpl_state->user_directories);
  if (cmpl_state->the_completion.text)
    g_free (cmpl_state->the_completion.text);
  if (cmpl_state->updated_text)
    g_free (cmpl_state->updated_text);

  g_free (cmpl_state);
}

static void
free_dir(CompletionDir* dir)
{
  g_free(dir->fullname);
  g_free(dir);
}

static void
free_dir_sent(CompletionDirSent* sent)
{
  g_free(sent->name_buffer);
  g_free(sent->entries);
  g_free(sent);
}

static void
prune_memory_usage(CompletionState *cmpl_state)
{
  GList* cdsl = cmpl_state->directory_sent_storage;
  GList* cdl = cmpl_state->directory_storage;
  GList* cdl0 = cdl;
  gint len = 0;

  for(; cdsl && len < CMPL_DIRECTORY_CACHE_SIZE; len += 1)
    cdsl = cdsl->next;

  if (cdsl) {
    cmpl_free_dir_sent_list(cdsl->next);
    cdsl->next = NULL;
  }

  cmpl_state->directory_storage = NULL;
  while (cdl) {
    if (cdl->data == cmpl_state->reference_dir)
      cmpl_state->directory_storage = g_list_prepend(NULL, cdl->data);
    else
      free_dir (cdl->data);
    cdl = cdl->next;
  }

  g_list_free(cdl0);
}

/**********************************************************************/
/*                        The main entrances.                         */
/**********************************************************************/

static PossibleCompletion*
cmpl_completion_matches (gchar* text_to_complete,
			 gchar** remaining_text,
			 CompletionState* cmpl_state)
{
  gchar* first_slash;
  PossibleCompletion *poss;

  prune_memory_usage(cmpl_state);

  g_assert (text_to_complete != NULL);

  cmpl_state->user_completion_index = -1;
  cmpl_state->last_completion_text = text_to_complete;
  cmpl_state->the_completion.text[0] = 0;
  cmpl_state->last_valid_char = 0;
  cmpl_state->updated_text_len = -1;
  cmpl_state->updated_text[0] = 0;
  cmpl_state->re_complete = FALSE;

  first_slash = strchr (text_to_complete, '/');

  if (text_to_complete[0] == '~' && !first_slash)
    {
      /* Text starts with ~ and there is no slash, show all the
       * home directory completions.
       */
      poss = attempt_homedir_completion (text_to_complete, cmpl_state);

      update_cmpl(poss, cmpl_state);

      return poss;
    }

  cmpl_state->reference_dir =
    open_ref_dir (text_to_complete, remaining_text, cmpl_state);

  if(!cmpl_state->reference_dir)
    return NULL;

  cmpl_state->completion_dir =
    find_completion_dir (*remaining_text, remaining_text, cmpl_state);

  cmpl_state->last_valid_char = *remaining_text - text_to_complete;

  if(!cmpl_state->completion_dir)
    return NULL;

  cmpl_state->completion_dir->cmpl_index = -1;
  cmpl_state->completion_dir->cmpl_parent = NULL;
  cmpl_state->completion_dir->cmpl_text = *remaining_text;

  cmpl_state->active_completion_dir = cmpl_state->completion_dir;

  cmpl_state->reference_dir = cmpl_state->completion_dir;

  poss = attempt_file_completion(cmpl_state);

  update_cmpl(poss, cmpl_state);

  return poss;
}

static PossibleCompletion*
cmpl_next_completion (CompletionState* cmpl_state)
{
  PossibleCompletion* poss = NULL;

  cmpl_state->the_completion.text[0] = 0;

  if(cmpl_state->user_completion_index >= 0)
    poss = attempt_homedir_completion(cmpl_state->last_completion_text, cmpl_state);
  else
    poss = attempt_file_completion(cmpl_state);

  update_cmpl(poss, cmpl_state);

  return poss;
}

/**********************************************************************/
/*			 Directory Operations                         */
/**********************************************************************/

/* Open the directory where completion will begin from, if possible. */
static CompletionDir*
open_ref_dir(gchar* text_to_complete,
	     gchar** remaining_text,
	     CompletionState* cmpl_state)
{
  gchar* first_slash;
  CompletionDir *new_dir;

  first_slash = strchr(text_to_complete, '/');

  if (text_to_complete[0] == '~')
    {
      new_dir = open_user_dir(text_to_complete, cmpl_state);

      if(new_dir)
	{
	  if(first_slash)
	    *remaining_text = first_slash + 1;
	  else
	    *remaining_text = text_to_complete + strlen(text_to_complete);
	}
      else
	{
	  return NULL;
	}
    }
  else if (text_to_complete[0] == '/' || !cmpl_state->reference_dir)
    {
      gchar *tmp = g_strdup(text_to_complete);
      gchar *p;

      p = tmp;
      while (*p && *p != '*' && *p != '?')
	p++;

      *p = '\0';
      p = strrchr(tmp, '/');
      if (p)
	{
	  if (p == tmp)
	    p++;
      
	  *p = '\0';

	  new_dir = open_dir(tmp, cmpl_state);

	  if(new_dir)
	    *remaining_text = text_to_complete + 
	      ((p == tmp + 1) ? (p - tmp) : (p + 1 - tmp));
	}
      else
	{
	  /* If no possible candidates, use the cwd */
	  gchar *curdir = g_get_current_dir ();
	  
	  new_dir = open_dir(curdir, cmpl_state);

	  if (new_dir)
	    *remaining_text = text_to_complete;

	  g_free (curdir);
	}

      g_free (tmp);
    }
  else
    {
      *remaining_text = text_to_complete;

      new_dir = open_dir(cmpl_state->reference_dir->fullname, cmpl_state);
    }

  if(new_dir)
    {
      new_dir->cmpl_index = -1;
      new_dir->cmpl_parent = NULL;
    }

  return new_dir;
}

/* open a directory by user name */
static CompletionDir*
open_user_dir(gchar* text_to_complete,
	      CompletionState *cmpl_state)
{
  gchar *first_slash;
  gint cmp_len;

  g_assert(text_to_complete && text_to_complete[0] == '~');

  first_slash = strchr(text_to_complete, '/');

  if (first_slash)
    cmp_len = first_slash - text_to_complete - 1;
  else
    cmp_len = strlen(text_to_complete + 1);

  if(!cmp_len)
    {
      /* ~/ */
      gchar *homedir = g_get_home_dir ();

      if (homedir)
	return open_dir(homedir, cmpl_state);
      else
	return NULL;
    }
  else
    {
      /* ~user/ */
      char* copy = g_new(char, cmp_len + 1);
      struct passwd *pwd;
      strncpy(copy, text_to_complete + 1, cmp_len);
      copy[cmp_len] = 0;
      pwd = getpwnam(copy);
      g_free(copy);
      if (!pwd)
	{
	  cmpl_errno = errno;
	  return NULL;
	}

      return open_dir(pwd->pw_dir, cmpl_state);
    }
}

/* open a directory relative the the current relative directory */
static CompletionDir*
open_relative_dir(gchar* dir_name,
		  CompletionDir* dir,
		  CompletionState *cmpl_state)
{
  gchar path_buf[2*MAXPATHLEN];

  if(dir->fullname_len + strlen(dir_name) + 2 >= MAXPATHLEN)
    {
      cmpl_errno = CMPL_ERRNO_TOO_LONG;
      return NULL;
    }

  strcpy(path_buf, dir->fullname);

  if(dir->fullname_len > 1)
    {
      path_buf[dir->fullname_len] = '/';
      strcpy(path_buf + dir->fullname_len + 1, dir_name);
    }
  else
    {
      strcpy(path_buf + dir->fullname_len, dir_name);
    }

  return open_dir(path_buf, cmpl_state);
}

/* after the cache lookup fails, really open a new directory */
static CompletionDirSent*
open_new_dir(gchar* dir_name, struct stat* sbuf, gboolean stat_subdirs)
{
  CompletionDirSent* sent;
  DIR* directory;
  gchar *buffer_ptr;
  struct dirent *dirent_ptr;
  gint buffer_size = 0;
  gint entry_count = 0;
  gint i;
  struct stat ent_sbuf;
  char path_buf[MAXPATHLEN*2];
  gint path_buf_len;

  sent = g_new(CompletionDirSent, 1);
  sent->mtime = sbuf->st_mtime;
  sent->inode = sbuf->st_ino;
  sent->device = sbuf->st_dev;

  path_buf_len = strlen(dir_name);

  if (path_buf_len > MAXPATHLEN)
    {
      cmpl_errno = CMPL_ERRNO_TOO_LONG;
      return NULL;
    }

  strcpy(path_buf, dir_name);

  directory = opendir(dir_name);

  if(!directory)
    {
      cmpl_errno = errno;
      return NULL;
    }

  while((dirent_ptr = readdir(directory)) != NULL)
    {
      int entry_len = strlen(dirent_ptr->d_name);
      buffer_size += entry_len + 1;
      entry_count += 1;

      if(path_buf_len + entry_len + 2 >= MAXPATHLEN)
	{
	  cmpl_errno = CMPL_ERRNO_TOO_LONG;
 	  closedir(directory);
	  return NULL;
	}
    }

  sent->name_buffer = g_new(gchar, buffer_size);
  sent->entries = g_new(CompletionDirEntry, entry_count);
  sent->entry_count = entry_count;

  buffer_ptr = sent->name_buffer;

  rewinddir(directory);

  for(i = 0; i < entry_count; i += 1)
    {
      dirent_ptr = readdir(directory);

      if(!dirent_ptr)
	{
	  cmpl_errno = errno;
	  closedir(directory);
	  return NULL;
	}

      strcpy(buffer_ptr, dirent_ptr->d_name);
      sent->entries[i].entry_name = buffer_ptr;
      buffer_ptr += strlen(dirent_ptr->d_name);
      *buffer_ptr = 0;
      buffer_ptr += 1;

      path_buf[path_buf_len] = '/';
      strcpy(path_buf + path_buf_len + 1, dirent_ptr->d_name);

      if (stat_subdirs)
	{
	  if(stat(path_buf, &ent_sbuf) >= 0 && S_ISDIR(ent_sbuf.st_mode))
	    sent->entries[i].is_dir = 1;
	  else
	    /* stat may fail, and we don't mind, since it could be a
	     * dangling symlink. */
	    sent->entries[i].is_dir = 0;
	}
      else
	sent->entries[i].is_dir = 1;
    }

  qsort(sent->entries, sent->entry_count, sizeof(CompletionDirEntry), compare_cmpl_dir);

  closedir(directory);

  return sent;
}

static gboolean
check_dir(gchar *dir_name, struct stat *result, gboolean *stat_subdirs)
{
  /* A list of directories that we know only contain other directories.
   * Trying to stat every file in these directories would be very
   * expensive.
   */

  static struct {
    gchar *name;
    gboolean present;
    struct stat statbuf;
  } no_stat_dirs[] = {
    { "/afs", FALSE, { 0 } },
    { "/net", FALSE, { 0 } }
  };

  static const gint n_no_stat_dirs = sizeof(no_stat_dirs) / sizeof(no_stat_dirs[0]);
  static gboolean initialized = FALSE;

  gint i;

  if (!initialized)
    {
      initialized = TRUE;
      for (i = 0; i < n_no_stat_dirs; i++)
	{
	  if (stat (no_stat_dirs[i].name, &no_stat_dirs[i].statbuf) == 0)
	    no_stat_dirs[i].present = TRUE;
	}
    }

  if(stat(dir_name, result) < 0)
    {
      cmpl_errno = errno;
      return FALSE;
    }

  *stat_subdirs = TRUE;
  for (i=0; i<n_no_stat_dirs; i++)
    {
      if (no_stat_dirs[i].present &&
	  (no_stat_dirs[i].statbuf.st_dev == result->st_dev) &&
	  (no_stat_dirs[i].statbuf.st_ino == result->st_ino))
	{
	  *stat_subdirs = FALSE;
	  break;
	}
    }

  return TRUE;
}

/* open a directory by absolute pathname */
static CompletionDir*
open_dir(gchar* dir_name, CompletionState* cmpl_state)
{
  struct stat sbuf;
  gboolean stat_subdirs;
  CompletionDirSent *sent;
  GList* cdsl;

  if (!check_dir (dir_name, &sbuf, &stat_subdirs))
    return NULL;

  cdsl = cmpl_state->directory_sent_storage;

  while (cdsl)
    {
      sent = cdsl->data;

      if(sent->inode == sbuf.st_ino &&
	 sent->mtime == sbuf.st_mtime &&
	 sent->device == sbuf.st_dev)
	return attach_dir(sent, dir_name, cmpl_state);

      cdsl = cdsl->next;
    }

  sent = open_new_dir(dir_name, &sbuf, stat_subdirs);

  if (sent) {
    cmpl_state->directory_sent_storage =
      g_list_prepend(cmpl_state->directory_sent_storage, sent);

    return attach_dir(sent, dir_name, cmpl_state);
  }

  return NULL;
}

static CompletionDir*
attach_dir(CompletionDirSent* sent, gchar* dir_name, CompletionState *cmpl_state)
{
  CompletionDir* new_dir;

  new_dir = g_new(CompletionDir, 1);

  cmpl_state->directory_storage =
    g_list_prepend(cmpl_state->directory_storage, new_dir);

  new_dir->sent = sent;
  new_dir->fullname = g_strdup(dir_name);
  new_dir->fullname_len = strlen(dir_name);

  return new_dir;
}

static gint
correct_dir_fullname(CompletionDir* cmpl_dir)
{
  gint length = strlen(cmpl_dir->fullname);
  struct stat sbuf;

  if (strcmp(cmpl_dir->fullname + length - 2, "/.") == 0)
    {
      if (length == 2) 
	{
	  strcpy(cmpl_dir->fullname, "/");
	  cmpl_dir->fullname_len = 1;
	  return TRUE;
	} else {
	  cmpl_dir->fullname[length - 2] = 0;
	}
    }
  else if (strcmp(cmpl_dir->fullname + length - 3, "/./") == 0)
    cmpl_dir->fullname[length - 2] = 0;
  else if (strcmp(cmpl_dir->fullname + length - 3, "/..") == 0)
    {
      if(length == 3)
	{
	  strcpy(cmpl_dir->fullname, "/");
	  cmpl_dir->fullname_len = 1;
	  return TRUE;
	}

      if(stat(cmpl_dir->fullname, &sbuf) < 0)
	{
	  cmpl_errno = errno;
	  return FALSE;
	}

      cmpl_dir->fullname[length - 2] = 0;

      if(!correct_parent(cmpl_dir, &sbuf))
	return FALSE;
    }
  else if (strcmp(cmpl_dir->fullname + length - 4, "/../") == 0)
    {
      if(length == 4)
	{
	  strcpy(cmpl_dir->fullname, "/");
	  cmpl_dir->fullname_len = 1;
	  return TRUE;
	}

      if(stat(cmpl_dir->fullname, &sbuf) < 0)
	{
	  cmpl_errno = errno;
	  return FALSE;
	}

      cmpl_dir->fullname[length - 3] = 0;

      if(!correct_parent(cmpl_dir, &sbuf))
	return FALSE;
    }

  cmpl_dir->fullname_len = strlen(cmpl_dir->fullname);

  return TRUE;
}

static gint
correct_parent(CompletionDir* cmpl_dir, struct stat *sbuf)
{
  struct stat parbuf;
  gchar *last_slash;
  gchar *new_name;
  gchar c = 0;

  last_slash = strrchr(cmpl_dir->fullname, '/');

  g_assert(last_slash);

  if(last_slash != cmpl_dir->fullname)
    { /* last_slash[0] = 0; */ }
  else
    {
      c = last_slash[1];
      last_slash[1] = 0;
    }

  if (stat(cmpl_dir->fullname, &parbuf) < 0)
    {
      cmpl_errno = errno;
      return FALSE;
    }

  if (parbuf.st_ino == sbuf->st_ino && parbuf.st_dev == sbuf->st_dev)
    /* it wasn't a link */
    return TRUE;

  if(c)
    last_slash[1] = c;
  /* else
    last_slash[0] = '/'; */

  /* it was a link, have to figure it out the hard way */

  new_name = find_parent_dir_fullname(cmpl_dir->fullname);

  if (!new_name)
    return FALSE;

  g_free(cmpl_dir->fullname);

  cmpl_dir->fullname = new_name;

  return TRUE;
}

static gchar*
find_parent_dir_fullname(gchar* dirname)
{
  gchar buffer[MAXPATHLEN];
  gchar buffer2[MAXPATHLEN];

#if defined(sun) && !defined(__SVR4)
  if(!getwd(buffer))
#else
  if(!getcwd(buffer, MAXPATHLEN))
#endif    
    {
      cmpl_errno = errno;
      return NULL;
    }

  if(chdir(dirname) != 0 || chdir("..") != 0)
    {
      cmpl_errno = errno;
      return NULL;
    }

#if defined(sun) && !defined(__SVR4)
  if(!getwd(buffer2))
#else
  if(!getcwd(buffer2, MAXPATHLEN))
#endif
    {
      chdir(buffer);
      cmpl_errno = errno;

      return NULL;
    }

  if(chdir(buffer) != 0)
    {
      cmpl_errno = errno;
      return NULL;
    }

  return g_strdup(buffer2);
}

/**********************************************************************/
/*                        Completion Operations                       */
/**********************************************************************/

static PossibleCompletion*
attempt_homedir_completion(gchar* text_to_complete,
			   CompletionState *cmpl_state)
{
  gint index, length;

  if (!cmpl_state->user_dir_name_buffer &&
      !get_pwdb(cmpl_state))
    return NULL;
  length = strlen(text_to_complete) - 1;

  cmpl_state->user_completion_index += 1;

  while(cmpl_state->user_completion_index < cmpl_state->user_directories_len)
    {
      index = first_diff_index(text_to_complete + 1,
			       cmpl_state->user_directories
			       [cmpl_state->user_completion_index].login);

      switch(index)
	{
	case PATTERN_MATCH:
	  break;
	default:
	  if(cmpl_state->last_valid_char < (index + 1))
	    cmpl_state->last_valid_char = index + 1;
	  cmpl_state->user_completion_index += 1;
	  continue;
	}

      cmpl_state->the_completion.is_a_completion = 1;
      cmpl_state->the_completion.is_directory = 1;

      append_completion_text("~", cmpl_state);

      append_completion_text(cmpl_state->
			      user_directories[cmpl_state->user_completion_index].login,
			     cmpl_state);

      return append_completion_text("/", cmpl_state);
    }

  if(text_to_complete[1] ||
     cmpl_state->user_completion_index > cmpl_state->user_directories_len)
    {
      cmpl_state->user_completion_index = -1;
      return NULL;
    }
  else
    {
      cmpl_state->user_completion_index += 1;
      cmpl_state->the_completion.is_a_completion = 1;
      cmpl_state->the_completion.is_directory = 1;

      return append_completion_text("~/", cmpl_state);
    }
}

/* returns the index (>= 0) of the first differing character,
 * PATTERN_MATCH if the completion matches */
static gint
first_diff_index(gchar* pat, gchar* text)
{
  gint diff = 0;

  while(*pat && *text && *text == *pat)
    {
      pat += 1;
      text += 1;
      diff += 1;
    }

  if(*pat)
    return diff;

  return PATTERN_MATCH;
}

static PossibleCompletion*
append_completion_text(gchar* text, CompletionState* cmpl_state)
{
  gint len, i = 1;

  if(!cmpl_state->the_completion.text)
    return NULL;

  len = strlen(text) + strlen(cmpl_state->the_completion.text) + 1;

  if(cmpl_state->the_completion.text_alloc > len)
    {
      strcat(cmpl_state->the_completion.text, text);
      return &cmpl_state->the_completion;
    }

  while(i < len) { i <<= 1; }

  cmpl_state->the_completion.text_alloc = i;

  cmpl_state->the_completion.text = (gchar*)g_realloc(cmpl_state->the_completion.text, i);

  if(!cmpl_state->the_completion.text)
    return NULL;
  else
    {
      strcat(cmpl_state->the_completion.text, text);
      return &cmpl_state->the_completion;
    }
}

static CompletionDir*
find_completion_dir(gchar* text_to_complete,
		    gchar** remaining_text,
		    CompletionState* cmpl_state)
{
  gchar* first_slash = strchr(text_to_complete, '/');
  CompletionDir* dir = cmpl_state->reference_dir;
  CompletionDir* next;
  *remaining_text = text_to_complete;

  while(first_slash)
    {
      gint len = first_slash - *remaining_text;
      gint found = 0;
      gchar *found_name = NULL;         /* Quiet gcc */
      gint i;
      gchar* pat_buf = g_new (gchar, len + 1);

      strncpy(pat_buf, *remaining_text, len);
      pat_buf[len] = 0;

      for(i = 0; i < dir->sent->entry_count; i += 1)
	{
	  if(dir->sent->entries[i].is_dir &&
	     fnmatch(pat_buf, dir->sent->entries[i].entry_name,
		     FNMATCH_FLAGS)!= FNM_NOMATCH)
	    {
	      if(found)
		{
		  g_free (pat_buf);
		  return dir;
		}
	      else
		{
		  found = 1;
		  found_name = dir->sent->entries[i].entry_name;
		}
	    }
	}

      if (!found)
	{
	  /* Perhaps we are trying to open an automount directory */
	  found_name = pat_buf;
	}

      next = open_relative_dir(found_name, dir, cmpl_state);
      
      if(!next)
	{
	  g_free (pat_buf);
	  return NULL;
	}
      
      next->cmpl_parent = dir;
      
      dir = next;
      
      if(!correct_dir_fullname(dir))
	{
	  g_free(pat_buf);
	  return NULL;
	}
      
      *remaining_text = first_slash + 1;
      first_slash = strchr(*remaining_text, '/');

      g_free (pat_buf);
    }

  return dir;
}

static void
update_cmpl(PossibleCompletion* poss, CompletionState* cmpl_state)
{
  gint cmpl_len;

  if(!poss || !cmpl_is_a_completion(poss))
    return;

  cmpl_len = strlen(cmpl_this_completion(poss));

  if(cmpl_state->updated_text_alloc < cmpl_len + 1)
    {
      cmpl_state->updated_text =
	(gchar*)g_realloc(cmpl_state->updated_text,
			  cmpl_state->updated_text_alloc);
      cmpl_state->updated_text_alloc = 2*cmpl_len;
    }

  if(cmpl_state->updated_text_len < 0)
    {
      strcpy(cmpl_state->updated_text, cmpl_this_completion(poss));
      cmpl_state->updated_text_len = cmpl_len;
      cmpl_state->re_complete = cmpl_is_directory(poss);
    }
  else if(cmpl_state->updated_text_len == 0)
    {
      cmpl_state->re_complete = FALSE;
    }
  else
    {
      gint first_diff =
	first_diff_index(cmpl_state->updated_text,
			 cmpl_this_completion(poss));

      cmpl_state->re_complete = FALSE;

      if(first_diff == PATTERN_MATCH)
	return;

      if(first_diff > cmpl_state->updated_text_len)
	strcpy(cmpl_state->updated_text, cmpl_this_completion(poss));

      cmpl_state->updated_text_len = first_diff;
      cmpl_state->updated_text[first_diff] = 0;
    }
}

static PossibleCompletion*
attempt_file_completion(CompletionState *cmpl_state)
{
  gchar *pat_buf, *first_slash;
  CompletionDir *dir = cmpl_state->active_completion_dir;

  dir->cmpl_index += 1;

  if(dir->cmpl_index == dir->sent->entry_count)
    {
      if(dir->cmpl_parent == NULL)
	{
	  cmpl_state->active_completion_dir = NULL;

	  return NULL;
	}
      else
	{
	  cmpl_state->active_completion_dir = dir->cmpl_parent;

	  return attempt_file_completion(cmpl_state);
	}
    }

  g_assert(dir->cmpl_text);

  first_slash = strchr(dir->cmpl_text, '/');

  if(first_slash)
    {
      gint len = first_slash - dir->cmpl_text;

      pat_buf = g_new (gchar, len + 1);
      strncpy(pat_buf, dir->cmpl_text, len);
      pat_buf[len] = 0;
    }
  else
    {
      gint len = strlen(dir->cmpl_text);

      pat_buf = g_new (gchar, len + 2);
      strcpy(pat_buf, dir->cmpl_text);
      strcpy(pat_buf + len, "*");
    }

  if(first_slash)
    {
      if(dir->sent->entries[dir->cmpl_index].is_dir)
	{
	  if(fnmatch(pat_buf, dir->sent->entries[dir->cmpl_index].entry_name,
		     FNMATCH_FLAGS) != FNM_NOMATCH)
	    {
	      CompletionDir* new_dir;

	      new_dir = open_relative_dir(dir->sent->entries[dir->cmpl_index].entry_name,
					  dir, cmpl_state);

	      if(!new_dir)
		{
		  g_free (pat_buf);
		  return NULL;
		}

	      new_dir->cmpl_parent = dir;

	      new_dir->cmpl_index = -1;
	      new_dir->cmpl_text = first_slash + 1;

	      cmpl_state->active_completion_dir = new_dir;

	      g_free (pat_buf);
	      return attempt_file_completion(cmpl_state);
	    }
	  else
	    {
	      g_free (pat_buf);
	      return attempt_file_completion(cmpl_state);
	    }
	}
      else
	{
	  g_free (pat_buf);
	  return attempt_file_completion(cmpl_state);
	}
    }
  else
    {
      if(dir->cmpl_parent != NULL)
	{
	  append_completion_text(dir->fullname +
				 strlen(cmpl_state->completion_dir->fullname) + 1,
				 cmpl_state);
	  append_completion_text("/", cmpl_state);
	}

      append_completion_text(dir->sent->entries[dir->cmpl_index].entry_name, cmpl_state);

      cmpl_state->the_completion.is_a_completion =
	(fnmatch(pat_buf, dir->sent->entries[dir->cmpl_index].entry_name,
		 FNMATCH_FLAGS) != FNM_NOMATCH);

      cmpl_state->the_completion.is_directory = dir->sent->entries[dir->cmpl_index].is_dir;
      if(dir->sent->entries[dir->cmpl_index].is_dir)
	append_completion_text("/", cmpl_state);

      g_free (pat_buf);
      return &cmpl_state->the_completion;
    }
}


static gint
get_pwdb(CompletionState* cmpl_state)
{
  struct passwd *pwd_ptr;
  gchar* buf_ptr;
  gint len = 0, i, count = 0;

  if(cmpl_state->user_dir_name_buffer)
    return TRUE;
  setpwent ();

  while ((pwd_ptr = getpwent()) != NULL)
    {
      len += strlen(pwd_ptr->pw_name);
      len += strlen(pwd_ptr->pw_dir);
      len += 2;
      count += 1;
    }

  setpwent ();

  cmpl_state->user_dir_name_buffer = g_new(gchar, len);
  cmpl_state->user_directories = g_new(CompletionUserDir, count);
  cmpl_state->user_directories_len = count;

  buf_ptr = cmpl_state->user_dir_name_buffer;

  for(i = 0; i < count; i += 1)
    {
      pwd_ptr = getpwent();
      if(!pwd_ptr)
	{
	  cmpl_errno = errno;
	  goto error;
	}

      strcpy(buf_ptr, pwd_ptr->pw_name);
      cmpl_state->user_directories[i].login = buf_ptr;
      buf_ptr += strlen(buf_ptr);
      buf_ptr += 1;
      strcpy(buf_ptr, pwd_ptr->pw_dir);
      cmpl_state->user_directories[i].homedir = buf_ptr;
      buf_ptr += strlen(buf_ptr);
      buf_ptr += 1;
    }

  qsort(cmpl_state->user_directories,
	cmpl_state->user_directories_len,
	sizeof(CompletionUserDir),
	compare_user_dir);

  endpwent();

  return TRUE;

error:

  if(cmpl_state->user_dir_name_buffer)
    g_free(cmpl_state->user_dir_name_buffer);
  if(cmpl_state->user_directories)
    g_free(cmpl_state->user_directories);

  cmpl_state->user_dir_name_buffer = NULL;
  cmpl_state->user_directories = NULL;

  return FALSE;
}

static gint
compare_user_dir(const void* a, const void* b)
{
  return strcmp((((CompletionUserDir*)a))->login,
		(((CompletionUserDir*)b))->login);
}

static gint
compare_cmpl_dir(const void* a, const void* b)
{
  return strcmp((((CompletionDirEntry*)a))->entry_name,
		(((CompletionDirEntry*)b))->entry_name);
}

static gint
cmpl_state_okay(CompletionState* cmpl_state)
{
  return  cmpl_state && cmpl_state->reference_dir;
}

static gchar*
cmpl_strerror(gint err)
{
  if(err == CMPL_ERRNO_TOO_LONG)
    return "Name too long";
  else
    return g_strerror (err);
}

/* This is an internally used function to create pixmaps. */
GtkWidget*
create_pixmap(GtkWidget *widget, const gchar *pixmap_char)
{
  GdkPixmap *gdkpixmap;
  GdkBitmap *mask;
  GtkWidget *pixmap;
  GdkColormap *colormap;

  colormap = gtk_widget_get_colormap (widget);

  gdkpixmap = gdk_pixmap_colormap_create_from_xpm_d (GTK_WIDGET(widget)->window,
						     colormap,
						     &mask,
						     NULL, 
						     (gpointer) pixmap_char);
  if (gdkpixmap == NULL)
    {
      g_warning ("Error loading pixmap: %s", pixmap_char);
      return NULL;
    }
  pixmap = gtk_pixmap_new (gdkpixmap, mask);
  gdk_pixmap_unref (gdkpixmap);
  gdk_bitmap_unref (mask);
  return pixmap;
}


/* Testing area */
#ifdef TORRIE_DEBUG

/* Get the selected filename and print it to the console */
void file_ok_sel( GtkWidget        *w,
                  GtkFileSelection *fs )
{
    g_print ("%s\n", gtk_file_selection_get_filename (GTK_FILE_SELECTION (fs)));
}

void destroy( GtkWidget *widget,
              gpointer   data )
{
    gtk_main_quit ();
}

int main( int   argc,
          char *argv[] )
{
    GtkWidget *filew;
    const gchar *masks[] = { "mp3s/playlists <*.mp3,*.m3u>",
                             "src/hdr <*.[CcHh],*.[Cc][Cc],*.[Hh][Hh],*.cpp>",
                             NULL };

    gtk_init (&argc, &argv);

    /* Create a new file selection widget */
    filew = gtk_file_selection_new ("Spiffy File Selector");
//    gtk_file_selection_complete(GTK_FILE_SELECTION(filew),"bob");

    gtk_file_selection_set_masks (GTK_FILE_SELECTION (filew), masks);
		
    gtk_signal_connect (GTK_OBJECT (filew), "destroy",
			(GtkSignalFunc) destroy, &filew);
    /* Connect the ok_button to file_ok_sel function */
    gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filew)->ok_button),
			"clicked", (GtkSignalFunc) file_ok_sel, filew );

    /* Connect the cancel_button to destroy the widget */
    gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION
                                            (filew)->cancel_button),
			       "clicked", (GtkSignalFunc) gtk_widget_destroy,
			       GTK_OBJECT (filew));


    gtk_widget_show(filew);

/*
    g_print("%d",gtk_file_selection_match_mask("mask.c","m*.c"));
    g_print("%d",gtk_file_selection_match_mask("mask.c","m???.c"));
		g_print("%d",gtk_file_selection_match_mask("mask.c","m??*.c"));
		g_print("%d",gtk_file_selection_match_mask("mask.cout","m*.c"));
		g_print("%d",gtk_file_selection_match_mask("mask.cout","m*.c???"));
		g_print("%d",gtk_file_selection_match_mask("mask.cout","m*.c*"));
		g_print("%d",gtk_file_selection_match_mask("mask.cout","n*.c???"));
		g_print("%d",gtk_file_selection_match_mask("mask.c","[mn]*"));
		g_print("%d",gtk_file_selection_match_mask("COPYING","*.xpm"));
*/	
    gtk_main ();

    return 0;
}
/* example-end */
#endif
