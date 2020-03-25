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
#include <iomanip>
#include "log_macros.hpp"

#include <regex>
#include <lexer.hpp>

using namespace std;

namespace tipa {
    parse_exc::parse_exc() 
    {}
    
    parse_exc::parse_exc(const string &err_msg) : msg(err_msg)
    {} 

    parse_exc::~parse_exc() throw()
    {} 

    string parse_exc::what() const
    {
        return msg;
    }

    token create_lib_token(const std::string &reg_ex)
    {
        static token_id index = LEX_LIB_BASE;
        return token(++index, reg_ex);
    }
    
    lexer::lexer() 
    {
    }

    ahead_lexer::ahead_lexer(const std::vector<token> &keys) : array(keys)
    {
    }

    void ahead_lexer::add_token(const token_id &name, const string &expr)
    {
        array.push_back(token(name, expr));
    }

    void lexer::set_stream(istream &in)
    {
        all_lines.clear();
        while (!saved_ctx.empty()) saved_ctx.pop();

        p_input = &in;
        nline = 0;
        ncol = 0;
	
        next_line();
    }

    void lexer::set_comment(const std::string &b, const std::string &e, const std::string &sl)
    {
        comment_begin = b;
        comment_end = e;
        comment_single_line = sl;
    }

    bool lexer::eof()
    {
        do {
            skip_spaces();
            if (start != curr_line.end()) return false;
        } while (next_line());
        return ((start == curr_line.end()) && (nline == all_lines.size()) && p_input->eof());
    }
    
    bool lexer::next_line()
    {
        if (nline == all_lines.size()) {
            if (p_input->eof()) {
                return false;
            }
            getline(*p_input, curr_line);
            all_lines.push_back(curr_line);
        } else {
            if (nline > all_lines.size())
                throw parse_exc("Lexer: exceeding all_lines array lenght!");
        }
    
        nline++;
        curr_line = all_lines[nline-1];
        ncol = 0;
        start = curr_line.begin();	
        return true;
    }

    void lexer::save() 
    {
        ctx c;
        c.nl = nline; 
        c.nc = ncol;
        if (start == curr_line.end()) c.dist = -1;
        else c.dist = std::distance(curr_line.begin(), start);

        saved_ctx.push(c);
    }

    void lexer::restore()
    {
        ctx c = saved_ctx.top();
        saved_ctx.pop();
        nline = c.nl;
        ncol = c.nc;
        curr_line = all_lines[nline-1];
        if (c.dist < 0) start = curr_line.end();
        else start = curr_line.begin() + c.dist;
    }

    void lexer::discard_saved()
    {
        saved_ctx.pop();
    }

    void lexer::advance_start(int n)
    {
        while (n>=1) {
            if (*start == '\t')
                ncol = (1+(ncol/8))*8;
            else ++ncol;
            ++start;
            //if (start == curr_line.end()) next_line();
            --n;
        }
    }
    
    
    bool lexer::skip_spaces()
    {
        while(true) {            
            if (start == curr_line.end()) {
                bool f = next_line();
                // if eof, return
                if (!f) return false;
            }
            else {
                int d = std::distance(start, curr_line.end());
                int m = std::min((int)comment_begin.size(), d);
                int n = std::min((int)comment_single_line.size(), d);
                std::string p(start, start+m);
                std::string q(start, start+n);
                
                if (*start == ' ' or *start == '\t') 
                    advance_start();
                else if (m != 0 and comment_begin == p) {
                    advance_start(m);
                    extract(comment_begin, comment_end);
                } 
                else if (n != 0 and comment_single_line == q)
                    extract_line();
                else break;
            }
        }
        return true;
        
        // while (start != curr_line.end()) {
        //     int d = std::distance(start, curr_line.end());
        //     int m = std::min((int)comment_begin.size(), d);
        //     int n = std::min((int)comment_single_line.size(), d);
        //     std::string p(start, start+m);
        //     std::string q(start, start+n);
	
        //     if (*start == ' ' or *start == '\t')
        //         advance_start();
        //     else if (m != 0 and comment_begin == p) {
        //         advance_start(m);
        //         extract(comment_begin, comment_end);
        //     } 
        //     else if (n != 0 and comment_single_line == q)
        //         extract_line();
        //     else break;
        // }
    }


    token_val lexer::try_token(const token &x)
    {
        static std::match_results<std::string::iterator> what;

        if (not skip_spaces()) return { LEX_ERROR, "EOF" }; 

        std::regex expr(x.get_expr());
        auto flag = std::regex_search(start, curr_line.end(), what, expr, 
                                      std::regex_constants::match_continuous);

        if (flag) {
            string res;
            copy(start, what[0].second, back_inserter(res));
            advance_start(distance(start, what[0].second));
            skip_spaces();
            return token_val(x.get_name(), res);
        }
        else return { LEX_ERROR, "Token does not match" };
    }

    std::pair<token_id, std::string> ahead_lexer::get_token()
    {
        static std::match_results<std::string::iterator> what;

        if (not skip_spaces()) return { LEX_ERROR, "EOF" }; 

        // try to identify which token
        for (auto x : array) {
            std::regex expr(x.get_expr());
            auto flag = std::regex_search(start, curr_line.end(), what, expr, 
                                          std::regex_constants::match_continuous);
            if (flag) {
                string res;
                copy(start, what[0].second, back_inserter(res));
                advance_start(distance(start, what[0].second));
                skip_spaces();
                return {x.get_name(), res};
            }
        }
        return { LEX_ERROR, "Unknown token" };
    }

    std::string lexer::extract_line()
    {
        std::string s(start, curr_line.end());
        next_line(); skip_spaces();
        return s;
    }

    std::string lexer::extract(const std::string &sym_begin, const std::string &sym_end)
    {
        std::string result;

        for (;;) {
            // move to the first non empty line
            while (start == curr_line.end()) {
                if (not next_line()) 
                    throw parse_exc("END OF INPUT WHILE EXTRACTING");
                result += '\n';
            }
            std::string s1(start, start + sym_begin.size() );
            std::string s2(start, start + sym_end.size() );
            if (sym_begin != "" and s1 == sym_begin) {
                result += s1;
                advance_start(sym_begin.size());
                result += extract(sym_begin, sym_end) + sym_end;
            } else if (s2 == sym_end) {
                advance_start(sym_end.size());
                //skip_spaces();
                return result;
            }
            else {
                result += *start;
                advance_start();
            }
        }
    }
}
