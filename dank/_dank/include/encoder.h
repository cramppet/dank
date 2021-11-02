/*
 * Please see Appendix A of "Protocol Misidentification Made Easy with Format-Transforming Encryption"
 * url: http://dl.acm.org/citation.cfm?id=2516657
 *
 * and
 *
 * "Compression and ranking"
 * url: http://dl.acm.org/citation.cfm?id=22194
 *
 * for details about (un)ranking for regular languages.
 */


#ifndef _DFA_ENCODER_H
#define _DFA_ENCODER_H

#include <map>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
//#include <gmpxx.h>

#include <stdint.h>

#include "dfa.h"
#include "infint.h"

typedef std::vector<char> array_type_char_t1;
typedef std::vector<bool> array_type_bool_t1;
typedef std::vector<uint32_t> array_type_uint32_t1;
typedef std::vector< std::vector<uint32_t> > array_type_uint32_t2;
//typedef std::vector< std::vector<mpz_class> > array_type_mpz_t2;
typedef std::vector< std::vector<InfInt> > array_type_mpz_t2;
typedef std::vector< std::string > array_type_string_t1;

namespace py = pybind11;

class DFAEncoder {

private:
    // the maximum value for which buildTable is computed
    uint32_t _max_fixed_slice;
    uint32_t _fixed_slice;

    // our DFA start state
    uint32_t _start_state;

    // the number of states in our DFA
    uint32_t _num_states;

    // the number of symbols in our DFA alphabet
    uint32_t _num_symbols;

    // the symbols of our DFA alphabet
    array_type_uint32_t1 _symbols;

    // our mapping between integers and the symbols in our alphabet; ints -> chars
    std::map<uint32_t, char> _sigma;

    // the reverse mapping of sigma, chars -> ints
    std::map<char, uint32_t> _sigma_reverse;

    // the states in our DFA
    array_type_uint32_t1 _states;

    // our transitions table
    array_type_uint32_t2 _delta;

    // a lookup table used for additional performance gain
    // for each state we detect if all outgoing transitions are to the same state
    array_type_bool_t1 _delta_dense;

    // the set of final states in our DFA
    array_type_uint32_t1 _final_states;

    // buildTable builds a mapping from [q, i] -> n
    //   q: a state in our DFA
    //   i: an integer
    //   n: the number of words in our language that have a path to a final
    //      state that is exactly length i
    void _buildTable();

    // Checks the properties of our DFA, to ensure that we meet all constraints.
    // Throws an exception upon failure.
    void _validate();

    // _T is our cached table, the output of buildTable
    // For a state q and integer i, the value _T[q][i] is the number of unique
    // accepting paths of length exactly i from state q.
    array_type_mpz_t2 _T;

public:
    // Allows us to control the fixed slice value after DFA construction
    void set_fixed_slice(const uint32_t);
    uint32_t get_fixed_slice();

    uint32_t num_states();

    // The constructor of our rank/urank DFA class
    DFAEncoder( const std::string, const uint32_t );

    // our unrank function an int -> str mapping
    // given an integer i, return the ith lexicographically ordered string in
    // the language accepted by the DFA
    // std::string unrank( const mpz_class );
    std::string unrank( const InfInt );

    // our rank function performs the inverse operation of unrank
    // mpz_class rank( const std::string );
    InfInt rank( const std::string );

    // given integers [n,m] returns the number of words accepted by the
    // DFA that are at least length n and no greater than length m
    // mpz_class getNumWordsInLanguage( const uint32_t, const uint32_t );
    InfInt getNumWordsInLanguage( const uint32_t, const uint32_t );
};

class DFAEncoderPy {
private:
  DFAEncoder* myEncoder;
public:
  DFAEncoderPy(const std::string& regex, const uint32_t fixed_slice) {
    DFA myDFA = DFA::from_regex(regex);
    const std::string fst = myDFA.to_fst();
    myEncoder = new DFAEncoder(fst, fixed_slice);
  }

  ~DFAEncoderPy() {
    delete myEncoder;
  }

  py::bytes unrank(const py::int_ ranking) {
    py::object strfn = py::module::import("builtins").attr("str");    
    py::object tmp = strfn(ranking);
    std::string result = tmp.cast<std::string>();
    InfInt input = InfInt(result);
    return py::bytes(myEncoder->unrank(input));
  }

  py::int_ rank(const std::string& element) {
    return py::cast(myEncoder->rank(element).toString());
  }

  py::int_ num_words(const uint32_t lower, const uint32_t upper) {
    return py::cast(myEncoder->getNumWordsInLanguage(lower, upper).toString());
  }

  void set_fixed_slice(const uint32_t val) {
    myEncoder->set_fixed_slice(val);
  }

  py::int_ get_fixed_slice() {
    return py::cast(myEncoder->get_fixed_slice());
  }

  py::int_ num_states() {
    return py::cast(myEncoder->num_states());
  }
};

#endif /* _DFA_ENCODER_H */
