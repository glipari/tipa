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

//#define __LOG__ 1
#include "log_macros.hpp"
#include "tinyparser.hpp"
#include "wptr.hpp"

#include <sstream>
#include <set>
#include <algorithm>

#ifdef __LOG__
int abs_counter=0;
#define INC_COUNT do { ++abs_counter; } while(0)
#define DEC_COUNT do { --abs_counter; } while(0)
#else
#define INC_COUNT do {} while(0) 
#define DEC_COUNT do {} while(0) 
#endif

namespace tipa {
    //-----------------------------------------
    /* Implementation of Parser Context

       This is a simple data structure that contains the state of the
       parser.  In fact, the state is not stored in the rules, but in
       this context that is passed around the rules and updated accordingly.
    */
    parser_context::parser_context() : lex{}
    {}
    
    void parser_context::set_stream(std::istream &in)
    {
        lex.set_stream(in);
        collected.clear();
        //saved.clear();
        while (!saved.empty()) saved.pop();
        //while (!ncoll.empty()) ncoll.pop();
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
        collected.push_back({tk_extracted.get_name(), s});
    }

    void parser_context::save() 
    {
        lex.save();
        saved.push(collected);
    }

    void parser_context::restore()
    {
        lex.restore();
        if (saved.size() < 1) throw "parser_context::restore() on an empty stack !!!" ;
        collected = saved.top();
        saved.pop();
    }
 
    void parser_context::discard_saved()
    {
        lex.discard_saved();
        //ncoll.pop();
        if (saved.size() < 1) throw "parser_context::discard_saved() on an empty stack !!!" ;
        saved.pop();
    }

    token_val parser_context::get_last_token()
    {
        if (collected.size() < 1) throw "parser_context::get_last_token(): there is no token!!";
        else return collected[collected.size()-1];
    }


    std::vector<token_val> parser_context::collect_tokens()
    {
        auto c = collected;
        collected.clear();
        return c;
    }

    // returns the last n tokens (in the same order they have been read)
    std::vector<token_val> parser_context::collect_tokens(int n)
    {
        std::vector<token_val> v;
        for (int i=0; i<n; i++) {
            if (collected.size() == 0) break;
            v.push_back(collected.back());
            collected.pop_back();
        }
        std::reverse(std::begin(v), std::end(v));
        return v;
    }
        
    void parser_context::set_error(const token_val &err_msg)
    {
        error_msg = err_msg;
    }

