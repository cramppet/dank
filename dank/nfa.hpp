#include <map>
#include <set>
#include <vector>

using namespace std;

#ifndef NFA_H
#define NFA_H

struct DFA;

struct NFA {
  struct State {
    bool final;
    map<unsigned char, set<size_t>> trans;
  };

  set<size_t> init;
  vector<State> pool;

  template <class I> static NFA from_regex(I, I);

  void insert(size_t, unsigned char, size_t);
  void get_closure(set<size_t> &req) const;
  DFA determinize() const;

private:
  template <class I> void _from_regex(size_t, size_t, I, I);
  template <class I> void _preprocess(I, I);
};

#endif // NFA_H
