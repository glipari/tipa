//#define __LOG__ 1
#include <iomanip>
#include "log_macros.hpp"

#include <boost/regex.hpp>
#include <tipa/lexer.hpp>

using namespace std;

namespace tipa {
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

    void lexer::next_line()
    {
	if (nline == all_lines.size()) {
	    curr_line = "";
	    getline(*p_input, curr_line);
	    all_lines.push_back(curr_line);
	} else if (nline > all_lines.size()) 
	    throw "Lexer: exceeding all_lines array lenght!";
    
	nline++;
	curr_line = all_lines[nline-1];
	ncol = 0;
	start = curr_line.begin();	
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

    void lexer::skip_spaces()
    {
	while (start != curr_line.end()) {
	    int d = std::distance(start, curr_line.end());
	    int m = std::min((int)comment_begin.size(), d);
	    int n = std::min((int)comment_single_line.size(), d);
	    std::string p(start, start+m);
	    std::string q(start, start+n);
	
	    if (*start == ' ' or *start == '\n') {
		++start; ++ncol;
	    } else if (*start == '\t') {
		++start; ncol += 8;
	    } else if (m != 0 and comment_begin == p) {
		start += m;
		extract(comment_begin, comment_end);
	    } 
	    else if (n != 0 and comment_single_line == q)
		extract_line();
	    else break;
	}
    }


    std::pair<token_id, std::string> lexer::try_token(const token &x)
    {
	static boost::match_results<std::string::iterator> what;
	skip_spaces(); 

	while (start == curr_line.end() or *start == 0) {
	    if (p_input->eof())
		return { LEX_ERROR, "EOF" };
	    next_line();
	    skip_spaces();
	}

	INFO_LINE("try_token(): start is at \"" << *start << "\"");

	INFO_LINE(curr_line);
	INFO_LINE(std::setw(ncol) << "^");

	boost::regex expr(x.get_expr());
	auto flag = boost::regex_search(start, curr_line.end(), what, expr, 
					boost::match_continuous);
	if (flag) {
	    string res;
	    copy(start, what[0].second, back_inserter(res));
	    ncol += distance(start, what[0].second);
	    start = what[0].second;
	    return {x.get_name(), res};
	}
	return { LEX_ERROR, "Token does not match" };
    }

    std::pair<token_id, std::string> ahead_lexer::get_token()
    {
	static boost::match_results<std::string::iterator> what;

	skip_spaces(); 

	while (start == curr_line.end() or *start == 0) {
	    if (p_input->eof())
		return { LEX_ERROR, "EOF" };
	    next_line();
	    skip_spaces();
	}

	INFO_LINE("get_token(): start is at \"" << *start << "\"");
	INFO_LINE(curr_line);
	INFO_LINE(std::setw(ncol) << "^");

	// try to identify which token
	for (auto x : array) {
	    boost::regex expr(x.get_expr());
	    auto flag = boost::regex_search(start, curr_line.end(), what, expr, 
					    boost::match_continuous);
	    if (flag) {
		string res;
		copy(start, what[0].second, back_inserter(res));
		ncol += distance(start, what[0].second);
		start = what[0].second;
		// INFO_LINE("get_token(): start now pointing at \"" << *start << "\"");
		// INFO_LINE("token: " << res);
		return {x.get_name(), res};
		break;
	    }
	}
	INFO_LINE("token not found");
	return { LEX_ERROR, "Unknown token" };
    }

    std::string lexer::extract_line()
    {
	std::string s(start, curr_line.end());
	next_line();
	return s;
    }

    std::string lexer::extract(const std::string &sym_begin, const std::string &sym_end)
    {
	std::string result;

	for (;;) {
	    while (start == curr_line.end()) {
		if (p_input->eof()) 
		    throw "END OF INPUT WHILE EXTRACTING";
		next_line();
		result += "\n";
	    }
	    std::string s1(start, start + sym_begin.size() );
	    std::string s2(start, start + sym_end.size() );
	    if (sym_begin != "" and s1 == sym_begin) {
		result += s1;
		start += sym_begin.size();
		result += extract(sym_begin, sym_end) + sym_end;
	    } else if (s2 == sym_end) {
		start += sym_end.size();
		return result;
	    }
	    else {
		result += *start;
		start++;
	    }
	}
    }
}
