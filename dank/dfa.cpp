#include <cassert>
#include <sstream>
#include "dfa.hpp"
#include "nfa.hpp"
#include "infint.hpp"

using namespace std;

void DFA::insert(size_t s, unsigned char c, size_t t) {
  assert(s < pool.size());
  assert(pool[s].trans.find(c) == pool[s].trans.end());
  pool[s].trans[c] = t;
}

string DFA::to_dot() const {
  stringstream ss;
  ss << "digraph {" << endl;
  ss << "  node[shape=circle];" << endl;
  ss << "  edge[arrowhead=vee];" << endl;
  ss << "  START[shape=point, color=white];" << endl;
  for (size_t i = 0; i < pool.size(); i++)
    if (pool[i].final)
      ss << "  " << i << "[shape=doublecircle];" << endl;
  ss << "  START -> " << init << " [label=start];" << endl;
  for (size_t i = 0; i < pool.size(); i++)
    for (auto j = pool[i].trans.begin(); j != pool[i].trans.end(); j++)
      ss << "  " << i << " -> " << j->second << " [label=\"\\" << j->first
         << "\"];" << endl;
  ss << "}" << endl;
  return ss.str();
}

string DFA::to_fst() const {
  stringstream ss;
  for (size_t i = 0; i < pool.size(); i++) {
    for (auto j = pool[i].trans.begin(); j != pool[i].trans.end(); j++) {
      ss << i << "\t" << j->second << "\t" << (int)j->first << "\t" << (int)j->first << endl;
    }
    if (pool[i].final)
      ss << i << endl;
  }
  return ss.str();
}

NFA DFA::reverse() const {
  NFA nfa;
  const DFA &dfa = *this;
  nfa.init.clear();
  nfa.pool.assign(dfa.pool.size(), NFA::State());
  for (size_t i = 0; i < dfa.pool.size(); i++) {
    for (auto j = dfa.pool[i].trans.begin(); j != dfa.pool[i].trans.end(); j++)
      nfa.insert(j->second, j->first, i);
    if (dfa.pool[i].final)
      nfa.init.insert(i);
  }
  nfa.pool[dfa.init].final = 1;
  return nfa;
}

// Brzozowski's algorithm
//template <class I> DFA DFA::from_regex(I lo, I hi) {
DFA DFA::from_regex(const string &regex) {
  return move(NFA::from_regex(regex.begin(), regex.end())
                  .determinize()
                  .reverse()
                  .determinize()
                  .reverse()
                  .determinize());
}
