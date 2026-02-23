#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Return_Button.H>

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

int changed = 0;
char filename[256] = "";
Fl_Text_Buffer *textbuf = nullptr;
int loading = 0;
char title[256];

class EditorWindow: public Fl_Double_Window{
    public:
    EditorWindow(int w, int h, const char* t);
    ~EditorWindow();

    Fl_Input *replace_find;
    Fl_Input *replace_with;
    Fl_Button *replace_all;
    Fl_Button *replace_next;
    Fl_Button *replace_cancel;
    Fl_Text_Editor *editor;
    Fl_Window *replace_dlg;
    char search[256];

    Fl_Window *find_dlg;
    Fl_Input  *find_input;
    Fl_Button *find_next;
    Fl_Button *find_cancel;
};

void new_cb(Fl_Widget*, void*);
void open_cb(Fl_Widget*, void*);
void save_cb(Fl_Widget*, void*);
void saveas_cb(Fl_Widget*, void*);
void quit_cb(Fl_Widget*, void*);
void cut_cb(Fl_Widget*, void*);
void copy_cb(Fl_Widget*, void*);
void paste_cb(Fl_Widget*, void*);
void delete_cb(Fl_Widget*, void*);
void find_cb(Fl_Widget*, void*);
void find2_cb(Fl_Widget*, void*);
void replace_cb(Fl_Widget*, void*);
void replace2_cb(Fl_Widget*, void*);
void replace_all_cb(Fl_Widget*, void*);
void changed_cb(int, int, int, int, const char*, void*);
int check_save(void);
void load_file(char*, int);
void save_file(char*);
void set_title(Fl_Window*);

void insert_cb(Fl_Widget*, void*);
void view_cb(Fl_Widget*, void*);
void close_cb(Fl_Widget*, void*);
void undo_cb(Fl_Widget*, void*);

// label, shortcut, callback, userdata, flags

EditorWindow::EditorWindow(int w, int h, const char* t): Fl_Double_Window(w, h, t){
    Fl_Menu_Bar *m = new Fl_Menu_Bar(0, 0, w, 30);
    
    m->add("&File/&New File", 0, new_cb, this, FL_MENU_DIVIDER);
    m->add("&File/&Open File...", FL_CTRL + 'o', open_cb, this);
    m->add("&File/&Insert File...", FL_CTRL + 'i', insert_cb, this, FL_MENU_DIVIDER);
    m->add("&File/&Save File", FL_CTRL + 's', save_cb, this);
    m->add("&File/Save File &As...", FL_CTRL + FL_SHIFT + 's', saveas_cb, this, FL_MENU_DIVIDER);
    m->add("&File/New &View", FL_ALT + 'v', view_cb, this);
    m->add("&File/&Close View", FL_CTRL + 'w', close_cb, this, FL_MENU_DIVIDER);
    m->add("&File/E&xit", FL_CTRL + 'q', quit_cb, this);

    m->add("&Edit/&Undo", FL_CTRL + 'z', undo_cb, this, FL_MENU_DIVIDER);
    m->add("&Edit/Cu&t", FL_CTRL + 'x', cut_cb, this);
    m->add("&Edit/&Copy", FL_CTRL + 'c', copy_cb, this);
    m->add("&Edit/&Paste", FL_CTRL + 'v', paste_cb, this);
    m->add("&Edit/&Delete", 0, delete_cb, this);

    m->add("&Search/&Find...", FL_CTRL + 'f', find_cb, this);
    m->add("&Search/&Replace...", FL_CTRL + 'r', replace_cb, this);

    editor = new Fl_Text_Editor(0, 30, w, h-30);
    editor->buffer(textbuf);
    editor->textfont(FL_COURIER);

    textbuf->add_modify_callback(changed_cb, this);
    textbuf->call_modify_callbacks();    

    // replace dialog
    replace_dlg = new Fl_Window(300, 105, "Replace");
    replace_find = new Fl_Input(70, 10, 200, 25, "Find:");
    replace_with = new Fl_Input(70, 40, 200, 25, "Replace:");
    replace_all = new Fl_Button(10, 70, 90, 25, "Replace All");
    replace_next = new Fl_Button(105, 70, 120, 25, "Replace Next");
    replace_cancel = new Fl_Button(230, 70, 60, 25, "Cancel");

    replace_next->callback(replace2_cb, this);
    replace_all->callback(replace_all_cb, this);
    replace_cancel->callback([](Fl_Widget*, void* v) {
        ((EditorWindow*)v)->replace_dlg->hide();
    }, this);

    // find dialog
    find_dlg = new Fl_Window(300, 100, "Find");
    find_input = new Fl_Input(70, 10, 200, 25, "Find:");
    find_next = new Fl_Button(70, 50, 100, 25, "Find Next");
    find_cancel = new Fl_Button(180, 50, 80, 25, "Cancel");

    find_next->callback(find2_cb, this);
    find_cancel->callback([](Fl_Widget*, void* v) {
        ((EditorWindow*)v)->find_dlg->hide();
    }, this);

    find_dlg->end();

    replace_dlg->end();

    end();
}

EditorWindow::~EditorWindow() {}

int main(int argc, char **argv)
{
    textbuf = new Fl_Text_Buffer;
    EditorWindow *win = new EditorWindow(640, 400, "AsaText Varieditor");
    win->show(argc, argv);
     return Fl::run();
}

void changed_cb(int, int nInserted, int nDeleted,int, const char*, void* v) {
  if ((nInserted || nDeleted) && !loading) changed = 1;
  EditorWindow *w = (EditorWindow *)v;
  set_title(w);
  if (loading) w->editor->show_insert_position();
}

