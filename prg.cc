void f() {}
void g() { f(); }
void h() { g(); f(); }

int main() { g(); f(); h(); }
