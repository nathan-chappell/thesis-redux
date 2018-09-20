// get_call_graph.cc

#include <clang-c/Index.h>
#include <iostream>

using namespace std;

void fatal(const char *s) {
  cerr << s << endl;
  exit(-1);
}

ostream &operator<<(ostream &stream, const CXString &str) {
  stream << clang_getCString(str);
  clang_disposeString(str);
  return stream;
}

// My Code
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

CXChildVisitResult visit(CXCursor c, CXCursor parent,
                         CXClientData client_data) {
  CXCursorKind kind = clang_getCursorKind(c);
  bool indented = false;

  if (kind == CXCursor_Constructor || kind == CXCursor_ConversionFunction ||
      kind == CXCursor_CXXMethod || kind == CXCursor_Destructor ||
      kind == CXCursor_FunctionDecl || kind == CXCursor_FunctionTemplate) {
    cout << string(2 * indent, ' ');
    printFullName(c);
    cout << " " << clang_getCursorExtent(c) << endl;

    ++indent;
    indented = true;
  } else if (kind == CXCursor_CallExpr) {
    CXCursor ref = clang_getCursorReferenced(c);

    CXCursor generic = clang_getSpecializedCursorTemplate(ref);
    if (!clang_Cursor_isNull(generic))
      ref = generic;

    if (!spelling_is_null(ref)) {
      cout << string(2 * indent, ' ') << "calls ";
      printFullName(ref);
      cout << " " << clang_getCursorExtent(c) << endl;
    }
  }

  clang_visitChildren(c, visit, nullptr);

  if (indented)
    --indent;

  return CXChildVisit_Continue;
}

int main(int argc, char *argv[]) {
  if (argc != 2)
    fatal("usage: ./get_call_graph <file>");

  CXIndex index = clang_createIndex(0,  // excludeDeclarationsFromPCH
                                    0); // displayDiagnostics

  CXTranslationUnit unit = clang_parseTranslationUnit(
      index, argv[1], nullptr, 0, // command_line_args, num_command_line_args
      nullptr, 0,                 // unsaved_files, num_unsaved_files
      CXTranslationUnit_None);

  if (!unit)
    fatal("parse failed");

  CXCursor cursor = clang_getTranslationUnitCursor(unit);
  clang_visitChildren(cursor, visit, nullptr);

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);
}

