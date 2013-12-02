//#define __LOG__ 1
#include <log_macros.hpp>

#include "parser.hpp"

#include <sstream>

parser_context::parser_context() : lex{}
{}

void parser_context::set_stream(std::istream &in)
{
    lex.set_stream(in);
}

void parser_context::set_comment(const std::string &comment_begin, 
				 const std::string &comment_end,
				 const std::string &comment_single_line)
{
    lex.set_comment(comment_begin, comment_end, comment_single_line);
}


token_val parser_context::try_token(const token &tk)
{
    return lex.try_token(tk);
}

std::string parser_context::extract(const std::string &op, const std::string &cl)
{
    return lex.extract(op, cl);
}

std::string parser_context::extract_line()
{
    return lex.extract_line();
}

void parser_context::push_token(token_val tk)
{
    collected.push_back(tk);
}

void parser_context::push_token(const std::string &s)
{
    collected.push_back({LEX_EXTRACTED_STRING, s});
}


void parser_context::save() 
{
    lex.save();
    ncoll.push(collected.size());
}

void parser_context::restore()
{
    lex.restore();
    unsigned lev = ncoll.top();
    ncoll.pop();
    while (lev != collected.size()) collected.pop_back();
}
 
void parser_context::discard_saved()
{
    lex.discard_saved();
    ncoll.pop();
}

token_val parser_context::get_last_token()
{
    return collected[collected.size()-1];
}


std::vector<token_val> parser_context::collect_tokens()
{
    auto c = collected;
    collected.clear();
    return c;
}

void parser_context::set_error(const token_val &err_msg)
{
    error_msg = err_msg;
}

std::string parser_context::get_formatted_err_msg()
{
    std::stringstream err;
    err << "At line " << lex.get_pos().first 
	<< ", column " << lex.get_pos().second << std::endl;
    err << lex.get_currline() << std::endl;    
    for (int i=0; i<lex.get_pos().second-1; ++i) err << " ";
    err << "^" << std::endl;
    err << "Error code: " << error_msg.first << std::endl;
    err << "Error msg : " << error_msg.second << std::endl; 
    return err.str();
}

/* ----------------------------------------------- */

/** 
    The abstract class
*/
class abs_rule {
protected:
    action_t fun;
public:
    abs_rule() : fun(nullptr) {}
    virtual bool parse(parser_context &pc) = 0;
    bool action(parser_context &pc);
    virtual ~abs_rule() {}

    void install_action(action_t);
};

void abs_rule::install_action(action_t f)
{
    fun = f;
}

bool abs_rule::action(parser_context &pc)
{
    INFO_LINE("abs_rule::action()");
    if (fun) {
	INFO_LINE("-- action found");
	fun(pc);
	INFO_LINE("-- action completed");
    }
    return true;
}

/**
   The actual implementation. 
   It contains a pointer to the actual implementation. 
 */
struct impl_rule {
    std::shared_ptr<abs_rule> abs_impl;

    impl_rule() : abs_impl(nullptr) {}
    impl_rule(abs_rule *r) : abs_impl(r) {}
    
    bool parse(parser_context &pc) {
	if (!abs_impl) return false;

	bool f = abs_impl->parse(pc); 
	if (f) abs_impl->action(pc);
	return f;
    }
    bool action(parser_context &pc) {
	if (!abs_impl) return false;
	return abs_impl->action(pc);
    }
    void install_action(action_t f) {
	abs_impl->install_action(f);
    }
    
};

/* ----------------------------------------------- */

class term_rule : public abs_rule {
    token mytoken;
public:
    term_rule(const token &tk) : mytoken(tk) {}
    virtual bool parse(parser_context &pc);
};

/* ----------------------------------------------- */

rule::rule() : pimpl(new impl_rule())
{
}

rule::rule(const rule &r) : 
    pimpl(r.pimpl)
{
}

rule::rule(std::shared_ptr<impl_rule> ir) : pimpl(ir)
{
}

static std::string padding(const std::string &p)
{
    static std::string elements{".[{}()\\*+?|^$"};
    std::string r;
    for (auto c : p) {
	if (elements.find_first_of(c) != std::string::npos) 
	    r.append("\\");
	r.append(1, c);
    }
    INFO_LINE("PADDING RESULTS: " << r);
    return r;
}

rule::rule(char c)
{
    std::string p{c};
    p = padding(p);
    token tk = {LEX_CHAR, p};
    pimpl = std::make_shared<impl_rule>(new term_rule(tk));
}

rule::rule(const std::string &s)
{
    std::string p = padding(s);
    token tk = {LEX_CHAR, p};
    pimpl = std::make_shared<impl_rule>(new term_rule(tk));
}

rule::rule(const token &tk) : pimpl(new impl_rule(new term_rule(tk)))
{
}

rule & rule::operator=(const rule &r) 
{
    pimpl->abs_impl = r.pimpl->abs_impl;
    return *this;
}

bool rule::parse(parser_context &pc) 
{ 
    bool f = pimpl->parse(pc); 
    return f;
}

rule& rule::operator[](action_t af)
{
    pimpl->install_action(af);
    INFO_LINE("Action installed");
    return *this;
}


bool term_rule::parse(parser_context &pc)
{
    INFO("term_rule::parse() trying " << mytoken.get_expr());
    token_val result = pc.try_token(mytoken);
    if (result.first == mytoken.get_name()) {
	INFO_LINE(" ** ok");
	pc.push_token(result);
	return true;
    } else {
	INFO_LINE(" ** FALSE");
	pc.set_error(result);
	return false;
    }
}

