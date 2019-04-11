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
    /** 
        Helper functions to convert from a string to a variable of
        type T
    */
    inline void convert_to(const std::string &s, std::string& t) { t = s; }    
    inline void convert_to(const std::string &s, int &i) { i = std::stoi(s); }
    inline void convert_to(const std::string &s, float &f) { f = std::stof(s); }
    inline void convert_to(const std::string &s, double &d) { d = std::stod(s); }

    /** 
     * It contains the lexer and the last token that has been read,
     * that is the parser state during parsing. An object of this
     * class must be created by the user and passed to the parse()
     * method of the root rule. It is then passed around the rule tree
     * structure.
     */
    class parser_context {
    public:
        struct error_message {
            std::string msg;
            std::pair<int, int> position;
            token_val token;
            std::string line;
        };

    private:
        lexer lex;
        // the collected tokens
        std::vector<token_val> collected;

        std::stack<std::vector<token_val>> saved;
    
        //token_val error_msg;
        std::stack<error_message> error_stack;
        
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

        /// reads the last token
        token_val get_last_token();

        void set_error(const token_val &tk, const std::string &err_msg);
        void empty_error_stack() { while (!error_stack.empty()) error_stack.pop(); }
        error_message get_last_error() const { if (!error_stack.empty()) return error_stack.top(); else return error_message();}
        
        std::string get_error_string() const { if (!error_stack.empty()) return error_stack.top().token.second; else return "";}
        std::string get_formatted_err_msg();
        bool eof();

        void push_token(token_val tk);
        void push_token(const std::string &s);

        /// returns all tokens collected so far
        std::vector<token_val> collect_tokens();

        /// returns the last n tokens (in the same order they have been read)
        std::vector<token_val> collect_tokens(int n);

        /// writes the last n tokens (in the same order they have been
        /// read), applying a filter on it (the default filter
        /// extracts only the second element of the token_val pair)
        template<typename It, typename F=std::function<std::string(token_val)>>
        void collect_tokens(int n, It it, F fun=[](token_val tv) { return tv.second; }) {
            int s = collected.size();
            if (s < n) 
                throw std::string("too few parameters");
            
            auto p = begin(collected) + s - n;
            for(auto q = p; q != end(collected); q++) *(it++) = fun(*q);
            collected.erase(p, collected.end());
        }

        template<typename It, typename F=std::function<std::string(token_val)>>
        void collect_tokens(It it, F fun=[](token_val tv) { return tv.second; }) {
            auto p = begin(collected);
            for (auto q = p; q != end(collected); q++) *(it++) = fun(*q);
            collected.erase(p, collected.end());
        }

        std::string read_token() {
            if (collected.size() == 0) throw std::string("expecting a token");
            token_val tv = collected.back();
            collected.pop_back();
            return tv.second;
        }
    };


    /**
       Reads a token and updates a variable 
     */
    template<typename T>
    void read_all(parser_context &pc, T&& var)
    {
        convert_to(pc.read_token(), std::forward<T&>(var));
    }
    
    /**
       Reads a sequence of N tokens to update N variables
     */
    template<typename T, typename ...Args>
    void read_all(parser_context &pc, T&& var, Args&&...args)
    {
        read_all(pc, std::forward<Args&>(args)...);
        read_all(pc, std::forward<T&>(var));
    }
    
    /// forward declaration: implementation dependent
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
    
        /// Sets an action for this rule
        rule& set_action(action_t af);
                
        /// Installs a special action that reads a sequence of variables
        template<typename ...Args>
        rule & read_vars(Args&&... args) {
            set_action([&args...](auto &pc) { read_all(pc, std::forward<Args&>(args)...); });
            return *this;
        }

        /// Parses a rule
        bool parse(parser_context &pc) const;

        /* -------------------------- */
        
        /// This constructor is not part of the interface, it is for
        /// internal use only!!  (However it must be public to not
        /// overcomplicate the implementation)
        explicit rule(std::shared_ptr<impl_rule> ir);
        std::shared_ptr<impl_rule> get_pimpl() { return pimpl; }

        /// print a representation of the rule structure (for debugging)
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

    /** creates a rule that parses a list of elements */
    rule list_rule(rule &&r, const std::string &sep = ",");

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
