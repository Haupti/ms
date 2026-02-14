#include "preprocessor_token.hpp"
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {
struct Tokenizer {
  filesystem::path filename;
  string code;
  uint64_t pos;
  uint64_t len;
};

Tokenizer tok_build(const string &filename, string code) {
  Tokenizer t;
  t.filename = filename;
  t.code = std::move(code);
  t.pos = 0;
  t.len = t.code.size();
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
bool is_letter(char c) {
  return (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z');
}
bool is_operator_char(char c) {
  return c == '+' or c == '-' or c == '*' or c == '/' or c == '>' or c == '<' or
         c == '=' or c == '!';
}

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
  tok_adv(t);
  uint64_t start = t->pos;
  while (tok_peek(t) != '"') {
    tok_adv(t);
  }
  string value = t->code.substr(start, t->pos - start);
  tok_adv(t);
  return build_pptoken_string(get_location(t, start - 1),
                              create_interned_string(value));
}
PreprocessorToken tokenize_symbol(Tokenizer *t) {
  uint64_t start = t->pos;
  tok_adv(t);
  while (!tok_eof(t)) {
    char c = tok_peek(t);
    if (is_letter(c) or is_number(c) or c == '_') {
      tok_adv(t);
    } else {
      break;
    }
  }
  string value = t->code.substr(start, t->pos - start);
  return build_pptoken_symbol(get_location(t, start), create_symbol(value));
}

PreprocessorToken tokenize_operator(Tokenizer *t) {
  uint64_t start = t->pos;
  tok_adv(t);
  while (!tok_eof(t)) {
    char c = tok_peek(t);
    if (is_operator_char(c)) {
      tok_adv(t);
    } else {
      break;
    }
  }
  string value = t->code.substr(start, t->pos - start);
  if (value == "-") {
    return build_pptoken(get_location(t, start), PpTokenTag::OP_SUB);
  } else if (value == "+") {
    return build_pptoken(get_location(t, start), PpTokenTag::OP_ADD);
  } else if (value == "*") {
    return build_pptoken(get_location(t, start), PpTokenTag::OP_MUL);
  } else if (value == "/") {
    return build_pptoken(get_location(t, start), PpTokenTag::OP_DIV);
  } else if (value == ">") {
    return build_pptoken(get_location(t, start), PpTokenTag::OPERATOR_GT);
  } else if (value == ">=") {
    return build_pptoken(get_location(t, start), PpTokenTag::OPERATOR_GTE);
  } else if (value == "<") {
    return build_pptoken(get_location(t, start), PpTokenTag::OPERATOR_LT);
  } else if (value == "<=") {
    return build_pptoken(get_location(t, start), PpTokenTag::OPERATOR_LTE);
  } else if (value == "==") {
    return build_pptoken(get_location(t, start), PpTokenTag::OP_EQ);
  } else if (value == "!=") {
    return build_pptoken(get_location(t, start), PpTokenTag::OP_NEQ);
  } else if (value == "<>") {
    return build_pptoken(get_location(t, start),
                         PpTokenTag::OPERATOR_STRCONCAT);
  } else if (value == "=") {
    return build_pptoken(get_location(t, start), PpTokenTag::ASSIGN);
  } else {
    throw runtime_error("unknown operator '" + value + "'\n");
  }
}
PreprocessorToken tokenize_macro_or_subtraction_operator(Tokenizer *t) {
  uint64_t start = t->pos;
  tok_adv(t);
  while (!tok_eof(t)) {
    char c = tok_peek(t);
    if (is_letter(c)) {
      tok_adv(t);
    } else {
      break;
    }
  }
  string value = t->code.substr(start, t->pos - start);
  if (value == "-define") {
    return build_pptoken(get_location(t, start), PpTokenTag::DEFINE_MACRO);
  } else if (value == "-fset") {
    return build_pptoken(get_location(t, start), PpTokenTag::FSET_MACRO);
  } else if (value == "-funset") {
    return build_pptoken(get_location(t, start), PpTokenTag::FUNSET_MACRO);
  } else if (value == "-endif") {
    return build_pptoken(get_location(t, start), PpTokenTag::ENDIF_MACRO);
  } else if (value == "-include") {
    return build_pptoken(get_location(t, start), PpTokenTag::INCLUDE_MACRO);
  } else if (value == "-iffset") {
    return build_pptoken(get_location(t, start), PpTokenTag::IFFSET_MACRO);
  } else if (value == "-ifnfset") {
    return build_pptoken(get_location(t, start), PpTokenTag::IFNFSET_MACRO);
  } else if (value == "-") {
    return build_pptoken(get_location(t, start), PpTokenTag::OP_SUB);
  } else {
    throw runtime_error("unknown macro '" + value + "'\n");
  }
}
PreprocessorToken tokenize_identifier_and_others(Tokenizer *t) {
  uint64_t start = t->pos;
  tok_adv(t);
  while (!tok_eof(t)) {
    char c = tok_peek(t);
    if (is_letter(c) or is_number(c) or c == '_' or c == ':') {
      tok_adv(t);
    } else {
      break;
    }
  }
  string value = t->code.substr(start, t->pos - start);
  if (value == "function") {
    return build_pptoken(get_location(t, start), PpTokenTag::FUNCTION);
  } else if (value == "try") {
    return build_pptoken(get_location(t, start), PpTokenTag::TRY);
  } else if (value == "expect") {
    return build_pptoken(get_location(t, start), PpTokenTag::EXPECT);
  } else if (value == "not") {
    return build_pptoken(get_location(t, start), PpTokenTag::OP_NOT);
  } else if (value == "and") {
    return build_pptoken(get_location(t, start), PpTokenTag::OP_AND);
  } else if (value == "or") {
    return build_pptoken(get_location(t, start), PpTokenTag::OP_OR);
  } else if (value == "let") {
    return build_pptoken(get_location(t, start), PpTokenTag::LET);
  } else if (value == "set") {
    return build_pptoken(get_location(t, start), PpTokenTag::SET);
  } else if (value == "at") {
    return build_pptoken(get_location(t, start), PpTokenTag::AT);
  } else if (value == "if") {
    return build_pptoken(get_location(t, start), PpTokenTag::IF);
  } else if (value == "elif") {
    return build_pptoken(get_location(t, start), PpTokenTag::ELIF);
  } else if (value == "else") {
    return build_pptoken(get_location(t, start), PpTokenTag::ELSE);
  } else if (value == "mod") {
    return build_pptoken(get_location(t, start), PpTokenTag::OP_MOD);
  } else if (value == "return") {
    return build_pptoken(get_location(t, start), PpTokenTag::RETURN);
  } else if (value == "list") {
    return build_pptoken(get_location(t, start), PpTokenTag::LIST);
  } else {
    return build_pptoken_identifier(get_location(t, start),
                                    create_interned_string(value));
  }
}
} // namespace

std::vector<PreprocessorToken>
preprocessor_tokenize(const filesystem::path &filename,
                      const std::string &code) {
  Tokenizer t = tok_build(filename, code);

  vector<PreprocessorToken> tokens;
  while (!tok_eof(&t)) {
    char c = tok_peek(&t);
    if (is_number(c)) {
      tokens.push_back(tokenize_number(&t));
    } else if (c == '"') {
      tokens.push_back(tokenize_string(&t));
    } else if (c == '#') {
      tokens.push_back(tokenize_symbol(&t));
    } else if (c == '-') {
      tokens.push_back(tokenize_macro_or_subtraction_operator(&t));
    } else if (is_operator_char(c)) {
      tokens.push_back(tokenize_operator(&t));
    } else if (is_letter(c) or c == '_') {
      tokens.push_back(tokenize_identifier_and_others(&t));
    } else if (c == '.') {
      tokens.push_back(build_pptoken(get_location(&t, t.pos), PpTokenTag::DOT));
      tok_adv(&t);
    } else if (c == '|') {
      tokens.push_back(build_pptoken(get_location(&t, t.pos), PpTokenTag::BAR));
      tok_adv(&t);
    } else if (c == ',') {
      tokens.push_back(
          build_pptoken(get_location(&t, t.pos), PpTokenTag::COMMA));
      tok_adv(&t);
    } else if (c == '(') {
      tokens.push_back(
          build_pptoken(get_location(&t, t.pos), PpTokenTag::BROPEN));
      tok_adv(&t);
    } else if (c == ')') {
      tokens.push_back(
          build_pptoken(get_location(&t, t.pos), PpTokenTag::BRCLOSE));
      tok_adv(&t);
    } else if (c == '{') {
      tokens.push_back(
          build_pptoken(get_location(&t, t.pos), PpTokenTag::CURLOPEN));
      tok_adv(&t);
    } else if (c == '}') {
      tokens.push_back(
          build_pptoken(get_location(&t, t.pos), PpTokenTag::CURLCLOSE));
      tok_adv(&t);
    } else if (c == '\n' or c == '\r' or c == '\t' or c == ' ') {
      tok_adv(&t);
    } else {
      throw runtime_error("unexpected character '" + string(1, c) + "'\n");
    }
  }
  return tokens;
}
