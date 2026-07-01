// findq_parser.gtest.cpp

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

#include <unistd.h>
#include <getopt.h>

#include <sstream>
#include <string>
#include <optional>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "lexer/findq_lexer.h"
#include "findq_parser.bison.h"

using namespace std;

using namespace ::testing;

namespace findqparser::testing {

using Token = FindqParser::symbol_kind;

MATCHER_P(IsTokenKind, tokenKind, "") { return arg.kind() == tokenKind; }

// custom matcher macro
MATCHER_P(MatchTokenP, expectedToken, "custom token matcher macro") {
  if(arg.kind() != expectedToken.kind()) {
    return false;
  }
  switch(arg.kind()) {
  case Token::S_STARTING_POINT:
  case Token::S_STRING_ARG:
    return arg.value.template as<string>() == expectedToken.value.template as<string>();
  case Token::S_NUMBER_ARG:
    return arg.value.template as<NumberArg>() == expectedToken.value.template as<NumberArg>();
  default:
    return true;
  }
}

// custom matcher class
struct TokenMatcher {
  using is_gtest_matcher = void;

  FindqParser::symbol_type expectedToken;

  TokenMatcher(const FindqParser::symbol_type& expected): expectedToken(expected) {}

  bool MatchAndExplain(const FindqParser::symbol_type& token, ostream*) const {
    if(token.kind() != expectedToken.kind()) {
      return false;
    }
    switch(token.kind()) {
    case Token::S_STARTING_POINT:
    case Token::S_STRING_ARG:
      return token.value.template as<string>() == expectedToken.value.as<string>();
    case Token::S_NUMBER_ARG:
      return token.value.template as<NumberArg>() == expectedToken.value.as<NumberArg>();
    default:
      return true;
    }
  }

  void DescribeTo(ostream* os) const {*os << "match success";}