/* ----------------------------------------------- */

/* 
   A sequence of rules to be evaluated in order. 
   I expect that they match one after the other. 
*/
class seq_rule : public abs_rule {
    std::vector< std::shared_ptr<impl_rule> > rl;
public:
    seq_rule(rule a, rule b); 

    virtual bool parse(parser_context &pc);
};

/* ----------------------------------------------- */

seq_rule::seq_rule(rule a, rule b)
{
    rl.push_back(a.get_pimpl());
    rl.push_back(b.get_pimpl());
}

bool seq_rule::parse(parser_context &pc)
{
    INFO("seq_rule::parse(), curr-token: " << pc.get_token().second << " | ");

    pc.save();
    for (auto &x : rl) {
	if (!x->parse(pc)) {
	    // TODO, better error is necessary!
	    pc.set_error({ERR_PARSE_SEQ, "Wrong element in sequence"});
	    INFO_LINE(" ** FALSE ");
	    pc.restore();
	    return false;
	}
    }    
    INFO_LINE(" ** ok ");
    return true;
}

rule operator>>(rule a, rule b)
{
    auto s = std::make_shared<impl_rule>(new seq_rule(a,b));
    return rule(s);
}

/* -------------------------------------------- */

/*
  An alternation of rules. One of the rules in the alternation list
  must be matched
 */
class alt_rule : public abs_rule {
    std::vector< std::shared_ptr<impl_rule> > rl;
public:
    alt_rule(rule a, rule b);

    virtual bool parse(parser_context &pc);
};

alt_rule::alt_rule(rule a, rule b)
{
    rl.push_back(a.get_pimpl());
    rl.push_back(b.get_pimpl());
}

bool alt_rule::parse(parser_context &pc)
{
    INFO("alt_rule::parse(), curr-token: " << pc.get_token().second << " | ");
    for (auto &x : rl)
	if (x->parse(pc)) {
	    INFO_LINE(" ** ok");
	    return true;
	}
    pc.set_error({ERR_PARSE_ALT, "None of the alternatives parsed correctly"});
    INFO_LINE(" ** FALSE");
    return false;
}

rule operator|(rule a, rule b)
{
    auto s = std::make_shared<impl_rule>(new alt_rule(a,b));
    return rule(s);
}

/* ------------------------------------------- */

/*
  A repetition of zero or one instances of a rule
 */
class rep_rule : public abs_rule {
    std::shared_ptr<impl_rule> rl;
public:
    rep_rule(rule a);

    virtual bool parse(parser_context &pc);
};

rep_rule::rep_rule(rule a) : rl(a.get_pimpl())
{
}

bool rep_rule::parse(parser_context &pc)
{
    INFO("rep_rule::parse(), curr-token: " << pc.get_token().second << " | ");

    while (rl->parse(pc)) {
	INFO("*");
    }
    INFO(" end ");
    return true;	
}


rule operator*(rule a) 
{
    auto s = std::make_shared<impl_rule>(new rep_rule(a));
    return rule(s);    
}

class extr_rule : public abs_rule {
    std::string open_sym;
    std::string close_sym;
    bool nested;
    bool line;
public:
    extr_rule(const std::string &op, const std::string &cl) :
	open_sym(op), close_sym(cl), nested(true), line(false)
	{}
    extr_rule(const std::string &op_cl, bool l = false) :
	open_sym(op_cl), close_sym(op_cl), nested(false), line(l)
	{}
    bool parse(parser_context &pc) {
	INFO("extr_rule::parse()");
	token open_tk = {LEX_CHAR, padding(open_sym)};
	if (pc.try_token(open_tk).first == LEX_CHAR) {
	    if (line) {
		pc.push_token(pc.extract_line());
		INFO_LINE(" ** ok");
		//pc.next_token();
		return true; 
	    }
	    std::string o = "";
	    if (nested) o = open_sym;
	    pc.push_token(pc.extract(o, close_sym));
	    INFO_LINE(" ** ok");
	    //pc.next_token();
	    return true;
	}
	else {
	    INFO_LINE(" ** FALSE");
	    return false;
	}
    }
};

class keyword_rule : public abs_rule {
    std::string kw;
    term_rule rl;
public:
    keyword_rule(const std::string &key) : kw(key), rl(tk_ident) {}

    virtual bool parse(parser_context &pc);
};

bool keyword_rule::parse(parser_context &pc)
{
    pc.save();
    bool flag = rl.parse(pc);
    if (flag && pc.get_last_token().second == kw) {
	pc.discard_saved();
	return true;
    }
    else {
	pc.restore();
	return false;
    }
}


rule extract_rule(const std::string &op, const std::string &cl)
{
    auto s = std::make_shared<impl_rule>(new extr_rule(op, cl));
    return rule(s);
}

rule extract_rule(const std::string &opcl)
{
    auto s = std::make_shared<impl_rule>(new extr_rule(opcl));
    return rule(s);
}

rule extract_line_rule(const std::string &opcl)
{
    auto s = std::make_shared<impl_rule>(new extr_rule(opcl, true));
    return rule(s);
}

rule keyword(const std::string &key)
{
    auto s = std::make_shared<impl_rule>(new keyword_rule(key));
    return rule(s);
}
