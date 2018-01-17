/*
    This file is part of darktable,
    copyright (c) 2018 Peter Budai.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    darktable is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with darktable.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <sqlite3.h>
#include "../common/file_location.h"

// local variables
GtkBuilder  *builder;
GtkWidget   *window;
sqlite3     *g_datadbhandle;
char        *g_progname;

typedef struct dt_library_t
{
  gboolean  is_default;
  guint     id;
  gchar    *name;
  gchar    *description;
  gchar    *path;
}
dt_library_t;

enum
{
  COLUMN_IS_DEFAULT,
  COLUMN_NAME,
  COLUMN_DESCRIPTION,
  COLUMN_PATH,
  COLUMN_ID,
  NUM_COLUMNS
};

// forward declarations
G_MODULE_EXPORT void on_new_library_activate(GSimpleAction *simple, GVariant *parameter, gpointer user_data);
G_MODULE_EXPORT void on_move_library_activate(GSimpleAction *simple, GVariant *parameter, gpointer user_data);
G_MODULE_EXPORT void on_delete_library_activate(GSimpleAction *simple, GVariant *parameter, gpointer user_data);
G_MODULE_EXPORT void on_launch_dt_activate(GSimpleAction *simple, GVariant *parameter, gpointer user_data);

dt_library_t *get_selected_library();
void free_library(dt_library_t *library);
int get_selected_library_id();
gchar * get_selected_library_path();
void open_database();
void fill_libaries();

// local actions
GSimpleActionGroup *action_group_main;
const GActionEntry action_entries[] = { { "new_library", on_new_library_activate },
                                        { "move_library", on_move_library_activate },
                                        { "delete_library", on_delete_library_activate },
                                        { "launch_dt", on_launch_dt_activate } };

G_MODULE_EXPORT gint on_main_delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{

  g_print("delete event occurred\n");
  return (FALSE);
}

G_MODULE_EXPORT void on_main_destroy(GtkWidget *widget, gpointer data)
{
  g_print("main destroy event occurred\n");

  /* Destroy builder, since we don't need it anymore */
  g_object_unref(G_OBJECT(builder));

  gtk_main_quit();
}

G_MODULE_EXPORT void on_btnNew_clicked(GtkButton *widget, gpointer data)
{
  g_print("on_btnNew_clicked\n");

  GtkWidget *dialog;
  gint response;
  dt_library_t *library;
  library = (dt_library_t *)calloc(1, sizeof(dt_library_t));

  dialog = GTK_WIDGET(gtk_builder_get_object(builder, "dialogLibrary"));

  // Create a new entry
  library->name = "";
  library->description = "";
  library->path = "[NOT SET]";
  library->is_default = FALSE;
  library->id = -1;

  gtk_entry_set_text (GTK_ENTRY (gtk_builder_get_object(builder, "txtName")), library->name);
  gtk_entry_set_text (GTK_ENTRY (gtk_builder_get_object(builder, "txtDescription")), library->description);
  gtk_label_set_text (GTK_LABEL (gtk_builder_get_object(builder, "lblPath")), library->path);

  //Show the dialog
  response = gtk_dialog_run (GTK_DIALOG (dialog));

  if (response == GTK_RESPONSE_OK)
    {
      library->name = (gchar*) gtk_entry_get_text (GTK_ENTRY (gtk_builder_get_object(builder, "txtName")));
      library->description = (gchar*) gtk_entry_get_text (GTK_ENTRY (gtk_builder_get_object(builder, "txtDescription")));
      library->path = (gchar*) gtk_label_get_text (GTK_LABEL (gtk_builder_get_object(builder, "lblPath")));
    }

  gtk_widget_hide(dialog);
}

G_MODULE_EXPORT void on_btnEdit_clicked(GtkButton *widget, gpointer data)
{
  g_print("on_btnEdit_clicked\n");

}

G_MODULE_EXPORT void on_btnDelete_clicked(GtkButton *widget, gpointer data)
{
  g_print("on_btnDelete_clicked\n");
}

