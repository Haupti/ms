#include "../compile_error.hpp"
#include "../filename.hpp"
#include "../sourcecode.hpp"
#include "preprocessor_token.hpp"
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

namespace {
struct Tokenizer {
  Filename filename;
  string code;
  uint64_t pos;
  uint64_t len;
};

Tokenizer tok_build(Filename filename, string code) {
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
char tok_has_next(Tokenizer *t) { return t->pos + 1 < t->len; }
char tok_peek_next(Tokenizer *t) {
  if (tok_eof(t) || t->pos + 1 >= t->len) {
    throw runtime_error("unexpected EOF");
  } else {
    return t->code.at(t->pos + 1);
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

inline LocationRef get_location(Tokenizer *t, uint64_t start) {
  return create_location(t->filename, start);
}

PreprocessorToken tokenize_number(Tokenizer *t) {
  uint64_t start = t->pos;
  bool has_dot = false;
  while (!tok_eof(t)) {
    char c = tok_peek(t);
    if (is_number(c)) {
      tok_adv(t);
    } else if (c == '.' && !has_dot && tok_has_next(t) &&
               is_number(tok_peek_next(t))) {
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
  string value;
  while (!tok_eof(t) && tok_peek(t) != '"') {
    char c = tok_peek(t);
    if (c == '\\') {
      tok_adv(t);
      if (tok_eof(t)) {
        throw compile_error(get_location(t, start), "unclosed string literal");
      }
      char next = tok_peek(t);
      if (next == 'n') {
        value += '\n';
      } else if (next == 't') {
        value += '\t';
      } else if (next == 'r') {
        value += '\r';
      } else if (next == '"') {
        value += '"';
      } else if (next == '\\') {
        value += '\\';
      } else {
        throw compile_error(get_location(t, t->pos - 1),
                            "invalid escape sequence '\\" + string(1, next) +
                                "'");
      }
      tok_adv(t);
    } else {
      value += c;
      tok_adv(t);
    }
  }
  if (tok_eof(t)) {
    throw compile_error(get_location(t, start), "unclosed string literal");
  }
  tok_adv(t);
  return build_pptoken_string(get_location(t, start),
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
    throw compile_error(get_location(t, start),
                        "unknown operator '" + value + "'\n");
  }
}
PreprocessorToken tokenize_macro(Tokenizer *t) {
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
  if (value == "$define") {
    return build_pptoken(get_location(t, start), PpTokenTag::DEFINE_MACRO);
  } else if (value == "$fset") {
    return build_pptoken(get_location(t, start), PpTokenTag::FSET_MACRO);
  } else if (value == "$funset") {
    return build_pptoken(get_location(t, start), PpTokenTag::FUNSET_MACRO);
  } else if (value == "$endif") {
    return build_pptoken(get_location(t, start), PpTokenTag::ENDIF_MACRO);
  } else if (value == "$include") {
    return build_pptoken(get_location(t, start), PpTokenTag::INCLUDE_MACRO);
  } else if (value == "$iffset") {
    return build_pptoken(get_location(t, start), PpTokenTag::IFFSET_MACRO);
  } else if (value == "$ifnfset") {
    return build_pptoken(get_location(t, start), PpTokenTag::IFNFSET_MACRO);
  } else {
    throw compile_error(get_location(t, start),
                        "unknown macro '" + value + "'\n");
  }
}
PreprocessorToken tokenize_subtraction_operator(Tokenizer *t) {
  uint64_t start = t->pos;
  tok_adv(t);
  return build_pptoken(get_location(t, start), PpTokenTag::OP_SUB);
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
  } else if (value == "ref") {
    return build_pptoken(get_location(t, start), PpTokenTag::REF);
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
  } else if (value == "for") {
    return build_pptoken(get_location(t, start), PpTokenTag::FOR);
  } else if (value == "in") {
    return build_pptoken(get_location(t, start), PpTokenTag::IN);
  } else if (value == "none") {
    return build_pptoken(get_location(t, start), PpTokenTag::NONE);
  } else if (value == "continue") {
    return build_pptoken(get_location(t, start), PpTokenTag::CONTINUE);
  } else if (value == "break") {
    return build_pptoken(get_location(t, start), PpTokenTag::BREAK);
  } else if (value == "invoke") {
    return build_pptoken_invoke(get_location(t, start),
                                create_interned_string(value));
  } else {
    return build_pptoken_identifier(get_location(t, start),
                                    create_interned_string(value));
  }
}
void skip_comment(Tokenizer *t) {
  tok_adv(t);
  if (tok_eof(t)) {
    return;
  }
  char c = tok_peek(t);
  while (true) {
    if (c == '\n') {
      tok_adv(t);
      return;
    }
    if (c == ';') {
      tok_adv(t);
      return;
    }
    tok_adv(t);
    if (!tok_eof(t)) {
      c = tok_peek(t);
    } else {
      return;
    }
  }
}
} // namespace

std::vector<PreprocessorToken>
preprocessor_tokenize(const string *const filename, const std::string &code) {

  Filename f = get_filename(filename);
  cache_sourcecode(f, code);
  Tokenizer t = tok_build(f, code);

  vector<PreprocessorToken> tokens;
  while (!tok_eof(&t)) {
    char c = tok_peek(&t);
    if (is_number(c)) {
      tokens.push_back(tokenize_number(&t));
    } else if (c == '"') {
      tokens.push_back(tokenize_string(&t));
    } else if (c == '#') {
      tokens.push_back(tokenize_symbol(&t));
    } else if (c == '$') {
      tokens.push_back(tokenize_macro(&t));
    } else if (c == '-') {
      tokens.push_back(tokenize_subtraction_operator(&t));
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
    } else if (c == '[') {
      tokens.push_back(
          build_pptoken(get_location(&t, t.pos), PpTokenTag::BRACKETOPEN));
      tok_adv(&t);
    } else if (c == ']') {
      tokens.push_back(
          build_pptoken(get_location(&t, t.pos), PpTokenTag::BRACKETCLOSE));
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
    } else if (c == ';') {
      skip_comment(&t);
    } else {
      throw compile_error(get_location(&t, t.pos),
                          "unexpected character '" + string(1, c) + "'\n");
    }
  }
  return tokens;
}
