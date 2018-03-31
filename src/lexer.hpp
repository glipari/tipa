/*
  Copyright 2015-2018 Giuseppe Lipari
  email: giuseppe.lipari@univ-lille.fr
  
  This file is part of TiPa.

  TiPa is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  TiPa is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.
  
  You should have received a copy of the GNU General Public License
  along with TiPa. If not, see <http://www.gnu.org/licenses/>
 */
#ifndef __LEXER_HPP__
#define __LEXER_HPP__

#include <string>
#include <iostream>
#include <vector>
#include <stack>

#define LEX_EMPTY  0
#define LEX_ERROR -1

typedef int token_id;

namespace tipa {

    class parse_exc : public std::exception {
	std::string msg;
    public:
	parse_exc();
	parse_exc(const std::string &err_msg); 
	std::string what();
	~parse_exc() throw();
    };


/** 
    This class is a simple pair that represent a token for the Lexer.
    A token is a pair token-name, regular expression that identifies
    the token.  

    @todo embed the regex into the class for optimizing execution
    time.  Could also be done in a lazy way...
*/
    struct token {
	token(const std::pair<token_id, std::string> &p) :
	    name(p.first), expr(p.second) {}

	token(const token_id &n, const std::string &e) :
	    name(n), expr(e) {}

	token_id    get_name() const { return name; } 
	std::string get_expr() const { return expr; } 
    private:
	token_id name;
	std::string expr;
    };

    typedef std::pair<token_id, std::string> token_val;

/// These are already defined in the lexer
    const int LEX_LIB_BASE    = 1000;
    const int LEX_INT         = (LEX_LIB_BASE + 1);
    const int LEX_IDENTIFIER  = (LEX_LIB_BASE + 2);
    const int LEX_EXTRACTED_STRING = (LEX_LIB_BASE + 3);
    const int LEX_CHAR        = (LEX_LIB_BASE + 4);
    
    

    const token tk_int(LEX_INT, "^\\d+\\b"); 
    const token tk_ident(LEX_IDENTIFIER, "^[^\\d\\W]\\w*");

/**
   This class performs the work of the lexer.  

   It is initialized with a list of tokens (see the token class). Then
   it is necessary to pass in the input stream by calling set_stream(). 

   At this stage, the parsing can start: the user invokes the
   get_token() function, usually in a loop, to retreive the next
   token, and the function returns a pair of strings: 
 
   - The first one contains the token identifier, 
   
   - the second one the actual value of the token as read from the stream. 

   For example, if we want to parse a list of integers, we write the following code: 
   
   \code
   lexer lex({
   {"int", "^\\d+\\b"},
   {"sep", ","}
   });

   stringstream str("32, 15");
   lex.set_stream(str);
   \endcode

   Successive calls to lex.get_token() will return {"int", "32"}
   {"sep", ","} {"int", "15"}.

   Notice that lexer automatically skips standard space characters,
   like " \t\n" etc.  

   @todo solve the issues with copying lexers. It would be better to
   copy the stream, rather than having the pointer to it.
*/
    class lexer {
    protected:
	struct ctx {
	    int dist;
	    unsigned nl, nc;
	};

	std::string::iterator start;
	std::istream *p_input;
	std::string curr_line;
	unsigned nline, ncol;

	std::vector<std::string> all_lines;
	std::stack<ctx> saved_ctx; 

	std::string comment_begin; 
	std::string comment_end;
	std::string comment_single_line;

	bool next_line();
	void skip_spaces();
    public:
    
	lexer();
    
	/// Saves the context of the lexer, we can restore it later
	void save(); 
	/// Restores the context to the last saved one 
	void restore();
	/// Discard the last saved context
	void discard_saved();

	/// Configures the lexer to skip all characters between strings b
	/// and e, and all characters from string sl until the end of the
	/// current line. The intended use is to skip comments.
	void set_comment(const std::string &b, 
			 const std::string &e, 
			 const std::string &sl);

	/// set the stream for this lexer
	void set_stream(std::istream &in);

	/// checks if the token is found, and returns it, or an error
	token_val try_token(const token &x);

	/// returns the current position (line num, column num)
	std::pair<int, int> get_pos() const { return {nline, ncol}; }

	/// returns the line that is currently being processed
	std::string get_currline() const { return curr_line; }

	/**
	   Extracts a string encompassed between the two strings
	   sym_begin and sym_end. It takes into account nesting, so it
	   returns the string corresponding to the matching symbol, and
	   throws an exception if it does not find one.

	   Useful for implementing nesting parsers, and extract comments.
	*/
	std::string extract(const std::string &sym_begin, const std::string &sym_end);
	std::string extract_line();
    };

    class ahead_lexer : public lexer {
	std::vector<token> array;
    public:
	ahead_lexer(const std::vector<token> &keys);
	void add_token(const token_id &name, const std::string &expr);

	/// returns the next token as a pair of strings
	std::pair<token_id, std::string> get_token();
    };
}

#endif
