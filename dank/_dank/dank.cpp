#include <iostream>
#include <pybind11/pybind11.h>

#include <dfa.h>
#include <nfa.h>
#include <infint.h>
#include <encoder.h>

namespace py = pybind11;
using namespace std;


PYBIND11_MODULE(_dank, m) {
    py::class_<DFA>(m, "DFA")
        .def(py::init())
        .def("from_regex", &DFA::from_regex)
        .def("to_fst", &DFA::to_fst)
        .def("to_dot", &DFA::to_dot);
    py::class_<DFAEncoderPy>(m, "DFAEncoder")
        .def(py::init<>())
        .def("rank", &DFAEncoderPy::rank)
        .def("unrank", &DFAEncoderPy::unrank)
        .def("num_words", &DFAEncoderPy::num_words)
        .def("get_fixed_slice", &DFAEncoderPy::get_fixed_slice)
        .def("set_fixed_slice", &DFAEncoderPy::set_fixed_slice)
        .def("num_states", &DFAEncoderPy::num_states)
        .def("from_regex", &DFAEncoderPy::from_regex)
        .def("from_fst", &DFAEncoderPy::from_fst);
}
