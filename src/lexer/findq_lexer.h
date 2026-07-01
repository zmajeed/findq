#ifndef FINDQ_LEXER_H
#define FINDQ_LEXER_H
// findq_lexer.h

/*
MIT License

Copyright (c) 2024-2026 Zartaj Majeed

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "findq_lexer_guard.h"
#include "findq_parser.bison.h"

namespace findqparser {
using namespace std;

class Lexer: public yyFlexLexer {
public:

// can only declare here since flex generates the implementation
  FindqParser::symbol_type yylex(location&);

  Lexer() = default;

  explicit Lexer(istream& yyin_arg): yyFlexLexer(&yyin_arg) {}

  enum class State {
// lexer state to match and return a string argument token
    stringArg,
// lexer state to match and return a number argument token
    numberArg,
    execArgs,
  };

  int setState(State);
  void unsetState();

private:

  using yyFlexLexer::yylex;

};

}

#endif


