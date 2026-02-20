#include "preprocess.hpp"
#include "../../lib/asap/util.hpp"
#include "preprocessor_token.hpp"
#include "preprocessor_tokenize.hpp"
#include "token.hpp"
#include <cassert>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;
namespace {

struct PpParser {
  filesystem::path absolute_filepath;
  vector<PreprocessorToken> tokens;
  unordered_map<uint64_t, Token> defines; // interned strings as key
  unordered_set<uint64_t> flags;          // interned strings as key
  IncludedModules *includes;
  uint64_t pos;
  uint64_t len;
};
PpParser build_PpParser(const string &absolute_current_path,
                        IncludedModules *includes,
                        vector<PreprocessorToken> tokens) {
  PpParser p;
  p.includes = includes;
  p.absolute_filepath = absolute_current_path;
  p.tokens = std::move(tokens);
  p.pos = 0;
  p.len = p.tokens.size();
  return p;
}
bool ppparser_eof(PpParser *p) { return p->pos >= p->len; }
void ppparser_adv(PpParser *p) { p->pos += 1; }
PreprocessorToken ppparser_peek(PpParser *p) {
  if (ppparser_eof(p)) {
    throw runtime_error("unexpected EOF\n");
  }
  return p->tokens.at(p->pos);
}

// token utils
Token pptoken_to_token(const PreprocessorToken &pptoken) {
  Token token;
  token.location = pptoken.location;
  switch (pptoken.tag) {
  case PpTokenTag::INT:
    token.as.INT = pptoken.as.INT;
    token.tag = TokenTag::INT;
    return token;
  case PpTokenTag::FLOAT:
    token.as.FLOAT = pptoken.as.FLOAT;
    token.tag = TokenTag::FLOAT;
    return token;
  case PpTokenTag::SYMBOL:
    token.as.SYMBOL = pptoken.as.SYMBOL;
    token.tag = TokenTag::SYMBOL;
    return token;
  case PpTokenTag::STRING:
    token.as.STR = pptoken.as.STR;
    token.tag = TokenTag::STRING;
    return token;
  case PpTokenTag::LET:
    token.tag = TokenTag::LET;
    return token;
  case PpTokenTag::SET:
    token.tag = TokenTag::SET;
    return token;
  case PpTokenTag::IF:
    token.tag = TokenTag::IF;
    return token;
  case PpTokenTag::ELIF:
    token.tag = TokenTag::ELIF;
    return token;
  case PpTokenTag::ELSE:
    token.tag = TokenTag::ELSE;
    return token;
  case PpTokenTag::FUNCTION:
    token.tag = TokenTag::FUNCTION;
    return token;
  case PpTokenTag::TRY:
    token.tag = TokenTag::TRY;
    return token;
  case PpTokenTag::EXPECT:
    token.tag = TokenTag::EXPECT;
    return token;
  case PpTokenTag::ASSIGN:
    token.tag = TokenTag::ASSIGN;
    return token;
  case PpTokenTag::BROPEN:
    token.tag = TokenTag::BROPEN;
    return token;
  case PpTokenTag::BRCLOSE:
    token.tag = TokenTag::BRCLOSE;
    return token;
  case PpTokenTag::CURLOPEN:
    token.tag = TokenTag::CURLOPEN;
    return token;
  case PpTokenTag::CURLCLOSE:
    token.tag = TokenTag::CURLCLOSE;
    return token;
  case PpTokenTag::COMMA:
    token.tag = TokenTag::COMMA;
    return token;
  case PpTokenTag::DOT:
    token.tag = TokenTag::DOT;
    return token;
  case PpTokenTag::BAR:
    token.tag = TokenTag::BAR;
    return token;
  case PpTokenTag::OP_ADD:
    token.tag = TokenTag::OP_ADD;
    return token;
  case PpTokenTag::OP_SUB:
    token.tag = TokenTag::OP_SUB;
    return token;
  case PpTokenTag::OP_MUL:
    token.tag = TokenTag::OP_MUL;
    return token;
  case PpTokenTag::OP_DIV:
    token.tag = TokenTag::OP_DIV;
    return token;
  case PpTokenTag::OP_MOD:
    token.tag = TokenTag::OP_MOD;
    return token;
  case PpTokenTag::OP_EQ:
    token.tag = TokenTag::OP_EQ;
    return token;
  case PpTokenTag::OP_NEQ:
    token.tag = TokenTag::OP_NEQ;
    return token;
  case PpTokenTag::OP_NOT:
    token.tag = TokenTag::OP_NOT;
    return token;
  case PpTokenTag::OP_OR:
    token.tag = TokenTag::OP_OR;
    return token;
  case PpTokenTag::OP_AND:
    token.tag = TokenTag::OP_AND;
    return token;
  case PpTokenTag::OPERATOR_LT:
    token.tag = TokenTag::OPERATOR_LT;
    return token;
  case PpTokenTag::OPERATOR_LTE:
    token.tag = TokenTag::OPERATOR_LTE;
    return token;
  case PpTokenTag::OPERATOR_GT:
    token.tag = TokenTag::OPERATOR_GT;
    return token;
  case PpTokenTag::OPERATOR_GTE:
    token.tag = TokenTag::OPERATOR_GTE;
    return token;
  case PpTokenTag::OPERATOR_STRCONCAT:
    token.tag = TokenTag::OPERATOR_STRCONCAT;
    return token;
  case PpTokenTag::IDENTIFIER:
    token.as.IDENTIFIER = pptoken.as.IDENTIFIER;
    token.tag = TokenTag::IDENTIFIER;
    return token;
  case PpTokenTag::RETURN:
    token.tag = TokenTag::RETURN;
    return token;
  case PpTokenTag::INCLUDE_MACRO:
  case PpTokenTag::DEFINE_MACRO:
  case PpTokenTag::FSET_MACRO:
  case PpTokenTag::FUNSET_MACRO:
  case PpTokenTag::IFFSET_MACRO:
  case PpTokenTag::IFNFSET_MACRO:
  case PpTokenTag::ENDIF_MACRO:
    throw runtime_error("invalid use of preprocessor directive\n");
  }
}

// flags
bool ppparser_has_flag(PpParser *p, const InternedString &name) {
  return p->flags.count(name.index) > 0;
}
void ppparser_set_flag(PpParser *p, const InternedString &definition) {
  if (p->flags.count(definition.index) > 0) {
    throw runtime_error("flag '" + resolve_interned_string(definition) +
                        "' is already set\n");
  }
  p->flags.insert(definition.index);
}
void ppparser_unset_flag(PpParser *p, const InternedString &definition) {
  if (p->flags.count(definition.index) == 0) {
    throw runtime_error("flag '" + resolve_interned_string(definition) +
                        "' is not set\n");
  }
  p->flags.erase(definition.index);
}

// definitions
bool ppparser_has_def(PpParser *p, const InternedString &name) {
  return p->defines.count(name.index) > 0;
}
void ppparser_def(PpParser *p, const InternedString &definition,
                  const PreprocessorToken &token) {
  if (p->defines.count(definition.index) > 0) {
    throw runtime_error("macro '" + resolve_interned_string(definition) +
                        "' is already defined\n");
  }
  p->defines[definition.index] = pptoken_to_token(token);
}
Token ppparser_get_def(PpParser *p, const InternedString &name) {
  return p->defines.at(name.index);
}

// assertions
void assert_identifier(const PreprocessorToken &token) {
  if (token.tag != PpTokenTag::IDENTIFIER) {
    throw runtime_error("expected an identifier\n");
  }
}
void assert_literal_string(const PreprocessorToken &token) {
  if (token.tag != PpTokenTag::STRING) {
    throw runtime_error("expected a literal string\n");
  }
}
void assert_literal_value(const PreprocessorToken &token) {
  switch (token.tag) {
  case PpTokenTag::INT:
    break;
  case PpTokenTag::FLOAT:
    break;
  case PpTokenTag::SYMBOL:
    break;
  case PpTokenTag::STRING:
    break;
  default:
    throw runtime_error("expected a value\n");
  }
}

void assert_isat_assign(PpParser *p) {
  if (ppparser_peek(p).tag != PpTokenTag::ASSIGN) {
    throw runtime_error("expected an '='\n");
  }
}
} // namespace