G_MODULE_EXPORT void on_btnLaunch_clicked(GtkButton *widget, gpointer data)
{
  g_print("on_btnLaunch_clicked\n");

  GtkWidget *dialog;
  GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT;

  // Launch dt using that library
  gchar * library_path = get_selected_library_path();

  // Check the existance of directory before launching
  /*
  if(!g_file_test(library_path, G_FILE_TEST_IS_DIR))
  {
    // directory  does not exist
    dialog = gtk_message_dialog_new (GTK_WINDOW(gtk_builder_get_object(builder, "main")),
                                    flags,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE,
                                    "Library file does not exist!");
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    goto bail_out;
  }
*/

  if(library_path == NULL)
  {
    // No libaray has been selected - notify the user and return
    dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_builder_get_object(builder, "main")),
                                    flags,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE,
                                    "Please select a library first!");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    goto bail_out;
  }

  char *argv[] = { "", "--library", "", NULL };
  char *output = NULL; // will contain command output
  GError *error = NULL;
  int exit_status = 0;

#if defined _WIN32
  argv[0] = g_build_filename(dt_loc_find_install_dir("bin", g_progname), "darktable.exe", NULL);
#else
  //TODO: Needs to be fixed
  argv[0] = g_build_filename(dt_loc_find_install_dir(), "darktable", NULL);
#endif
  argv[2] = g_strdup(library_path);

  // Check the existance of darktable executable before launching
  if(!g_file_test(argv[0], G_FILE_TEST_EXISTS))
  {
    // darktable executable does not exists?!
    dialog = gtk_message_dialog_new (GTK_WINDOW(gtk_builder_get_object(builder, "main")),
                                    flags,
                                    GTK_MESSAGE_ERROR,
                                    GTK_BUTTONS_CLOSE,
                                    "Cannot find darktable executable!");
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
    goto bail_out;
  }

  // Start darktable with the given library as a parameter
  if(!g_spawn_sync(NULL, argv, NULL, 0, NULL, NULL, &output, NULL, &exit_status, &error))
  {
    // handle error here
  }

bail_out:
  g_free(argv[0]);
  g_free(argv[2]);
  g_free(library_path);
}


G_MODULE_EXPORT void on_btnClose_clicked(GtkButton *widget, gpointer data)
{
  g_print("on_btnClose_clicked\n");
  gtk_widget_destroy(window);
}

G_MODULE_EXPORT void on_new_library_activate(GSimpleAction *simple, GVariant *parameter, gpointer user_data)
{
  g_print("on_new_library_activate\n");

}

G_MODULE_EXPORT void on_move_library_activate(GSimpleAction *simple, GVariant *parameter, gpointer user_data)
{
  g_print("on_move_library_activate\n");

  // Make sure there is a selected library

  // Show properties window, only location enabled
}

G_MODULE_EXPORT void on_delete_library_activate(GSimpleAction *simple, GVariant *parameter, gpointer user_data)
{
  g_print("on_delete_library_activate\n");

  // Make sure there is a selected library

  // Ask a user for confirmation

  //delete the selected library
}

G_MODULE_EXPORT void on_launch_dt_activate(GSimpleAction *simple, GVariant *parameter, gpointer user_data)
{
  g_print("on_launch_dt_activate\n");

}

G_MODULE_EXPORT void on_crIsDefault_toggled (GtkCellRendererToggle  *cell,
                                            gchar                   *path_str,
                                            gpointer                data)
{
  GtkTreeModel  *model = (GtkTreeModel*)data;
  GtkTreePath   *path = gtk_tree_path_new_from_string (path_str);
  GtkTreeIter   iter;
  gboolean      is_default;

  // get toggled iter
  gtk_tree_model_get_iter (model, &iter, path);
  gtk_tree_path_free (path);

  gtk_tree_model_get (model, &iter, COLUMN_IS_DEFAULT, &is_default, -1);

  // check the existing flag
  if(is_default)
  {
    // if user has clicked on a library which has been the default, don't change it
    return;
  }
  else{
    //Clear the existing default library

    // Then flip the selected to default
    is_default ^= 1;
  }

  /* show the new values */
  fill_libaries();
}

