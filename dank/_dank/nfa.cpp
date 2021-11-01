#include <iostream>
#include <cassert>
#include <queue>
#include <string>

using namespace std;

#include <dfa.h>
#include <nfa.h>

void NFA::insert(size_t s, unsigned char c, size_t t) {
  assert(s < pool.size());
  pool[s].trans[c].insert(t);
}

void NFA::get_closure(set<size_t> &req) const {
  const NFA &nfa = *this;
  queue<size_t> q;
  for (auto i = req.begin(); i != req.end(); i++)
    q.push(*i);
  while (q.size()) {
    size_t u = q.front();
    q.pop();
    auto x = nfa.pool[u].trans.find(0);
    if (x == nfa.pool[u].trans.end())
      continue;
    for (auto i = x->second.begin(); i != x->second.end(); i++)
      if (req.find(*i) == req.end()) {
        req.insert(*i);
        q.push(*i);
      }
  }
}

// Thompson's construction algorithm BEGIN

template <class I> NFA NFA::from_regex(I lo, I hi) {
  NFA nfa;
  nfa.pool.clear();
  nfa.pool.push_back(NFA::State());
  nfa.pool.push_back(NFA::State());
  nfa.pool[1].final = 1;
  nfa.init.clear();
  nfa.init.insert(0);
  nfa._from_regex(0, 1, lo, hi);
  nfa.get_closure(nfa.init);
  return nfa;
}
template NFA NFA::from_regex(const char *, const char *);
template NFA NFA::from_regex(string::const_iterator, string::const_iterator);

template <class I> void NFA::_from_regex(size_t s, size_t t, I lo, I hi) {
  NFA &nfa = *this;
  if (hi - lo == 1) {
    nfa.insert(s, *lo, t);
    return;
  }
  if (hi - lo == 2 && *lo == '\\') {
    nfa.insert(s, *(lo + 1), t);
    return;
  }
  I option(lo), concatenation(lo);
  size_t _ = 0;
  for (I i = lo; i != hi; ++i)
    switch (*i) {
    case '\\':
      if (!_)
        concatenation = i;
      ++i;
      break;
    case '(':
      if (!_)
        concatenation = i;
      ++_;
      break;
    case ')':
      assert(_);
      --_;
      break;
    case '|':
      if (!_)
        option = i;
      break;
    case '?':
      break;
    case '*':
      break;
    case '+':
      break;
    default:
      if (!_)
        concatenation = i;
    }
  assert(_ == 0);
  if (option != lo) {
    size_t i0 = nfa.pool.size(), i1 = nfa.pool.size() + 1;
    nfa.pool.push_back(NFA::State());
    nfa.pool.push_back(NFA::State());
    nfa.insert(s, 0, i0);
    nfa.insert(i1, 0, t);
    _from_regex(i0, i1, lo, option);
    i0 = nfa.pool.size(), i1 = nfa.pool.size() + 1;
    nfa.pool.push_back(NFA::State());
    nfa.pool.push_back(NFA::State());
    nfa.insert(s, 0, i0);
    nfa.insert(i1, 0, t);
    _from_regex(i0, i1, option + 1, hi);
  } else if (concatenation != lo) {
    size_t i0 = nfa.pool.size(), i1 = nfa.pool.size() + 1;
    nfa.pool.push_back(NFA::State());
    nfa.pool.push_back(NFA::State());
    nfa.insert(i0, 0, i1);
    _from_regex(s, i0, lo, concatenation);
    _from_regex(i1, t, concatenation, hi);
  } else if (*(hi - 1) == '?') {
    size_t i0 = nfa.pool.size(), i1 = nfa.pool.size() + 1;
    nfa.pool.push_back(NFA::State());
    nfa.pool.push_back(NFA::State());
    nfa.insert(s, 0, i0);
    nfa.insert(s, 0, t);
    nfa.insert(i1, 0, t);
    _from_regex(i0, i1, lo, hi - 1);
  } else if (*(hi - 1) == '*') {
    size_t i0 = nfa.pool.size(), i1 = nfa.pool.size() + 1;
    nfa.pool.push_back(NFA::State());
    nfa.pool.push_back(NFA::State());
    nfa.insert(s, 0, i0);
    nfa.insert(s, 0, t);
    nfa.insert(i1, 0, i0);
    nfa.insert(i1, 0, t);
    _from_regex(i0, i1, lo, hi - 1);
  } else if (*(hi - 1) == '+') {
    size_t i0 = nfa.pool.size(), i1 = nfa.pool.size() + 1;
    nfa.pool.push_back(NFA::State());
    nfa.pool.push_back(NFA::State());
    nfa.insert(i0, 0, i1);
    _from_regex(s, i0, lo, hi - 1);
    s = i1;
    i0 = nfa.pool.size(), i1 = nfa.pool.size() + 1;
    nfa.pool.push_back(NFA::State());
    nfa.pool.push_back(NFA::State());
    nfa.insert(s, 0, i0);
    nfa.insert(s, 0, t);
    nfa.insert(i1, 0, i0);
    nfa.insert(i1, 0, t);
    _from_regex(i0, i1, lo, hi - 1);
  } else {
    assert(*lo == '(' && *(hi - 1) == ')');
    _from_regex(s, t, lo + 1, hi - 1);
  }
}
template void NFA::_from_regex(size_t, size_t, const char *, const char *);
template void NFA::_from_regex(size_t, size_t, string::const_iterator,
                               string::const_iterator);

// Thompson's construction algorithm END

// Powerset construction
DFA NFA::determinize() const {
  const NFA &nfa = *this;
  DFA dfa;
  map<set<size_t>, size_t> m;
  queue<set<size_t>> q;
  vector<bool> inQ;
  dfa.init = 0;
  dfa.pool.clear();
  dfa.pool.push_back(DFA::State());
  for (auto i = nfa.init.begin(); i != nfa.init.end(); i++)
    if (nfa.pool[*i].final) {
      dfa.pool[0].final = 1;
      break;
    }
  m[nfa.init] = 0;
  q.push(nfa.init);
  inQ.push_back(1);
  while (q.size()) {
    set<size_t> u0 = q.front();
    size_t u1 = m[u0];
    map<unsigned char, set<size_t>> _;
    q.pop();
    inQ[u1] = 0;
    for (auto i = u0.begin(); i != u0.end(); ++i)
      for (auto j = nfa.pool[*i].trans.upper_bound(0);
           j != nfa.pool[*i].trans.end(); j++)
        _[j->first].insert(j->second.begin(), j->second.end());
    for (auto i = _.begin(); i != _.end(); i++) {
      nfa.get_closure(i->second);
      auto __ = m.find(i->second);
      if (__ == m.end()) {
        size_t v1 = dfa.pool.size();
        dfa.pool.push_back(DFA::State());
        dfa.insert(u1, i->first, v1);
        m[i->second] = v1;
        q.push(i->second);
        inQ.push_back(1);
        for (auto j = i->second.begin(); j != i->second.end(); j++)
          if (nfa.pool[*j].final) {
            dfa.pool.back().final = 1;
            break;
          }
      } else
        dfa.insert(u1, i->first, __->second);
    }
  }
  return dfa;
}
