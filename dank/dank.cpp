#include <iostream>
#include <pybind11/pybind11.h>

#include <dfa.h>
#include <nfa.h>
#include <infint.h>
#include <encoder.h>

namespace py = pybind11;
using namespace std;


PYBIND11_MODULE(dank, m) {
    py::class_<DFA>(m, "DFA")
        .def(py::init())
        .def("from_regex", &DFA::from_regex)
        .def("to_fst", &DFA::to_fst)
        .def("to_dot", &DFA::to_dot);
    py::class_<DFAEncoderPy>(m, "DFAEncoder")
        .def(py::init<const string&, const uint32_t>())
        .def("rank", &DFAEncoderPy::rank)
        .def("unrank", &DFAEncoderPy::unrank)
        .def("num_words", &DFAEncoderPy::num_words);
}