void open_database()
{
  char configdir[PATH_MAX] = { 0 };
  gchar * datadbfilename = NULL;
  g_datadbhandle = NULL;

  dt_loc_get_user_config_dir(configdir, sizeof(configdir));
  datadbfilename = g_build_filename(configdir, "data.db", NULL);

  // TODO: make sure no one is locking the database

  // Show the user which is the config directory where data.db is located
  gchar * lblConfig = g_strdup_printf("Config directory: %s", configdir);
  gtk_label_set_text(GTK_LABEL(gtk_builder_get_object (builder, "lblConfigDirectory")), lblConfig);
  g_free(lblConfig);

  // set the threading mode to Serialized
  sqlite3_config(SQLITE_CONFIG_SERIALIZED);

  // opening / creating database
  if(sqlite3_open(datadbfilename, &g_datadbhandle))
  {
    // TODO: handle if  database cannot be opened
  }

  // some sqlite3 config
  sqlite3_exec(g_datadbhandle, "PRAGMA synchronous = OFF", NULL, NULL, NULL);
  sqlite3_exec(g_datadbhandle, "PRAGMA journal_mode = MEMORY", NULL, NULL, NULL);
  sqlite3_exec(g_datadbhandle, "PRAGMA page_size = 32768", NULL, NULL, NULL);

  // Get current datbase schema version
  int rc = 0;
  sqlite3_stmt *stmt;

  rc = sqlite3_prepare_v2(g_datadbhandle, "select value from db_info where key = 'version'", -1, &stmt, NULL);
  if(rc == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW)
  {
    // compare the version of the db with what is current for this executable
    const int db_version = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if(db_version < 2)
    {
      // Upgrade database schema
      sqlite3_exec(g_datadbhandle, "BEGIN TRANSACTION", NULL, NULL, NULL);
      sqlite3_exec(g_datadbhandle,
                   "CREATE TABLE IF NOT EXISTS libraries ("
                   "library_id  INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "name        VARCHAR NOT NULL,"
                   "description VARCHAR,"
                   "path        VARCHAR NOT NULL,"
                   "is_default  BOOLEAN NOT NULL"
                   "               DEFAULT FALSE);",
                   NULL, NULL, NULL);
      sqlite3_exec(g_datadbhandle, "COMMIT", NULL, NULL, NULL);

      // write the new version to db
      sqlite3_prepare_v2(g_datadbhandle,
                         "INSERT OR REPLACE INTO db_info (key, value)"
                         "VALUES ('version', ?1)",
                         -1, &stmt, NULL);
      // TODO change it to 2!!!
      sqlite3_bind_int(stmt, 1, 1);
      sqlite3_step(stmt);
      sqlite3_finalize(stmt);
    }
  }

  g_free(datadbfilename);
}

// Get information from the ListStore for the selected row
dt_library_t *get_selected_library()
{
  GtkTreeView       *tv;
  GtkTreeModel      *model;
  GtkTreeSelection  *selected;
  GtkTreeIter       iter;
  dt_library_t      *library = NULL;

  library = (dt_library_t *)calloc(1, sizeof(dt_library_t));

  tv = GTK_TREE_VIEW(gtk_builder_get_object(builder, "tvLibraries"));
  g_return_if_fail(tv);

  selected = gtk_tree_view_get_selection(tv);
  if(gtk_tree_selection_count_selected_rows(selected) !=1)
    return library;

  gtk_tree_selection_get_selected (selected, &model, &iter);
  gtk_tree_model_get (model, &iter,
                        COLUMN_ID, &library->id,
                        COLUMN_NAME, &library->name,
                        COLUMN_DESCRIPTION, &library->description,
                        COLUMN_PATH, &library->path,
                        COLUMN_IS_DEFAULT, &library->is_default,
                        -1);

  return library;
}

// Clean up the structure
void free_library(dt_library_t *library)
{
  if(library->name) g_free(library->name);
  if(library->description) g_free(library->description);
  if(library->path) g_free(library->path);
  if(library) free(library);
}

// Returns the ID of the selected library, 0 if nothing has been selected
int get_selected_library_id()
{
  int               library_id = 0;
  dt_library_t      *library = NULL;

  library = get_selected_library();
  if(library)
    library_id = library->id;

  free_library(library);

  return library_id;
}

// Returns the path for the selected library, NULL if nothing has been selected
gchar * get_selected_library_path()
{
  gchar             *library_path = NULL;
  dt_library_t      *library = NULL;

  library = get_selected_library();
  if(library)
    library_path = g_strdup(library->path);

  free_library(library);

  return library_path;
}

