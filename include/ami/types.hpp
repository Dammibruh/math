#include <memory>
#include <variant>
#include <vector>

#include "ast.hpp"

namespace ami {
using val_t = std::variant<Number, Boolean, NullExpr, IntervalExpr, UnionExpr,
                           InterSectionExpr, SetObject, Vector, Matrix, Point,
                           std::string>;
using arg_t = std::vector<val_t>;
using ptr_t = std::shared_ptr<Expr>;
using iscope_t = std::map<std::string, val_t>;
using fscope_t = std::map<std::string, Function>;
}  // namespace ami
