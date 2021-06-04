#ifndef DFA_H
#define DFA_H

#include <map>
#include <vector>

using namespace std;

struct NFA;

struct DFA {
  struct State {
    bool final;
    map<unsigned char, size_t> trans;
  };

  size_t init;
  vector<State> pool;

  static DFA from_regex(const string&);

  void insert(size_t, unsigned char, size_t);
  string to_dot() const;
  string to_fst() const;
  NFA reverse() const;
};

#endif // DFA_H