// Fill the listbox with the existing libraries
void fill_libaries()
{

  g_return_if_fail(g_datadbhandle);

  GtkListStore *store = NULL;
  GtkTreeIter iter;
  sqlite3_stmt *stmt;

  store = GTK_LIST_STORE(gtk_builder_get_object(builder, "listLibraries"));

  // Reset list
  gtk_list_store_clear (store);

  // TODO: Check whether the table is empty
  // In that case the config file should be read to figure out the default library name
  // And use the config dir to set up the default library

  // Read all the existing known libraries from the data.db
  sqlite3_prepare_v2(g_datadbhandle, "SELECT name, description, is_default, path, library_id FROM libraries ORDER BY name", -1, &stmt, NULL);
  while(sqlite3_step(stmt) == SQLITE_ROW)
  {
    // Make sure there is no conflict betwen default database in the config file and in data.db

    // Add one line for each known library
    // Each line is one entry in the listbox
    gtk_list_store_insert_with_values (store, &iter, -1,
                        COLUMN_IS_DEFAULT, !g_strcmp0((const char *)sqlite3_column_text(stmt, 2), "TRUE"),
                        COLUMN_NAME, sqlite3_column_text(stmt, 0),
                        COLUMN_DESCRIPTION, sqlite3_column_text(stmt, 1),
                        COLUMN_PATH, sqlite3_column_text(stmt, 3),
                        COLUMN_ID, sqlite3_column_int(stmt, 4),
                        -1);
  }

  sqlite3_finalize(stmt);
}

void wnd_main_init(char *progname)
{
  GError *error = NULL;

  char datadir[PATH_MAX] = { 0 };
  g_progname = progname;
  dt_loc_init_user_config_dir(NULL);
  dt_loc_init_datadir(NULL);
  dt_loc_get_datadir(datadir, sizeof(datadir));

  // load the style / theme
  GtkSettings *settings = gtk_settings_get_default();
  g_object_set(G_OBJECT(settings), "gtk-application-prefer-dark-theme", TRUE, (gchar *)0);
  g_object_set(G_OBJECT(settings), "gtk-theme-name", "Adwaita", (gchar *)0);
  g_object_unref(settings);

  // Create new GtkBuilder object
  builder = gtk_builder_new();

  // Load UI from file. If error occurs, report it and quit application.
  if(!gtk_builder_add_from_file(builder, g_build_filename(datadir, "librarymgmt.glade", NULL), &error))
  {
    g_warning("%s", error->message);
    g_free(error);
    return;
  }

  // Get main window pointer from UI
  window = GTK_WIDGET(gtk_builder_get_object(builder, "main"));

  // Set up actions
  action_group_main = g_simple_action_group_new();
  g_action_map_add_action_entries(G_ACTION_MAP(action_group_main), action_entries, G_N_ELEMENTS(action_entries),
                                  NULL);
  gtk_widget_insert_action_group(window, "librarymgmt", G_ACTION_GROUP(action_group_main));

  // Connect signals
  gtk_builder_connect_signals(builder, NULL);

  // Connect actions to buttons
  /*
  gtk_actionable_set_action_name(GTK_ACTIONABLE(gtk_builder_get_object (builder, "btnNew")), "librarymgmt.new_library");
  gtk_actionable_set_action_name(GTK_ACTIONABLE(gtk_builder_get_object (builder, "btnMove")), "librarymgmt.move_library");
  gtk_actionable_set_action_name(GTK_ACTIONABLE(gtk_builder_get_object (builder, "btnDelete")), "librarymgmt.delete_library");
  gtk_actionable_set_action_name(GTK_ACTIONABLE(gtk_builder_get_object (builder, "btnLaunch")), "librarymgmt.launch_dt");
*/

  // Open data.db
  open_database();
  
  // List all known libraries
  fill_libaries();

  // Show main window
  gtk_widget_show(window);

  return;
}

// modelines: These editor modelines have been set for all relevant files by tools/update_modelines.sh
// vim: shiftwidth=2 expandtab tabstop=2 cindent
// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-spaces modified;