    bool parser_context::eof()
    {
        return lex.eof();
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

    /* Here we describe the design of the implementation. 
       
       We have two levels of classes/objects for implementing the rule
       class.  

       +------------+
       | rule       |      +-------------+ 
       |            |      |  impl_rule  |      +-----------+
       |       * ---+----->|             |      | abs_rule  |
       |            |      |       * ----+----->|           |
       +------------+      |             |      |           |
                           +-------------+      |           |
                                                +-----------+
 
       1) struct impl_rule: the rule class contains a shared pointer to this
       structure. This class is not polymorphic. It contains a shared
       pointer to an abs_rule.

       2) A polymorphic family of classes, with root in abs_rule. 
       These classes contain the actual parsing code for the different rules. 
       A abs_rule class can be:
       - a term-rule: this represents a leaf in the tree, and the parsing is done by the lexer
       - a seq_rule: contains a vector of pointers to struct impl_rule (all must be true) 
       - a alt_rule: contains a vector of pointers to struct impl_rule (at least one must be true)
       - a rep_rule: contains one shared pointer to a struct impl_rule

       therefore:

       rule --> impl_rule --> abs_rule        

       (all --> arrows are shared pointers).

       One example of recursive rule: 
       rule sum;
       rule expr = sum | null_rule();
       sum = rule(tk_int) >> rule('+') >> expr ; 

       In this case we have

       expr --> impl_rule(expr) --> alt_rule --> null_rule
       ^              |
       |              v
       |             impl_rule(sum) --> seq_rule --> term_rule(tk_int)
       |                                  | 
       |                                  v
       |                                impl_rule
       |                                  |
       |                                  v
       +------------------------------- seq_rule --> term_rule('+')
                                                         
       
       Notice that there is a cycle. If all pointers were shared_prt,
       we run into a memory leak when we destroy expr.  So, the
       strategy is the following: 

       - if a rule is created from a rvalue (for example, by rules
       built on the fly), then it takes ownership of the impl_rule
       by using a shared_ptr;
      
       - if a rule is created from a lvalue (for example, by rules
       built on the stack, like rule sum in the example above), then
       it uses a weak_ptr and does not take ownership.

       So, be careful when building a rule from other rules; make sure
       that they stay alive after construction; if you want to force
       ownership (because you know you are going to destroy the rule)
       use std::move() to transform a lvalue into a rvalue and force
       the rule to use a shared_ptr.
    */


    /** 
        this is used to implement the print method (used for debugging)
    */
    struct impl_rule;
    typedef std::set<impl_rule *> av_set;

    /** 
        The abstract class for the implementation.
    */
    class abs_rule {
    protected:
        action_t fun;
    public:
        abs_rule() : fun(nullptr) { INC_COUNT; }
        virtual bool parse(parser_context &pc) const = 0;
        bool action(parser_context &pc);
        virtual ~abs_rule() { DEC_COUNT; }
        virtual std::string print(av_set &already_visited) {return std::string("");}

        void install_action(action_t);
    };

    void abs_rule::install_action(action_t f)
    {
        fun = f;
    }

    bool abs_rule::action(parser_context &pc)
    {
        if (fun) {
            INFO_LINE("-- action found");
            fun(pc);
            INFO_LINE("-- action completed");
        }
        return true;
    }

    /** The implementation structure.  It contains a pointer to the
     *  REAL implementation.
     */
    struct impl_rule {
        std::shared_ptr<abs_rule> abs_impl;

        impl_rule() : abs_impl(nullptr) {}
        impl_rule(abs_rule *r) : abs_impl(r) {}
    
        bool parse(parser_context &pc) const {
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
        bool collect;
    public:
        term_rule(const token &tk, bool c = true) : mytoken(tk), collect(c) {}
        virtual bool parse(parser_context &pc) const;
        std::string print(av_set &av) {
            return std::string("TERM: ") + mytoken.get_expr(); 
        }
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

    std::string rule::print()
    {
        av_set av;
        return pimpl->abs_impl->print(av);
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

    rule::rule(char c, bool collect)
    {
        std::string p{c};
        p = padding(p);
        token tk = {tk_char.get_name(), p};
        pimpl = std::make_shared<impl_rule>(new term_rule(tk, collect));
    }

    rule::rule(const std::string &s, bool collect)
    {
        std::string p = padding(s);
        token tk = {tk_char.get_name(), p};
        pimpl = std::make_shared<impl_rule>(new term_rule(tk, collect));
    }

    rule::rule(const token &tk) : pimpl(new impl_rule(new term_rule(tk, true)))
    {
    }

    rule & rule::operator=(const rule &r) 
    {
        pimpl->abs_impl = r.pimpl->abs_impl;
        return *this;
    }

    bool rule::parse(parser_context &pc) const
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

    bool term_rule::parse(parser_context &pc) const
    {
        INFO_LINE("term_rule::parse() trying " << mytoken.get_expr());
        token_val result = pc.try_token(mytoken);
        INFO_LINE("term_rule::parse() completed on " << result.first);
        if (result.first == mytoken.get_name()) {
            INFO_LINE(" ** ok");
            if (collect) pc.push_token(result);
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
    protected:
        std::vector< WPtr<impl_rule> > rl;
    public:

        seq_rule(rule &a, rule &b); 
        seq_rule(rule &&a, rule &b); 
        seq_rule(rule &a, rule &&b);
        seq_rule(rule &&a, rule &&b);
 
        virtual bool parse(parser_context &pc) const;
        std::string print(av_set &av);
    };

/* ----------------------------------------------- */

    seq_rule::seq_rule(rule &a, rule &b)
    {
        rl.push_back(WPtr<impl_rule>(a.get_pimpl(), WPTR_WEAK));
        rl.push_back(WPtr<impl_rule>(b.get_pimpl(), WPTR_WEAK));
    }

    seq_rule::seq_rule(rule &&a, rule &b)
    {
        rl.push_back(WPtr<impl_rule>(a.get_pimpl(), WPTR_STRONG));
        rl.push_back(WPtr<impl_rule>(b.get_pimpl(), WPTR_WEAK));
    }

    seq_rule::seq_rule(rule &a, rule &&b)
    {
        rl.push_back(WPtr<impl_rule>(a.get_pimpl(), WPTR_WEAK));
        rl.push_back(WPtr<impl_rule>(b.get_pimpl(), WPTR_STRONG));
    }

    seq_rule::seq_rule(rule &&a, rule &&b)
    {
        rl.push_back(WPtr<impl_rule>(a.get_pimpl(), WPTR_STRONG));
        rl.push_back(WPtr<impl_rule>(b.get_pimpl(), WPTR_STRONG));
    }

    bool seq_rule::parse(parser_context &pc) const
    {
        INFO("seq_rule::parse()");

        int i = 0;
        pc.save();
        for (auto &x : rl) {
            if (auto spt = x.get()) {
                if (!spt->parse(pc)) {
                    if (pc.get_error_string() == "EOF" && i == 0) return false; 
                    else {
                        pc.set_error({ERR_PARSE_SEQ, "Wrong element in sequence"});
                        INFO_LINE(" ** FALSE ");
                        pc.restore();
                        return false;
                    }
                }
                i++;
            }
            else {
                throw parse_exc("seq_parse: weak pointer error!");
            }
        }
        // everything is ok, return true
        pc.discard_saved();
        INFO_LINE(" ** ok ");
        return true;
    }

    std::string seq_rule::print(av_set &av) 
    {
        std::string s("(SEQ: ");
        for (auto &x : rl)
            if (auto spt = x.get()) {
                if (av.find(spt.get()) == av.end()) {
                    av.insert(spt.get());
                    s += spt->abs_impl->print(av) + "(Weak) >> ";	
                }
                else s+= "[visited]";
            }
        return s + ")\n";
    }


    rule operator>>(rule &a, rule &b)
    {
        INFO_LINE("seq operator: &a and &b");
        auto s = std::make_shared<impl_rule>(new seq_rule(a,b));
        return rule(s);
    }

    rule operator>>(rule &&a, rule &b)
    {
        INFO_LINE("seq operator: &&a and &b");
        auto s = std::make_shared<impl_rule>(new seq_rule(std::move(a),b));
        return rule(s);
    }

    rule operator>>(rule &a, rule &&b)
    {
        INFO_LINE("seq operator: &a and &&b");
        auto s = std::make_shared<impl_rule>(new seq_rule(a,std::move(b)));
        return rule(s);
    }

    rule operator>>(rule &&a, rule &&b)
    {
        INFO_LINE("seq operator: &&a and &&b");
        auto s = std::make_shared<impl_rule>(new seq_rule(std::move(a),std::move(b)));
        return rule(s);
    }

/* -------------------------------------------- */

/*
  An alternation of rules. One of the rules in the alternation list
  must be matched
*/
    class alt_rule : public abs_rule {
        std::vector< WPtr<impl_rule> > rl;
        // std::vector< std::weak_ptr<impl_rule> > wl;
    public:
        alt_rule(rule &a, rule &b);
        alt_rule(rule &&a, rule &b);
        alt_rule(rule &a, rule &&b);
        alt_rule(rule &&a, rule &&b);

        virtual bool parse(parser_context &pc) const;
        virtual std::string print(av_set &av);
    };

    alt_rule::alt_rule(rule &a, rule &b)
    {
        rl.push_back(WPtr<impl_rule>(a.get_pimpl(), WPTR_WEAK));
        rl.push_back(WPtr<impl_rule>(b.get_pimpl(), WPTR_WEAK));
    }

    alt_rule::alt_rule(rule &&a, rule &b)
    {
        rl.push_back(WPtr<impl_rule>(a.get_pimpl(), WPTR_STRONG));
        rl.push_back(WPtr<impl_rule>(b.get_pimpl(), WPTR_WEAK));
    }

    alt_rule::alt_rule(rule &a, rule &&b)
    {
        rl.push_back(WPtr<impl_rule>(a.get_pimpl(), WPTR_WEAK));
        rl.push_back(WPtr<impl_rule>(b.get_pimpl(), WPTR_STRONG));
    }

    alt_rule::alt_rule(rule &&a, rule &&b)
    {
        rl.push_back(WPtr<impl_rule>(a.get_pimpl(), WPTR_STRONG));
        rl.push_back(WPtr<impl_rule>(b.get_pimpl(), WPTR_STRONG));
    }

    bool alt_rule::parse(parser_context &pc) const
    {
        INFO("alt_rule::parse() | ");
        for (auto &x : rl)
            if (auto spt = x.get()) {
                if (spt->parse(pc)) {
                    INFO_LINE(" ** ok");
                    return true;
                }
            }
            else {
                throw parse_exc("alt_rule: undefined weak pointer");
            }

        pc.set_error({ERR_PARSE_ALT, "None of the alternatives parsed correctly"});
        INFO_LINE(" ** FALSE");
        return false;
    }


    std::string alt_rule::print(av_set &av) {
        std::string s ("(ALT : ");

        for (auto &x : rl)
            if (auto spt = x.get()) {
                if (av.find(spt.get()) == av.end()) {
                    av.insert(spt.get());
                    s += spt->abs_impl->print(av) + " >> ";	
                }
                else s+= "[visited]";
            }
        return s + ")\n";
    }

    rule operator|(rule &a, rule &b)
    {
        INFO_LINE("alt operator: &a and &b");
        auto s = std::make_shared<impl_rule>(new alt_rule(a,b));
        return rule(s);
    }

    rule operator|(rule &&a, rule &b)
    {
        INFO_LINE("alt operator: &&a and &b");
        auto s = std::make_shared<impl_rule>(new alt_rule(std::move(a),b));
        return rule(s);
    }

    rule operator|(rule &a, rule &&b)
    {
        INFO_LINE("alt operator: &a and &&b");

        auto s = std::make_shared<impl_rule>(new alt_rule(a,std::move(b)));
        return rule(s);
    }

    rule operator|(rule &&a, rule &&b)
    {
        INFO_LINE("alt operator: &&a and &&b");

        auto s = std::make_shared<impl_rule>(new alt_rule(std::move(a),std::move(b)));
        return rule(s);
    }

    /** Null rule */

    class null_rule : public abs_rule {
    public:
        null_rule() {}
        virtual bool parse(parser_context &pc) const;
    };

    
    bool null_rule::parse(parser_context &pc) const
    {
        return true;
    }

/* ------------------------------------------- */

/*
  A repetition of zero or one instances of a rule
*/
    class rep_rule : public abs_rule {
        WPtr<impl_rule> rl;
    public:
        rep_rule(rule &a);
        rep_rule(rule &&a);

        virtual bool parse(parser_context &pc) const;
        virtual std::string print(av_set &av);
    };

    rep_rule::rep_rule(rule &a) : rl(WPtr<impl_rule>(a.get_pimpl(), WPTR_WEAK))
    {
    }

    rep_rule::rep_rule(rule &&a) : rl(WPtr<impl_rule>(a.get_pimpl(), WPTR_STRONG))
    {
    }


    /**
       I have a problem here.  What if the last instance failed to
       parse ?  

       1) this is indeed an error: there is nothing else to do, and
       part of the file could not be parsed correctly, so I have to
       point out this problem

       2) We encounter an EOF before parsing, so we are done with it,
       parsing is completed.

       3) Last part was not parsed correctly, but this is not a
       problem, because another rule follows which will correctly get
       it.

       Case 2 can be identified easily. Case 1 is difficult to
       distinguish from Case 3. It depends on what we do after: do we
       reparse the erroneous part or not?

       For example:

       abc abc abc 123

       if the grammar is *rule(tk_ident), the above is an error if the
       grammar is *rule(tk_ident) >> rule(tk_int), then no error.

       I guess the only way to understand if this is an error is to
       look at what happens after we parsed the whole file. Did we
       reach EOF ? If so, then OK. If not, then ERROR. 

       So, I will do a global function parse_all(rule) that invokes
       the rule.parse() and then checks if the whole file was parsed
       correctly. If so, then returns ok, otherwise it is an error.

       And the repetion rule just returns true ALWAYS. 
     */
    bool rep_rule::parse(parser_context &pc) const
    {
        INFO("rep_rule::parse() | ");
        if (rl.get()) {
            while (rl.get()->parse(pc)) INFO("*");
            INFO(" end ");
            // if (pc.get_error_string() != "EOF" && i == 0) return false;
            // else return true;
        }
        else {
            throw parse_exc("rep_rule: unvalid weak pointer");
        }
        return true;
    }

    std::string rep_rule::print(av_set &av) 
    {
        std::string s = "(REP :";
        if (rl.get()) {
            if (av.find(rl.get().get()) == av.end()) {
                av.insert(rl.get().get());
                s += rl.get()->abs_impl->print(av) + "(strong) * ";   
            }
            else {
                s += "[visited]";
            }
        }
        else s+=" <unvalid> ";
        return s + ")\n";
    }

    rule operator*(rule &a) 
    {
        auto s = std::make_shared<impl_rule>(new rep_rule(a));
        return rule(s);    
    }

    rule operator*(rule &&a) 
    {
        auto s = std::make_shared<impl_rule>(new rep_rule(std::move(a)));
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
        bool parse(parser_context &pc) const {
            INFO("extr_rule::parse()");
            token open_tk = {tk_char.get_name(), padding(open_sym)};
            if (pc.try_token(open_tk).first == tk_char.get_name()) {
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
        bool collect_flag;
    public:
        keyword_rule(const std::string &key, bool collect) : kw(key), rl(tk_ident, true), collect_flag(collect) {}

        virtual bool parse(parser_context &pc) const;
        virtual std::string print(av_set &av);
    };

    bool keyword_rule::parse(parser_context &pc) const
    {
        pc.save();
        bool flag = rl.parse(pc);
        
        if (flag && pc.get_last_token().second == kw) {
            pc.discard_saved();
            // if I do not wish to collect this token, I remove it from the pc. 
            if (!collect_flag) pc.collect_tokens(1);
            return true;
        }
        else {
            pc.restore();
            return false;
        }
    }

    std::string keyword_rule::print(av_set &av)
    {
        return std::string("TERM: ") + kw;
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

    rule keyword(const std::string &key, bool collect)
    {
        auto s = std::make_shared<impl_rule>(new keyword_rule(key, collect));
        return rule(s);
    }

/* 
   A sequence of rules to be evaluated in order. 
   I expect that they match one after the other. 
*/
    // class strict_seq_rule : public seq_rule {
    // public:

    //     strict_seq_rule(rule &a, rule &b);
    //     strict_seq_rule(rule &&a, rule &b);
    //     strict_seq_rule(rule &a, rule &&b);
    //     strict_seq_rule(rule &&a, rule &&b);

    //     virtual bool parse(parser_context &pc) const; 
    // };

    // strict_seq_rule::strict_seq_rule(rule &a, rule &b) : seq_rule(a, b)
    // {
    // }

    // strict_seq_rule::strict_seq_rule(rule &&a, rule &b) : seq_rule(std::move(a), b)
    // {
    // }

    // strict_seq_rule::strict_seq_rule(rule &a, rule &&b) : seq_rule(a, std::move(b))
    // {
    // }

    // strict_seq_rule::strict_seq_rule(rule &&a, rule &&b) : seq_rule(std::move(a), std::move(b))
    // {
    // }

    // bool strict_seq_rule::parse(parser_context &pc) const
    // {
    //     INFO("strict_seq_rule::parse()");

    //     int i = 0;
    //     for (auto &x : rl) {
    //         if (auto spt = x.get()) {
    //             if (!spt->parse(pc)) {
    //                 pc.set_error({ERR_PARSE_SEQ, "Wrong element in sequence"});
    //                 INFO_LINE(" ** FALSE ");
    //                 if (i==0) return false;
    //                 else throw parse_exc("Error in strict sequence (after first)");
    //             }
    //             i++;
    //         }
    //         else {
    //             throw parse_exc("strict_seq_rule: unvalid weak pointer");
    //         }
    //     }    

    //     INFO_LINE(" ** ok ");
    //     return true;
    // }

    // rule operator>(rule &a, rule &b)
    // {
    //     INFO_LINE("sseq operator: &a and &b");

    //     auto s = std::make_shared<impl_rule>(new strict_seq_rule(a,b));
    //     return rule(s);
    // }
    // rule operator>(rule &&a, rule &b)
    // {
    //     INFO_LINE("sseq operator: &&a and &b");
    //     auto s = std::make_shared<impl_rule>(new strict_seq_rule(std::move(a),b));
    //     return rule(s);
    // }
    // rule operator>(rule &a, rule &&b)
    // {
    //     INFO_LINE("sseq operator: &a and &&b");

    //     auto s = std::make_shared<impl_rule>(new strict_seq_rule(a,std::move(b)));
    //     return rule(s);
    // }
    // rule operator>(rule &&a, rule &&b)
    // {
    //     INFO_LINE("sseq operator: &&a and &&b");
    //     auto s = std::make_shared<impl_rule>(new strict_seq_rule(std::move(a),std::move(b)));
    //     return rule(s);
    // }

    rule null()
    {
        auto s = std::make_shared<impl_rule>(new null_rule);
        return rule(s);
    }


    rule operator-(rule &a)
    {
        auto s = std::make_shared<impl_rule>(new alt_rule(a, null()));
        return rule(s);
    }

    rule operator-(rule &&a)
    {
        auto s = std::make_shared<impl_rule>(new alt_rule(std::move(a), null()));
        return rule(s);
    }


    bool parse_all(const rule &r, parser_context &pc)
    {
        bool f = r.parse(pc);
        bool e = pc.eof();
        // std::cout << "parse_all() = " << f << "   eof = " << e << std::endl;
        if (!e) return false;
        else return f;
    }


}


/*
  Error handling

  One possibility is to define an error descriptor which contains

  - The last token that has been tried
  - The position of the failure
  - An error message 

  This error descriptor is saved on a stack every time we have an error.
  The stack would be cleaned when we can proceed. 

  The only rule that can cause such a problem is the alternation. So,
  it is the alternation that must clean the stack if one good
  alternative if found.
*/
