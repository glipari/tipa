#ifndef __PARSE_HPP__

#include <vector>
#include <lexer.hpp>
#include <memory>
#include <functional>

#define ERR_PARSE_SEQ   -100
#define ERR_PARSE_ALT   -101

/* 
   It contains the lexer and the last token that has been read, it is
   moved around the many rules in the parser
*/
class parser_context {
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

    token_val get_last_token();

    void set_error(const token_val &err_msg);
    int  get_error_code() const { return error_msg.first; }

    std::string get_error_string() const { return error_msg.second; }
    std::string get_formatted_err_msg();
    std::pair<int, int> get_error_pos() const { return lex.get_pos(); }

    void push_token(token_val tk);
    void push_token(const std::string &s);
    std::vector<token_val> collect_tokens();
};

/// implementation dependent
class impl_rule;

/// The action function which is passed the parser context
typedef std::function< void(parser_context &)> action_t;

/** The concrete rule class */
class rule {
    std::shared_ptr<impl_rule> pimpl;    
public:
    /// An empty rule
    rule();
    /// A copy constructor
    rule(const rule &r); 

    /// A rule that matches a simple character
    /// By default, this rule will NOT collect the character
    explicit rule(char c, bool collect = false);

    // A rule that matches a string of characters
    /// By default, this rule will NOT collect the character
    explicit rule(const std::string &s, bool collect = false);

    /// A rule that matches a token
    /// This rule will collect the token
    explicit rule(const token &tk);

    /// Assigmnent between rules
    rule &operator=(const rule &);
    
    /// Parses a rule
    bool parse(parser_context &pc);
    /// Sets an action for this rule
    rule& operator[](action_t af); 


    /// internal use only!!
    explicit rule(std::shared_ptr<impl_rule> ir);
    std::shared_ptr<impl_rule> get_pimpl() { return pimpl; }
};

/** Sequence of rules */
rule operator>>(rule a, rule b);

/** Alternation of rules */
rule operator|(rule a, rule b);

/** repetion of rules */
rule operator*(rule a);

/** extracting part of the text */
rule extract_rule(const std::string &op, const std::string &cl);
rule extract_rule(const std::string &opcl);
rule extract_line_rule(const std::string &opcl);

/* Matches a given keyword */
rule keyword(const std::string &key);

#endif