std::vector<Token>
preprocess_pptokens(const filesystem::path &absolute_current_path,
                    IncludedModules *includes,
                    const std::vector<PreprocessorToken> &tokens);

namespace {
vector<Token> parse_token(PpParser *p, const PreprocessorToken &token);
void parse_define(PpParser *p) {
  ppparser_adv(p); // to definition name

  PreprocessorToken identifier = ppparser_peek(p);
  assert_identifier(identifier);
  ppparser_adv(p); // to '='

  assert_isat_assign(p);
  ppparser_adv(p); // to value

  PreprocessorToken value = ppparser_peek(p);
  assert_literal_value(value);
  ppparser_def(p, identifier.as.IDENTIFIER, value);
  ppparser_adv(p); // to next
}

void parse_fset(PpParser *p) {
  ppparser_adv(p); // to flag
  PreprocessorToken identifier = ppparser_peek(p);
  assert_identifier(identifier);
  ppparser_set_flag(p, identifier.as.IDENTIFIER);
  ppparser_adv(p); // to next
}
void parse_funset(PpParser *p) {
  ppparser_adv(p); // to flag
  PreprocessorToken identifier = ppparser_peek(p);
  assert_identifier(identifier);
  if (ppparser_has_flag(p, identifier.as.IDENTIFIER)) {
    ppparser_unset_flag(p, identifier.as.IDENTIFIER);
  } else {
    throw runtime_error("flag '" +
                        resolve_interned_string(identifier.as.IDENTIFIER) +
                        "' not set\n");
  }
  ppparser_adv(p); // to next
}

vector<Token> parse_iffset(PpParser *p, bool mod) {
  ppparser_adv(p); // to identifier
  PreprocessorToken identifier = ppparser_peek(p);
  assert_identifier(identifier);
  ppparser_adv(p); // to first conditional token

  vector<Token> result;
  bool condition_met = (ppparser_has_flag(p, identifier.as.IDENTIFIER) == mod);
  while (ppparser_peek(p).tag != PpTokenTag::ENDIF_MACRO) {
    auto next = ppparser_peek(p);
    if (condition_met) {
      auto inner = parse_token(p, next);
      result.insert(result.end(), inner.begin(), inner.end());
    } else if (next.tag == PpTokenTag::IFFSET_MACRO ||
               next.tag == PpTokenTag::IFNFSET_MACRO) {
      // Skip identifier and inner tokens
      ppparser_adv(p);            // skip the macro
      auto id = ppparser_peek(p); //
      assert_identifier(id);      // ensure identifier exists
      ppparser_adv(p);            // skip the identifier

      // skip the contents counting depth
      int depth = 1;
      while (depth > 0) {
        auto t = ppparser_peek(p);
        if (t.tag == PpTokenTag::IFFSET_MACRO ||
            t.tag == PpTokenTag::IFNFSET_MACRO) {
          depth++;
        } else if (t.tag == PpTokenTag::ENDIF_MACRO) {
          depth--;
        }
        ppparser_adv(p); // skip -endif
      }
    } else {
      ppparser_adv(p); // just skip any token
    }
  }
  ppparser_adv(p); // to next (past endif)
  return result;
}
Token parse_identifier(PpParser *p, const PreprocessorToken &identifier) {
  if (ppparser_has_def(p, identifier.as.IDENTIFIER)) {
    Token t = ppparser_get_def(p, identifier.as.IDENTIFIER);
    // overwrite location
    t.location = identifier.location;
    ppparser_adv(p);
    return t;
  } else {
    ppparser_adv(p);
    return pptoken_to_token(identifier);
  }
}

vector<Token> run_include(PpParser *p) {
  ppparser_adv(p); // skip to match

  PreprocessorToken rel_path = ppparser_peek(p);
  assert_literal_string(rel_path);
  ppparser_adv(p);

  filesystem::path absolute_filepath =
      filesystem::canonical(p->absolute_filepath.parent_path() /
                            resolve_interned_string(rel_path.as.STR));
  if (p->includes->absolute_filepaths.count(absolute_filepath) > 0) {
    return vector<Token>({});
  } else {
    string content = read_file(absolute_filepath);
    return preprocess_pptokens(
        absolute_filepath, p->includes,
        preprocessor_tokenize(absolute_filepath, content));
  }
}

vector<Token> parse_token(PpParser *p, const PreprocessorToken &token) {
  vector<Token> result;
  if (token.tag == PpTokenTag::DEFINE_MACRO) {
    parse_define(p);
  } else if (token.tag == PpTokenTag::FSET_MACRO) {
    parse_fset(p);
  } else if (token.tag == PpTokenTag::FUNSET_MACRO) {
    parse_funset(p);
  } else if (token.tag == PpTokenTag::IFFSET_MACRO) {
    auto body = parse_iffset(p, true);
    result.insert(result.end(), body.begin(), body.end());
  } else if (token.tag == PpTokenTag::IFNFSET_MACRO) {
    auto body = parse_iffset(p, false);
    result.insert(result.end(), body.begin(), body.end());
  } else if (token.tag == PpTokenTag::ENDIF_MACRO) {
    throw runtime_error(
        "invalid use of preprocessor directive: no condition to close\n");
  } else if (token.tag == PpTokenTag::IDENTIFIER) {
    result.push_back(parse_identifier(p, token));
  } else if (token.tag == PpTokenTag::INCLUDE_MACRO) {
    auto include_result = run_include(p);
    result.insert(result.end(), include_result.begin(), include_result.end());
  } else {
    result.push_back(pptoken_to_token(token));
    ppparser_adv(p);
  }
  return result;
}
} // namespace

std::vector<Token>
preprocess_pptokens(const filesystem::path &absolute_current_path,
                    IncludedModules *includes,
                    const std::vector<PreprocessorToken> &tokens) {
  assert(includes != NULL);

  PpParser p = build_PpParser(absolute_current_path, includes, tokens);
  vector<Token> result;

  PreprocessorToken curr;
  while (!ppparser_eof(&p)) {
    curr = ppparser_peek(&p);
    auto parsed = parse_token(&p, curr);
    result.insert(result.end(), parsed.begin(), parsed.end());
  }

  return result;
}

std::vector<Token> preprocess(const std::string &entrypoint) {
  string content = read_file(entrypoint);
  IncludedModules included_modules;
  filesystem::path absolute_path = filesystem::canonical(entrypoint);
  auto pptokens = preprocessor_tokenize(absolute_path, content);
  return preprocess_pptokens(absolute_path, &included_modules, pptokens);
}
