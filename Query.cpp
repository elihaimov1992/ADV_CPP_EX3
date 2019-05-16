#include "Query.h"
#include "TextQuery.h"
#include <memory>
#include <set>
#include <algorithm>
#include <iostream>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <regex>
using namespace std;
////////////////////////////////////////////////////////////////////////////////
std::shared_ptr<QueryBase> QueryBase::factory(const string& s)
{
  // Example: if(s == "smart") return std::shared_ptr<QueryBase>(new WordQuery("smart"));

  // All Regex:
  regex regex_and(R"(^\\s+(\\w+)\\s+AND\\s+(\\w+)\\s*$)");
  regex regex_or(R"(^\\s+(\\w+)\\s+OR\\s+(\\w+)\\s*$)");
  regex regex_n(R"(^\\s+(\\w+)\\s+\(d+)\\s+(\\w+)\\s*$)");
  regex regex_not(R"(^\\s*NOT\\s+(\\w+)\\s*$)");
  regex regex_word((R"(^\\s*(\\w+)\\s*$)"));

  // AND case:
  auto words_begin = sregex_iterator(s.begin(), s.end(), regex_and);
  auto words_end = sregex_iterator();
  smatch Match = *words_begin;
  if(distance(words_begin,words_end) > 0)
  {
    return std::shared_ptr<QueryBase>(new AndQuery(Match.str(1),Match.str(2)));
  }

  // OR case:
  auto words_begin = sregex_iterator(s.begin(), s.end(), regex_or);
  auto words_end = sregex_iterator();
  smatch Match = *words_begin;
  if(distance(words_begin,words_end) > 0)
  {
    return std::shared_ptr<QueryBase>(new OrQuery(Match.str(1),Match.str(2)));
  }

  // n case:
  auto words_begin = sregex_iterator(s.begin(), s.end(), regex_n);
  auto words_end = sregex_iterator();
  smatch Match = *words_begin;
  if(distance(words_begin,words_end) > 0)
  {
    return std::shared_ptr<QueryBase>(new NQuery(Match[1].str(),Match[3].str(),stoi(Match.str(2))));
  }

  // NOT case:
  auto words_begin = sregex_iterator(s.begin(), s.end(), regex_not);
  auto words_end = sregex_iterator();
  smatch Match = *words_begin;
  if(distance(words_begin,words_end) > 0)
  {
    return std::shared_ptr<QueryBase>(new NotQuery(Match[1].str()));
  }

  // word case:
  auto words_begin = sregex_iterator(s.begin(), s.end(), regex_word);
  auto words_end = sregex_iterator();
  smatch Match = *words_begin;
  if(distance(words_begin,words_end) > 0)
  {
    return std::shared_ptr<QueryBase>(new WordQuery(Match[1].str()));
  }

  // else
  cout << "Unrecognized search" << endl;
  
}
////////////////////////////////////////////////////////////////////////////////
QueryResult NotQuery::eval(const TextQuery &text) const
{
  QueryResult result = text.query(query_word);
  auto ret_lines = std::make_shared<std::set<line_no>>();
  auto beg = result.begin(), end = result.end();
  auto sz = result.get_file()->size();
  
  for (size_t n = 0; n != sz; ++n)
  {
    if (beg==end || *beg != n)
		ret_lines->insert(n);
    else if (beg != end)
		++beg;
  }
  return QueryResult(rep(), ret_lines, result.get_file());
    
}

QueryResult AndQuery::eval (const TextQuery& text) const
{
  QueryResult left_result = text.query(left_query);
  QueryResult right_result = text.query(right_query);
  
  auto ret_lines = std::make_shared<std::set<line_no>>();
  std::set_intersection(left_result.begin(), left_result.end(),
      right_result.begin(), right_result.end(), 
      std::inserter(*ret_lines, ret_lines->begin()));

  return QueryResult(rep(), ret_lines, left_result.get_file());
}

QueryResult OrQuery::eval(const TextQuery &text) const
{
  QueryResult left_result = text.query(left_query);
  QueryResult right_result = text.query(right_query);
  
  auto ret_lines = 
      std::make_shared<std::set<line_no>>(left_result.begin(), left_result.end());

  ret_lines->insert(right_result.begin(), right_result.end());

  return QueryResult(rep(), ret_lines, left_result.get_file());
}
/////////////////////////////////////////////////////////
QueryResult NQuery::eval(const TextQuery &text) const
{

}
/////////////////////////////////////////////////////////