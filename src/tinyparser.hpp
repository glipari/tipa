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
#ifndef __TINYPARSE_HPP__
#define __TINYPARSE_HPP__

#include <vector>
#include <memory>
#include <functional>
#include <exception>
#include <lexer.hpp>

#define ERR_PARSE_SEQ   -100
#define ERR_PARSE_ALT   -101

namespace tipa {

    /** It contains the lexer and the last token that has been read,
     * that is the parser state during parsing. An object of this
     * class must be created by the user and passed to the parse()
     * method of the root rule. It is then passed around the rule tree
     * structure.
     */
    class parser_context {
    public:
        lexer lex;
        std::vector<token_val> collected;
        std::stack<unsigned int> ncoll;
    
        token_val error_msg;
    public:
        parser_context(); 

        void set_stream(std::istream &in);
        void set_comment(const std::string &comment_begin, 
                         const std::string &comment_end,
                         const std::string &comment_single_line);

        token_val        try_token(const token &tk);
        std::string      extract(const std::string &op, const std::string &cl);
        std::string      extract_line();

        void save();
        void restore();
        void discard_saved();

        // reads the last token
        token_val get_last_token();

        void set_error(const token_val &err_msg);
        int  get_error_code() const { return error_msg.first; }

        std::string get_error_string() const { return error_msg.second; }
        std::string get_formatted_err_msg();
        bool eof();
        std::pair<int, int> get_error_pos() const { return lex.get_pos(); }

        void push_token(token_val tk);
        void push_token(const std::string &s);

        // returns all tokens collected so far
        std::vector<token_val> collect_tokens();

        // returns the last n tokens (in the same order they have been read)
        std::vector<token_val> collect_tokens(int n);
    };

    /// implementation dependent
    struct impl_rule;

    /// The action function which is passed the parser context
    typedef std::function< void(parser_context &)> action_t;

    /** The concrete rule class */
    class rule {
        /// Implementation 
        std::shared_ptr<impl_rule> pimpl;    
    public:
        /// An empty rule
        rule();
        /// Copy constructor
        rule(const rule &r); 

        /// A rule that matches a simple character. By default, this
        /// rule will NOT collect the character, unless the second
        /// parameter is set to true
        explicit rule(char c, bool collect = false);

        /// A rule that matches a string of characters.  / By default,
        /// this rule will NOT collect the character, unless the second
        /// parameter is set to true
        explicit rule(const std::string &s, bool collect = false);

        /// A rule that matches a token
        /// This rule will collect the token
        explicit rule(const token &tk);

        /// Assigmnent between rules
        rule &operator=(const rule &);
    
        /// Parses a rule
        bool parse(parser_context &pc) const;

        /// Sets an action for this rule
        rule& operator[](action_t af); 

        /// This constructor is not part of the interface, it is for
        /// internal use only!!  (However it must be public to not
        /// overcomplicate the implementation)
        explicit rule(std::shared_ptr<impl_rule> ir);
        std::shared_ptr<impl_rule> get_pimpl() { return pimpl; }

        // print a representation of the rule structure
        std::string print();
    };

    /** This creates a null rule (a rule that always matches without
     * consuming input) */
    rule null(); 

    /** Sequence of rules (with backtrack) */
    rule operator>>(rule &a, rule &b);
    rule operator>>(rule &&a, rule &b);
    rule operator>>(rule &a, rule &&b);
    rule operator>>(rule &&a, rule &&b);

    /** Sequence of rules (no backtrack) */
    rule operator>(rule &a, rule &b);
    rule operator>(rule &&a, rule &b);
    rule operator>(rule &a, rule &&b);
    rule operator>(rule &&a, rule &&b);

    /** Alternation of rules */
    rule operator|(rule &a, rule &b);
    rule operator|(rule &&a, rule &b);
    rule operator|(rule &a, rule &&b);
    rule operator|(rule &&a, rule &&b);

    /** Repetion of rules */
    rule operator*(rule &a);
    rule operator*(rule &&a);

    /** Optional rule: this is a shortcut for the alternation of an
     * empty rule and the rule a */
    rule operator-(rule &a);
    rule operator-(rule &&a);

    /** Extracts (collects) part of the text. The first parameter
     * represents the string which marks the start of the text
     * sequence, whereas the second parameters represents the closing
     * sequence. For example, in this way it is possible to skip
     * (remove from parsing) c-style comments */
    rule extract_rule(const std::string &op, const std::string &cl);
    /** Extracts (collects) part of the text. Unlike the previous
     * function, this one uses the same sequence for opening and
     * closing.*/
    rule extract_rule(const std::string &opcl);

    /** This extracts from the starting sequence (the initial
     * parameter) until the end of the line. Useful for C++ style
     * comments '//' */
    rule extract_line_rule(const std::string &opcl);

    /** Matches a given keyword. By default, the keyword is collected. */
    rule keyword(const std::string &key, bool collect = true);

    /** the global parsing function */
    bool parse_all(const rule &r, parser_context &pc);
}

#endif
