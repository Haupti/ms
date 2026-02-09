#include "../../lib/asap/util.hpp"
#include "preprocessor_token.hpp"
#include "preprocessor_tokenize.hpp"
#include "token.hpp"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;
namespace {
struct PpParser {
  vector<PreprocessorToken> tokens;
  unordered_map<uint64_t, PreprocessorToken> defines; // interned strings as key
  unordered_set<uint64_t> defines_no_value;           // interned strings as key
  uint64_t pos;
  uint64_t len;
};
PpParser build_PpParser(const vector<PreprocessorToken> &tokens) {
  PpParser p;
  p.tokens = tokens;
  p.pos = 0;
  p.len = p.tokens.size();
  return p;
}
bool ppparser_eof(PpParser *p) { return p->pos >= p->len; }
void ppparser_adv(PpParser *p) { p->pos++; }
PreprocessorToken ppparser_peek(PpParser *p) {
  if (ppparser_eof(p)) {
    throw runtime_error("unexpected EOF\n");
  }
  return p->tokens.at(p->pos);
}
bool ppparser_peek_next_is_assign_and_not_eof(PpParser *p) {
  return p->pos + 1 < p->len &&
         p->tokens.at(p->pos + 1).tag == PpTokenTag::IDENTIFIER;
}
void ppparser_def(PpParser *p, const InternedString &definition) {
  if (p->defines.count(definition.index) > 0 ||
      p->defines_no_value.count(definition.index) > 0) {
    throw runtime_error("macro '" + resolve_interned_string(definition) +
                        "' is already defined\n");
  }
  p->defines_no_value.insert(definition.index);
}
void ppparser_def(PpParser *p, const InternedString &definition,
                  const PreprocessorToken &token) {
  if (p->defines.count(definition.index) > 0 ||
      p->defines_no_value.count(definition.index) > 0) {
    throw runtime_error("macro '" + resolve_interned_string(definition) +
                        "' is already defined\n");
  }
  p->defines[definition.index] = token;
}
void ppparser_undef(PpParser *p, const InternedString &definition) {
  if (p->defines.count(definition.index) == 0 &&
      p->defines_no_value.count(definition.index) == 0) {
    throw runtime_error("macro '" + resolve_interned_string(definition) +
                        "' is not defined\n");
  }
  p->defines.erase(definition.index);
}

void assert_identifier(const PreprocessorToken &token) {
  if (token.tag != PpTokenTag::IDENTIFIER) {
    throw runtime_error("expected an identifier\n");
  }
}
void assert_literal(const PreprocessorToken &token) {
  PpTokenTag tag = token.tag;
  if (tag != PpTokenTag::STRING || tag != PpTokenTag::INT ||
      tag != PpTokenTag::FLOAT || tag != PpTokenTag::SYMBOL) {
    throw runtime_error("expected a constant value\n");
  }
}
} // namespace

std::vector<Token> preprocess(const std::string &entrypoint) {
  string content = read_file(entrypoint);

  PpParser p = build_PpParser(preprocessor_tokenize(entrypoint, content));
  vector<Token> result;

  PreprocessorToken curr;
  while (!ppparser_eof(&p)) {
    curr = ppparser_peek(&p);
    if (curr.tag == PpTokenTag::DEFINE_MACRO) {
      ppparser_adv(&p);
      PreprocessorToken identifier = ppparser_peek(&p);
      assert_identifier(identifier);
      if (ppparser_peek_next_is_assign_and_not_eof(&p)) {
        ppparser_adv(&p); // move to assign
        ppparser_adv(&p); // move to value
        PreprocessorToken value = ppparser_peek(&p);
        assert_literal(value);
        ppparser_def(&p, identifier.as.IDENTIFIER, value);
        ppparser_adv(&p); // move to next
      } else {
        ppparser_def(&p, identifier.as.IDENTIFIER);
        ppparser_adv(&p); // move to next
      }
    } else if (curr.tag == PpTokenTag::IDENTIFIER) {
      // TODO check if no-value define -> error
      // TODO check if value define -> replace
      // TODO else, just add to output vector
    }
    // TODO parse other macro options
    // TODO other tokens are just added to output vector
  }

  return result;
}
