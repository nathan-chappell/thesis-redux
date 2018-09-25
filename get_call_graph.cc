// get_call_graph.cc

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

void fatal(const char *s) {
  cerr << s << endl;
  exit(-1);
}

ostream &operator<<(ostream &stream, const CXString &str) {
  if (!str.data) {
    return stream;
  }
  stream << clang_getCString(str);
  clang_disposeString(str);
  return stream;
}

ostream &operator<<(ostream &o, CXCompileCommand cmd) {
  for (unsigned i = 0; i < clang_CompileCommand_getNumArgs(cmd); ++i) {
    o << clang_CompileCommand_getArg(cmd, i) << " ";
  }
  return o;
}

vector<string> get_compile_command_str(CXCompileCommand command) {
  vector<string> result;
  ostringstream oss;
  for (unsigned i = 0; i < clang_CompileCommand_getNumArgs(command); ++i) {
    oss << clang_CompileCommand_getArg(command, i);
    result.push_back(move(oss.str()));
    oss.str("");
  }
  return result;
}

vector<vector<string>> get_compile_commands_str(CXCompileCommands commands) {
  vector<vector<string>> result;
  for (unsigned i = 0; i < clang_CompileCommands_getSize(commands); ++i) {
    result.push_back(move(get_compile_command_str(
        clang_CompileCommands_getCommand(commands, i))));
  }
  return result;
}

vector<vector<string>> get_compile_commands_directory(const string directory) {
  CXCompilationDatabase_Error error;
  CXCompilationDatabase db =
      clang_CompilationDatabase_fromDirectory(directory.c_str(), &error);
  if (error != CXCompilationDatabase_NoError) {
    cerr << "couldn't get a database from the directory!" << endl;
    return vector<vector<string>>();
  }
  CXCompileCommands commands =
      clang_CompilationDatabase_getAllCompileCommands(db);
  auto result = move(get_compile_commands_str(commands));
  clang_CompileCommands_dispose(commands);
  clang_CompilationDatabase_dispose(db);
  return result;
}

ostream &operator<<(ostream &stream, const CXSourceLocation location) {
  CXFile file;
  unsigned line;
  unsigned column;
  unsigned offset;
  clang_getSpellingLocation(location, &file, &line, &column, &offset);
  stream << clang_getFileName(file) << " " << line << " " << column << " "
         << offset;
  return stream;
}

ostream &operator<<(ostream &stream, const CXSourceRange range) {
  cout << clang_getRangeStart(range) << " "
       << " " << clang_getRangeEnd(range);
  return stream;
}

void printFullName(CXCursor c) {
  CXCursor parent = clang_getCursorSemanticParent(c);
  if (clang_getCursorKind(parent) != CXCursor_TranslationUnit &&
      !clang_Cursor_isNull(parent)) {
    printFullName(parent);
    cout << "::";
  }
  cout << clang_getCursorDisplayName(c);
}

int indent = 0;

bool spelling_is_null(CXCursor c) {
  bool result = false;
  CXString cx = clang_getCursorSpelling(c);
  result = clang_getCString(cx) == string("");
  clang_disposeString(cx);
  return result;
}

bool in_system_header(CXCursor c) {
  return clang_Location_isInSystemHeader(clang_getCursorLocation(c));
}

/*
 * TODO: fix the hack below.  I found out that not all CallExpr display names
 * have parenthesis.  I was previously using a ')' to mark the end of a function
 * call while parsing, but that does not suffice.  What does suffice (at least
 * for now) is to use '$' to delimit a function name.  This may be fragile, but
 * it works alright for the time being.
 */
CXChildVisitResult visit(CXCursor c, CXCursor parent,
                         CXClientData client_data) {
  CXCursorKind kind = clang_getCursorKind(c);
  bool indented = false;

  if ((kind == CXCursor_Constructor || kind == CXCursor_ConversionFunction ||
       kind == CXCursor_CXXMethod || kind == CXCursor_Destructor ||
       kind == CXCursor_FunctionDecl || kind == CXCursor_FunctionTemplate) &&
      //!in_system_header(c)) {
      clang_Location_isFromMainFile(clang_getCursorLocation(c))) {
    cout << string(2 * indent, ' ');
    cout << "$";
    printFullName(c);
    cout << "$";
    cout << " " << clang_getCursorExtent(c) << endl;

    ++indent;
    indented = true;
  } else if (kind == CXCursor_CallExpr && !in_system_header(c)) {
    CXCursor ref = clang_getCursorReferenced(c);

    CXCursor generic = clang_getSpecializedCursorTemplate(ref);
    if (!clang_Cursor_isNull(generic))
      ref = generic;

    if (!spelling_is_null(ref)) {
      cout << string(2 * indent, ' ') << "calls ";
      cout << "$";
      printFullName(ref);
      cout << "$";
      cout << " " << clang_getCursorExtent(c) << endl;
    }
  }

  clang_visitChildren(c, visit, nullptr);

  if (indented)
    --indent;

  return CXChildVisit_Continue;
}

int main(int argc, char *argv[]) {
  if (argc > 2) {
    fatal("usage: ./get_call_graph <directory containing "
          "compile_commands.json>\n\tdefaults to the current directory\n\r");
  }
  string directory = ".";
  if (argc == 2) {
    directory = argv[1];
  }

  CXIndex index = clang_createIndex(0,  // excludeDeclarationsFromPCH
                                    0); // displayDiagnostics

  auto compile_commands = get_compile_commands_directory(directory);

  for (auto &&command : compile_commands) {
    vector<const char *> c_str_command;
    for (auto &&str : command) {
      c_str_command.push_back(str.data());
    }
    CXTranslationUnit unit = clang_parseTranslationUnit(
        index, nullptr, c_str_command.data(),
        c_str_command.size(), // command_line_args, num_command_line_args
        nullptr, 0,           // unsaved_files, num_unsaved_files
        CXTranslationUnit_None);

    if (!unit) {
      cerr << "CXTranslationUnit: " << clang_getTranslationUnitSpelling(unit)
           << endl;
      fatal("parse failed");
    }

    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(cursor, visit, nullptr);

    clang_disposeTranslationUnit(unit);
  }
  clang_disposeIndex(index);
}