void copy_cb(Fl_Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  Fl_Text_Editor::kf_copy(0, e->editor);
}

void cut_cb(Fl_Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  Fl_Text_Editor::kf_cut(0, e->editor);
}

void delete_cb(Fl_Widget*, void* v) {
  textbuf->remove_selection();
}

void find_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;
    e->find_dlg->show();
}

void find2_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;

    const char* search = e->find_input->value();
    if (!search || search[0] == '\0') {
        fl_alert("Enter text to find.");
        return;
    }

    int start = e->editor->insert_position();
    int pos;

    int found = textbuf->search_forward(start, search, &pos);

    if (!found)
        found = textbuf->search_forward(0, search, &pos);

    if (found) {
        textbuf->select(pos, pos + strlen(search));
        e->editor->insert_position(pos + strlen(search));
        e->editor->show_insert_position();
    } else {
        fl_alert("No occurrences found.");
    }
}

void new_cb(Fl_Widget*, void*) {
  if (!check_save()) return;

  filename[0] = '\0';
  textbuf->select(0, textbuf->length());
  textbuf->remove_selection();
  changed = 0;
  textbuf->call_modify_callbacks();
}

void open_cb(Fl_Widget*, void*) {
  if (!check_save()) return;

  char *newfile = fl_file_chooser("Open File?", "*", filename);
  if (newfile != NULL) load_file(newfile, -1);
}

void paste_cb(Fl_Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  Fl_Text_Editor::kf_paste(0, e->editor);
}

void quit_cb(Fl_Widget*, void*) {
  if (changed && !check_save())
    return;

  exit(0);
}

void replace_cb(Fl_Widget*, void* v) {
  EditorWindow* e = (EditorWindow*)v;
  e->replace_dlg->show();
}

void replace2_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;

    const char* find = e->replace_find->value();
    const char* replace = e->replace_with->value();

    if (!find || find[0] == '\0') {
        fl_alert("Enter text to find.");
        return;
    }

    int start = e->editor->insert_position();
    int pos;

    int found = textbuf->search_forward(start, find, &pos);

    if (found) {
        textbuf->select(pos, pos + strlen(find));
        textbuf->remove_selection();
        textbuf->insert(pos, replace);

        e->editor->insert_position(pos + strlen(replace));
        e->editor->show_insert_position();
    } else {
        fl_alert("No occurrences found.");
    }
}

void replace_all_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;

    const char* find = e->replace_find->value();
    const char* replace = e->replace_with->value();

    if (!find || find[0] == '\0') {
        fl_alert("Enter text to find.");
        return;
    }

    int pos = 0;
    int count = 0;

    while (textbuf->search_forward(pos, find, &pos)) {
        textbuf->select(pos, pos + strlen(find));
        textbuf->remove_selection();
        textbuf->insert(pos, replace);

        pos += strlen(replace);
        count++;
    }

    fl_message("Replaced %d occurrences.", count);
}

void save_cb(Fl_Widget*, void*) {
  if (filename[0] == '\0') {
    saveas_cb(nullptr, nullptr);
    return;
  }
  else save_file(filename);
}

void saveas_cb(Fl_Widget*, void*) {
  char *newfile;

  newfile = fl_file_chooser("Save File As?", "*", filename);
  if (newfile != NULL) save_file(newfile);
}

int check_save(void) {
  if (!changed) return 1;

  int r = fl_choice("The current file has not been saved.\n"
                    "Would you like to save it now?",
                    "Cancel", "Save", "Discard");

  if (r == 1) {
    save_cb(nullptr, nullptr);
    return !changed;
  }

  return (r == 2) ? 1 : 0;
}

void load_file(char *newfile, int ipos) {
  loading = 1;
  int insert = (ipos != -1);
  changed = insert;
  if (!insert) strcpy(filename, "");
  int r;
  if (!insert) r = textbuf->loadfile(newfile);
  else r = textbuf->insertfile(newfile, ipos);
  if (r)
    fl_alert("Error reading from file \'%s\':\n%s.", newfile, strerror(errno));
  else
    if (!insert) strcpy(filename, newfile);
  loading = 0;
  textbuf->call_modify_callbacks();
}

void save_file(char *newfile) {
  if (textbuf->savefile(newfile))
    fl_alert("Error writing to file \'%s\':\n%s.", newfile, strerror(errno));
  else
    strcpy(filename, newfile);
  changed = 0;
  textbuf->call_modify_callbacks();
}

void set_title(Fl_Window* w) {
  if (filename[0] == '\0') strcpy(title, "Untitled");
  else {
    char *slash;
    slash = strrchr(filename, '/');
#ifdef WIN32
    if (slash == NULL) slash = strrchr(filename, '\\');
#endif
    if (slash != NULL) strcpy(title, slash + 1);
    else strcpy(title, filename);
  }

  if (changed) strcat(title, "*");

  w->label(title);
}

void insert_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;

    printf("v = %p\n", v);

    char *newfile = fl_file_chooser("Insert File?", "*", filename);
    if (newfile != nullptr) {
        int pos = e->editor->insert_position();
        load_file(newfile, pos);
    }
}

void view_cb(Fl_Widget*, void*) {
    EditorWindow *new_window = new EditorWindow(640, 400, "Additional View");
    new_window->show();
}

void close_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;

    if (changed && !check_save())
        return;

    e->hide();
    delete e;
}

void undo_cb(Fl_Widget*, void* v) {
    EditorWindow* e = (EditorWindow*)v;

    printf("v = %p\n", v);
    
    Fl_Text_Editor::kf_undo(0, e->editor);
}