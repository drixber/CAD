#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(cadursor, module) {
    module.doc() = "CADursor Python bindings";
    module.attr("__version__") = "0.1.0";
}
