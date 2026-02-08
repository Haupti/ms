#include "preprocessor_token.hpp"
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

struct Tokenizer {
  string filename;
  string code;
  uint64_t pos;
  uint64_t len;
};

Tokenizer tok_build(const string &filename, const string &code) {
  Tokenizer t;
  t.filename = filename;
  t.code = code;
  t.pos = 0;
  t.len = code.size();
  return t;
}

bool tok_eof(Tokenizer *t) { return t->pos >= t->len; }
void tok_adv(Tokenizer *t) { t->pos += 1; }
char tok_peek(Tokenizer *t) {
  if (tok_eof(t)) {
    throw runtime_error("unexpected EOF");
  } else {
    return t->code.at(t->pos);
  }
}

bool is_number(char c) { return c >= '0' and c <= '9'; }
LocationRef get_location(Tokenizer *t, uint64_t start) {
  uint64_t row = 0;
  uint64_t col = 1;
  for (uint64_t i = 0; i < start + 1; i++) {
    if (t->code.at(i) == '\n') {
      row++;
      col = 1;
    } else {
      col++;
    }
  }
  return create_location(t->filename, row, col);
}

PreprocessorToken tokenize_number(Tokenizer *t) {
  uint64_t start = t->pos;
  bool has_dot = false;
  while (!tok_eof(t)) {
    char c = tok_peek(t);
    if (is_number(c)) {
      tok_adv(t);
    } else if (c == '.' && !has_dot) {
      has_dot = true;
      tok_adv(t);
    } else {
      break;
    }
  }
  string value = t->code.substr(start, t->pos - start);
  if (has_dot) {
    return build_pptoken_float(get_location(t, start), std::stod(value));
  } else {
    return build_pptoken_int(get_location(t, start), std::stoll(value));
  }
}
PreprocessorToken tokenize_string(Tokenizer *t) {
  uint64_t start = t->pos;
  tok_adv(t);
  while (tok_peek(t) != '"') {
    tok_adv(t);
  }
  string value = t->code.substr(
      start + 1,
      t->pos - start - 1); // TODO that might be wrong, check in tests
  return build_pptoken_string(get_location(t, start),
                              create_interned_string(value));
}

int main() {
  string filename = "main.msl";
  string code = "let x = 1 + 2";
  Tokenizer t = tok_build(filename, code);

  vector<PreprocessorToken> tokens;
  while (!tok_eof(&t)) {
    char c = tok_peek(&t);
    if (is_number(c)) {
      tokens.push_back(tokenize_number(&t));
      tok_adv(&t);
    } else if (c == '"') {
      tokens.push_back(tokenize_string(&t));
      tok_adv(&t);
    } else if (c == '\n' or c == '\r' or c == '\t' or c == ' ') {
      tok_adv(&t);
    } else {
      throw runtime_error("unexpected character '" + string(1, c) + "'\n");
    }
    // TODO symbol
    // TODO bool
    // TODO keywords
    // TODO ppmacros
    // TODO operators
    // TODO identifier (can include some special character other than _ so i can establish a module-identifier naming convention) e.g. mymod:somefun()
  }
}