  void DescribeNegationTo(ostream* os) const {*os << "match failure";}

};

TokenMatcher MatchToken(const FindqParser::symbol_type& token) {
  return TokenMatcher(token);
}

TEST(FindqParser, test_0) {
  stringstream s("find -true");
  Lexer lexer(s);

  location loc{};
  BisonParam bisonParam{lexer};

  FindqParser parser([&lexer](location& loc) -> FindqParser::symbol_type {
    return lexer.yylex(loc);
  },
  bisonParam,
  loc);

  EXPECT_EQ(parser(), 0);
}

TEST(FindqParser, test_1) {
  stringstream s("find -false");
  Lexer lexer(s);

  location loc{};
  BisonParam bisonParam{lexer};

  FindqParser parser([&lexer](location& loc) -> FindqParser::symbol_type {
    return lexer.yylex(loc);
  },
  bisonParam,
  loc);

  EXPECT_EQ(parser(), 0);
}

TEST(FindqParser, test_2) {
  stringstream s("find ( -name build ) -prune");
  Lexer lexer(s);

  location loc{};
  BisonParam bisonParam{lexer};

  FindqParser parser([&lexer](location& loc) -> FindqParser::symbol_type {
    return lexer.yylex(loc);
  },
  bisonParam,
  loc);

  EXPECT_EQ(parser(), 0);
}

TEST(FindqParser, test_3) {
  stringstream s("find ( -name build -o -name node_modules ) -prune");
  Lexer lexer(s);

  location loc{};
  BisonParam bisonParam{lexer};

  FindqParser parser([&lexer](location& loc) -> FindqParser::symbol_type {
    return lexer.yylex(loc);
  },
  bisonParam,
  loc);

  EXPECT_EQ(parser(), 0);
}

TEST(FindqParser, test_4) {
  stringstream s("find -true");
  Lexer lexer(s);

  location loc{};
  BisonParam bisonParam{lexer};

  NiceMock<MockFunction<FindqParser::symbol_type(location&)>> mock_yylex;
  NiceMock<MockFunction<void(FindqParser::symbol_kind_type)>> check_token;

  ON_CALL(mock_yylex, Call).WillByDefault([&lexer, &check_token](location& loc){
      auto token = lexer.yylex(loc);
      check_token.Call(token.kind());
      return token;
  });

  EXPECT_CALL(mock_yylex, Call(_)).Times(3);

  EXPECT_CALL(check_token, Call(Token::S_FIND));
  EXPECT_CALL(check_token, Call(Token::S_TRUE));
  EXPECT_CALL(check_token, Call(Token::S_YYEOF));

  FindqParser parser(mock_yylex.AsStdFunction(), bisonParam, loc);

  EXPECT_EQ(parser(), 0);
}

TEST(FindqParser, test_5) {
  stringstream s("find ( -name build -o -name node_modules ) -prune");
  Lexer lexer(s);

  location loc{};
  BisonParam bisonParam{lexer};

  NiceMock<MockFunction<FindqParser::symbol_type(location&)>> mock_yylex;
  NiceMock<MockFunction<void(const FindqParser::symbol_kind_type)>> checkTokenType;
  NiceMock<MockFunction<void(const string&)>> checkStringToken;

  ON_CALL(mock_yylex, Call).WillByDefault([&lexer, &checkTokenType, &checkStringToken](location& loc) {
      auto token = lexer.yylex(loc);
      checkTokenType.Call(token.kind());
      if(token.kind() == Token::S_STRING_ARG) {
        checkStringToken.Call(token.value.as<string>());
      }
      return token;
  });

  EXPECT_CALL(mock_yylex, Call(_)).Times(AnyNumber());

  EXPECT_CALL(checkTokenType, Call(Token::S_FIND));
  EXPECT_CALL(checkTokenType, Call(Token::S_LEFT_PAREN));
  EXPECT_CALL(checkTokenType, Call(Token::S_NAME)).Times(2);
  EXPECT_CALL(checkTokenType, Call(Token::S_STRING_ARG)).Times(2);
  EXPECT_CALL(checkTokenType, Call(Token::S_OR));
  EXPECT_CALL(checkTokenType, Call(Token::S_RIGHT_PAREN));
  EXPECT_CALL(checkTokenType, Call(Token::S_PRUNE));
  EXPECT_CALL(checkTokenType, Call(Token::S_YYEOF));

  EXPECT_CALL(checkStringToken, Call("build"));
  EXPECT_CALL(checkStringToken, Call("node_modules"));

  FindqParser parser(mock_yylex.AsStdFunction(), bisonParam, loc);

  EXPECT_EQ(parser(), 0);
}

TEST(FindqParser, test_6) {
  stringstream s("find -type d -mindepth 2 -maxdepth 4");
  Lexer lexer(s);

  location loc{};
  BisonParam bisonParam{lexer};

  NiceMock<MockFunction<FindqParser::symbol_type(location&)>> mock_yylex;
  NiceMock<MockFunction<void(const FindqParser::symbol_kind_type)>> checkTokenType;
  NiceMock<MockFunction<void(const string&)>> checkStringToken;
  NiceMock<MockFunction<void(const NumberArg&)>> checkNumberToken;

  ON_CALL(mock_yylex, Call).WillByDefault(
    [&lexer, &checkTokenType, &checkStringToken, &checkNumberToken](location& loc) {
      auto token = lexer.yylex(loc);
      checkTokenType.Call(token.kind());
      if(token.kind() == Token::S_STRING_ARG) {
        checkStringToken.Call(token.value.as<string>());
      } else if(token.kind() == Token::S_NUMBER_ARG) {
        checkNumberToken.Call(token.value.as<NumberArg>());
      }
      return token;
  });

  EXPECT_CALL(mock_yylex, Call(_)).Times(AtLeast(1));

  EXPECT_CALL(checkStringToken, Call("d"));
  EXPECT_CALL(checkNumberToken, Call(FieldsAre(2u, 0)));
  EXPECT_CALL(checkNumberToken, Call(FieldsAre(4u, 0)));

  FindqParser parser(mock_yylex.AsStdFunction(), bisonParam, loc);

  EXPECT_EQ(parser(), 0);
}

TEST(FindqParser, test_7) {
  stringstream s("find src tools -type d -mindepth 2 -maxdepth 4");
  Lexer lexer(s);

  location loc{};
  BisonParam bisonParam{lexer};

  NiceMock<MockFunction<FindqParser::symbol_type(location&)>> mock_yylex;
  NiceMock<MockFunction<void(const FindqParser::symbol_kind_type)>> checkTokenType;
  NiceMock<MockFunction<void(const string&)>> checkStringToken;
  NiceMock<MockFunction<void(const NumberArg&)>> checkNumberToken;

  ON_CALL(mock_yylex, Call).WillByDefault(
    [&lexer, &checkTokenType, &checkStringToken, &checkNumberToken](location& loc) {
      auto token = lexer.yylex(loc);
      checkTokenType.Call(token.kind());
      switch(token.kind()) {
      case Token::S_STRING_ARG:
      case Token::S_STARTING_POINT:
        checkStringToken.Call(token.value.as<string>());
        break;
      case Token::S_NUMBER_ARG:
        checkNumberToken.Call(token.value.as<NumberArg>());
        break;
      default:
        break;
      }
      return token;
  });

  EXPECT_CALL(mock_yylex, Call(_)).Times(AtLeast(1));

  EXPECT_CALL(checkTokenType, Call(_)).Times(AnyNumber());
  EXPECT_CALL(checkTokenType, Call(Token::S_STARTING_POINT)).Times(2);

  EXPECT_CALL(checkStringToken, Call("src"));
  EXPECT_CALL(checkStringToken, Call("tools"));
  EXPECT_CALL(checkStringToken, Call("d"));
  EXPECT_CALL(checkNumberToken, Call(FieldsAre(2u, 0)));
  EXPECT_CALL(checkNumberToken, Call(FieldsAre(4u, 0)));

  FindqParser parser(mock_yylex.AsStdFunction(), bisonParam, loc);

  EXPECT_EQ(parser(), 0);
}

TEST(FindqParser, test_8) {
  stringstream s("find src tools -type f -exec ls -l {} ;");
  Lexer lexer(s);

  location loc{};
  BisonParam bisonParam{lexer};

  NiceMock<MockFunction<FindqParser::symbol_type(location&)>> mock_yylex;
  NiceMock<MockFunction<void(const FindqParser::symbol_kind_type)>> checkTokenType;

  NiceMock<MockFunction<void(const FindqParser::symbol_type&)>> checkTokenType1;

  NiceMock<MockFunction<void(const string&)>> checkStringToken;
  NiceMock<MockFunction<void(const NumberArg&)>> checkNumberToken;

  NiceMock<MockFunction<FindqParser::symbol_type(const FindqParser::symbol_type&)>> checkTokenType2;
  ON_CALL(checkTokenType2, Call).WillByDefault(ReturnArg<0>());

  ON_CALL(mock_yylex, Call).WillByDefault(
    [&lexer, &checkTokenType, &checkStringToken, &checkNumberToken, &checkTokenType1, &checkTokenType2](location& loc) -> FindqParser::symbol_type {
      FindqParser::symbol_type token(lexer.yylex(loc));
      checkTokenType.Call(token.kind());

      checkTokenType1.Call(token);

      switch(token.kind()) {
      case Token::S_STRING_ARG:
      case Token::S_STARTING_POINT:
        checkStringToken.Call(token.value.as<string>());
        break;
      case Token::S_NUMBER_ARG:
        checkNumberToken.Call(token.value.as<NumberArg>());
        break;
      default:
        break;
      }

      return checkTokenType2.Call(token);
  });

  EXPECT_CALL(mock_yylex, Call(_)).Times(AtLeast(1));

  EXPECT_CALL(checkTokenType, Call(_)).Times(AnyNumber());
  EXPECT_CALL(checkTokenType, Call(Token::S_STARTING_POINT)).Times(AtLeast(1));
  EXPECT_CALL(checkTokenType, Call(Token::S_EXEC));
  EXPECT_CALL(checkTokenType, Call(Token::S_SEMICOLON));

  EXPECT_CALL(checkTokenType1, Call(_)).Times(AnyNumber());
  EXPECT_CALL(checkTokenType1, Call(IsTokenKind(Token::S_EXEC)));
  EXPECT_CALL(checkTokenType1, Call(Truly([](const FindqParser::symbol_type& token) {return token.kind() == Token::S_SEMICOLON;})));
  EXPECT_CALL(checkTokenType1, Call(MatchToken(FindqParser::make_STARTING_POINT("src", loc))));
  EXPECT_CALL(checkTokenType1, Call(MatchToken(FindqParser::make_STARTING_POINT("tools", loc))));

  EXPECT_CALL(checkTokenType2, Call(_)).Times(AnyNumber());
  EXPECT_CALL(checkTokenType2, Call(MatchToken(FindqParser::make_STARTING_POINT("src", loc))));

  EXPECT_CALL(checkStringToken, Call(_)).Times(AnyNumber());
  EXPECT_CALL(checkStringToken, Call("src"));
  EXPECT_CALL(checkStringToken, Call("f"));
  EXPECT_CALL(checkStringToken, Call("ls"));
  EXPECT_CALL(checkStringToken, Call("-l"));
  EXPECT_CALL(checkStringToken, Call("{}"));

  FindqParser parser(mock_yylex.AsStdFunction(), bisonParam, loc);

  EXPECT_EQ(parser(), 0);
}

TEST(FindqParser, test_9) {
  stringstream s("find . -type f ! -path '*/node_modules/*'");
  Lexer lexer(s);

  location loc{};
  BisonParam bisonParam{lexer};

  NiceMock<MockFunction<FindqParser::symbol_type(const FindqParser::symbol_type&)>> mock_yylex_return;
  ON_CALL(mock_yylex_return, Call).WillByDefault(ReturnArg<0>());

  EXPECT_CALL(mock_yylex_return, Call(_)).Times(AnyNumber());
  EXPECT_CALL(mock_yylex_return, Call(MatchToken(FindqParser::make_STARTING_POINT(".", loc))));
  EXPECT_CALL(mock_yylex_return, Call(MatchToken(FindqParser::make_TYPE(loc))));
  EXPECT_CALL(mock_yylex_return, Call(MatchToken(FindqParser::make_STRING_ARG("f", loc))));
  EXPECT_CALL(mock_yylex_return, Call(MatchToken(FindqParser::make_NOT(loc))));
  EXPECT_CALL(mock_yylex_return, Call(MatchToken(FindqParser::make_PATH(loc))));
  EXPECT_CALL(mock_yylex_return, Call(MatchToken(FindqParser::make_STRING_ARG("*/node_modules/*", loc))));

  FindqParser parser([&lexer, &mock_yylex_return](location& loc) -> FindqParser::symbol_type {
    return mock_yylex_return.Call(lexer.yylex(loc));
  },
  bisonParam,
  loc);


  EXPECT_EQ(parser(), 0);
}

TEST(FindqParser, test_10) {
  stringstream s("find . -type f ! -path '*/node_modules/*'");
  Lexer lexer(s);

  location loc{};
  BisonParam bisonParam{lexer};

  NiceMock<MockFunction<FindqParser::symbol_type(const FindqParser::symbol_type&)>> mock_yylex_return;
  ON_CALL(mock_yylex_return, Call).WillByDefault(ReturnArg<0>());

  EXPECT_CALL(mock_yylex_return, Call(_)).Times(AnyNumber());
  EXPECT_CALL(mock_yylex_return, Call(MatchTokenP(FindqParser::make_STARTING_POINT(".", loc))));
  EXPECT_CALL(mock_yylex_return, Call(MatchToken(FindqParser::make_TYPE(loc))));
  EXPECT_CALL(mock_yylex_return, Call(MatchToken(FindqParser::make_STRING_ARG("f", loc))));
  EXPECT_CALL(mock_yylex_return, Call(MatchToken(FindqParser::make_NOT(loc))));
  EXPECT_CALL(mock_yylex_return, Call(MatchToken(FindqParser::make_PATH(loc))));
  EXPECT_CALL(mock_yylex_return, Call(MatchToken(FindqParser::make_STRING_ARG("*/node_modules/*", loc))));

  FindqParser parser([&lexer, &mock_yylex_return](location& loc) -> FindqParser::symbol_type {
    return mock_yylex_return.Call(lexer.yylex(loc));
  },
  bisonParam,
  loc);


  EXPECT_EQ(parser(), 0);
}

TEST(FindqParser, test_11) {
  stringstream s("find . -type f ! -path '*/node_modules/*'");
  Lexer lexer(s);

  NiceMock<MockFunction<FindqParser::symbol_type(const FindqParser::symbol_type&)>> mock_yylex_return;
  ON_CALL(mock_yylex_return, Call).WillByDefault(ReturnArg<0>());

  NiceMock<MockFunction<void()>> mock_binary_expression_from_and_cb;
  NiceMock<MockFunction<void()>> mock_and_expression_from_list_cb;
  NiceMock<MockFunction<void()>> mock_unary_expression_from_not_cb;
  NiceMock<MockFunction<void(const string&)>> mock_string_arg_cb;

  location loc{};
  BisonParam bisonParam{lexer,
    {
      mock_binary_expression_from_and_cb.AsStdFunction(),
      mock_and_expression_from_list_cb.AsStdFunction(),
      mock_unary_expression_from_not_cb.AsStdFunction(),
      mock_string_arg_cb.AsStdFunction(),
    }
  };

  EXPECT_CALL(mock_yylex_return, Call(_)).Times(AnyNumber());
  EXPECT_CALL(mock_yylex_return, Call(MatchTokenP(FindqParser::make_STARTING_POINT(".", loc))));
  EXPECT_CALL(mock_yylex_return, Call(MatchToken(FindqParser::make_TYPE(loc))));
  EXPECT_CALL(mock_yylex_return, Call(MatchToken(FindqParser::make_STRING_ARG("f", loc))));
  EXPECT_CALL(mock_yylex_return, Call(MatchToken(FindqParser::make_NOT(loc))));
  EXPECT_CALL(mock_yylex_return, Call(MatchToken(FindqParser::make_PATH(loc))));
  EXPECT_CALL(mock_yylex_return, Call(MatchToken(FindqParser::make_STRING_ARG("*/node_modules/*", loc))));

  EXPECT_CALL(mock_string_arg_cb, Call(_)).Times(AnyNumber());
  EXPECT_CALL(mock_string_arg_cb, Call("f"));
  EXPECT_CALL(mock_string_arg_cb, Call("*/node_modules/*"));

// both After() clause and InSequence codeblock work but test case expression has just one -and and one -not so cannot have more than one expectation for each
#if 1
  Expectation expect_and = EXPECT_CALL(mock_and_expression_from_list_cb, Call());
  Expectation expect_not = EXPECT_CALL(mock_unary_expression_from_not_cb, Call());

  EXPECT_CALL(mock_binary_expression_from_and_cb, Call())
    .After(expect_and, expect_not);
#else
  {
    InSequence s;
    EXPECT_CALL(mock_unary_expression_from_not_cb, Call());
    EXPECT_CALL(mock_and_expression_from_list_cb, Call());
    EXPECT_CALL(mock_binary_expression_from_and_cb, Call());
  }
#endif

  FindqParser parser([&lexer, &mock_yylex_return](location& loc) -> FindqParser::symbol_type {
    return mock_yylex_return.Call(lexer.yylex(loc));
  },
  bisonParam,
  loc);


  EXPECT_EQ(parser(), 0);
}


}

