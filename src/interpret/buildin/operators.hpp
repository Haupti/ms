#pragma once
#include "../../location.hpp"
#include "../../msl_runtime_error.hpp"
#include "../constants.hpp"
#include "../value_and_heap.hpp"

inline Value mslbuildin_op_neq(ILHeap *heap_ptr, const Value &leftval,
                               const Value &rightval) {
  switch (leftval.tag) {
  case ValueTag::INT:
    switch (rightval.tag) {
    case ValueTag::INT:
      if (leftval.as.INT == rightval.as.INT) {
        return Value::Symbol(_SYM_FALSE);
      } else {
        return Value::Symbol(_SYM_TRUE);
      }
    case ValueTag::FLOAT:
      if (leftval.as.INT == rightval.as.FLOAT) {
        return Value::Symbol(_SYM_FALSE);
      } else {
        return Value::Symbol(_SYM_TRUE);
      }
    default:
      return Value::Symbol(_SYM_TRUE);
    }
  case ValueTag::STRING:
    switch (rightval.tag) {
    case ValueTag::STRING:
      if (heap_ptr->get_string(leftval.as.STRING) ==
          heap_ptr->get_string(rightval.as.STRING)) {
        return Value::Symbol(_SYM_FALSE);
      } else {
        return Value::Symbol(_SYM_TRUE);
      }
    default:
      return Value::Symbol(_SYM_TRUE);
    }
  case ValueTag::FLOAT:
    switch (rightval.tag) {
    case ValueTag::INT:
      if (leftval.as.FLOAT == rightval.as.INT) {
        return Value::Symbol(_SYM_FALSE);
      } else {
        return Value::Symbol(_SYM_TRUE);
      }
    case ValueTag::FLOAT:
      if (leftval.as.FLOAT == rightval.as.FLOAT) {
        return Value::Symbol(_SYM_FALSE);
      } else {
        return Value::Symbol(_SYM_TRUE);
      }
    default:
      return Value::Symbol(_SYM_TRUE);
    }
  case ValueTag::SYMBOL:
    switch (rightval.tag) {
    case ValueTag::SYMBOL:
      if (leftval.as.SYMBOL.index == rightval.as.SYMBOL.index) {
        return Value::Symbol(_SYM_FALSE);
      } else {
        return Value::Symbol(_SYM_TRUE);
      }
    default:
      return Value::Symbol(_SYM_TRUE);
    }
  case ValueTag::LIST:
    if (rightval.tag == ValueTag::LIST && leftval.as.LIST == rightval.as.LIST) {
      return Value::Symbol(_SYM_FALSE);
    } else {
      return Value::Symbol(_SYM_TRUE);
    }
  case ValueTag::NONE:
    if (rightval.tag == ValueTag::NONE) {
      return Value::Symbol(_SYM_FALSE);
    } else {
      return Value::Symbol(_SYM_TRUE);
    }
  case ValueTag::ERROR:
    if (rightval.tag != ValueTag::ERROR) {
      return Value::Symbol(_SYM_TRUE);
    } else {
      return mslbuildin_op_neq(heap_ptr, heap_ptr->at(leftval.as.ERROR),
                               heap_ptr->at(rightval.as.ERROR));
    }
  }
}

inline Value mslbuildin_op_eq(ILHeap *heap_ptr, const Value &leftval,
                              const Value &rightval) {
  switch (leftval.tag) {
  case ValueTag::INT:
    switch (rightval.tag) {
    case ValueTag::INT:
      if (leftval.as.INT == rightval.as.INT) {
        return Value::Symbol(_SYM_TRUE);
      } else {
        return Value::Symbol(_SYM_FALSE);
      }
    case ValueTag::FLOAT:
      if (leftval.as.INT == rightval.as.FLOAT) {
        return Value::Symbol(_SYM_TRUE);
      } else {
        return Value::Symbol(_SYM_FALSE);
      }
    default:
      return Value::Symbol(_SYM_FALSE);
    }
  case ValueTag::STRING:
    switch (rightval.tag) {
    case ValueTag::STRING:
      if (heap_ptr->get_string(leftval.as.STRING) ==
          heap_ptr->get_string(rightval.as.STRING)) {
        return Value::Symbol(_SYM_TRUE);
      } else {
        return Value::Symbol(_SYM_FALSE);
      }
    default:
      return Value::Symbol(_SYM_FALSE);
    }
  case ValueTag::FLOAT:
    switch (rightval.tag) {
    case ValueTag::INT:
      if (leftval.as.FLOAT == rightval.as.INT) {
        return Value::Symbol(_SYM_TRUE);
      } else {
        return Value::Symbol(_SYM_FALSE);
      }
    case ValueTag::FLOAT:
      if (leftval.as.FLOAT == rightval.as.FLOAT) {
        return Value::Symbol(_SYM_TRUE);
      } else {
        return Value::Symbol(_SYM_FALSE);
      }
    default:
      return Value::Symbol(_SYM_FALSE);
    }
  case ValueTag::SYMBOL:
    switch (rightval.tag) {
    case ValueTag::SYMBOL:
      if (leftval.as.SYMBOL.index == rightval.as.SYMBOL.index) {
        return Value::Symbol(_SYM_TRUE);
      } else {
        return Value::Symbol(_SYM_FALSE);
      }
    default:
      return Value::Symbol(_SYM_FALSE);
    }
  case ValueTag::LIST:
    if (rightval.tag == ValueTag::LIST && leftval.as.LIST == rightval.as.LIST) {
      return Value::Symbol(_SYM_TRUE);
    } else {
      return Value::Symbol(_SYM_FALSE);
    }
  case ValueTag::NONE:
    if (rightval.tag == ValueTag::NONE) {
      return Value::Symbol(_SYM_TRUE);
    } else {
      return Value::Symbol(_SYM_FALSE);
    }
  case ValueTag::ERROR:
    if (rightval.tag != ValueTag::ERROR) {
      return Value::Symbol(_SYM_FALSE);
    } else {
      return mslbuildin_op_eq(heap_ptr, heap_ptr->at(leftval.as.ERROR),
                              heap_ptr->at(rightval.as.ERROR));
    }
  }
}

inline Value mslbuildin_op_str_concat(ILHeap *heap_ptr, LocationRef where,
                                      const Value &leftval,
                                      const Value &rightval) {
  if (leftval.tag != ValueTag::STRING || rightval.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "'<>' expected two strings");
  }
  ILHIDX str_idx =
      heap_ptr->add_string(heap_ptr->get_string(leftval.as.STRING) +
                           heap_ptr->get_string(rightval.as.STRING));
  return heap_ptr->at(str_idx);
}

inline Value mslbuildin_op_gt(LocationRef where, const Value &leftval,
                              const Value &rightval) {
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Int(leftval.as.INT > rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.INT > rightval.as.FLOAT);
    default:
      throw msl_runtime_error(where, "'>' expected two number arguments");
    }
  } break;

  case ValueTag::FLOAT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(leftval.as.FLOAT > rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.FLOAT > rightval.as.FLOAT);
    default:
      throw msl_runtime_error(where, "'>' expected two number arguments");
    }
  } break;
  default:
    throw msl_runtime_error(where, "'>' expected two number arguments");
  }
}

inline Value mslbuildin_op_gte(LocationRef where, const Value &leftval,
                               const Value &rightval) {
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Int(leftval.as.INT >= rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.INT >= rightval.as.FLOAT);
    default:
      throw msl_runtime_error(where, "'>=' expected two number arguments");
    }
  } break;

  case ValueTag::FLOAT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(leftval.as.FLOAT >= rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.FLOAT >= rightval.as.FLOAT);
    default:
      throw msl_runtime_error(where, "'>=' expected two number arguments");
    }
  } break;
  default:
    throw msl_runtime_error(where, "'>' expected two number arguments");
  }
}

inline Value mslbuildin_op_lt(LocationRef where, const Value &leftval,
                              const Value &rightval) {
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Int(leftval.as.INT < rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.INT < rightval.as.FLOAT);
    default:
      throw msl_runtime_error(where, "'<' expected two number arguments");
    }
  } break;

  case ValueTag::FLOAT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(leftval.as.FLOAT < rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.FLOAT < rightval.as.FLOAT);
    default:
      throw msl_runtime_error(where, "'<' expected two number arguments");
    }
  } break;
  default:
    throw msl_runtime_error(where, "'<' expected two number arguments");
  }
}
inline Value mslbuildin_op_lte(LocationRef where, const Value &leftval,
                               const Value &rightval) {
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Int(leftval.as.INT <= rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.INT <= rightval.as.FLOAT);
    default:
      throw msl_runtime_error(where, "'<=' expected two number arguments");
    }
  } break;

  case ValueTag::FLOAT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(leftval.as.FLOAT <= rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.FLOAT <= rightval.as.FLOAT);
    default:
      throw msl_runtime_error(where, "'<=' expected two number arguments");
    }
  } break;
  default:
    throw msl_runtime_error(where, "'<=' expected two number arguments");
  }
}
inline Value mslbuildin_op_mod(LocationRef where, const Value &leftval,
                               const Value &rightval) {
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Int(leftval.as.INT % rightval.as.INT);
    default:
      throw msl_runtime_error(where, "'mod' expected two integer arguments");
    }
  } break;
  default:
    throw msl_runtime_error(where, "'mod' expected two number arguments");
  }
}
inline Value mslbuildin_op_mul(LocationRef where, const Value &leftval,
                               const Value &rightval) {
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Int(leftval.as.INT * rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.INT * rightval.as.FLOAT);
    default:
      throw msl_runtime_error(where, "'*' expected two number arguments");
    }
  } break;

  case ValueTag::FLOAT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(leftval.as.FLOAT * rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.FLOAT * rightval.as.FLOAT);
    default:
      throw msl_runtime_error(where, "'*' expected two number arguments");
    }
  } break;
  default:
    throw msl_runtime_error(where, "'*' expected two number arguments");
  }
}
inline Value mslbuildin_op_div(LocationRef where, const Value &leftval,
                               const Value &rightval) {
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(static_cast<double>(leftval.as.INT) /
                          static_cast<double>(rightval.as.INT));
    case ValueTag::FLOAT:
      return Value::Float(static_cast<double>(leftval.as.INT) /
                          rightval.as.FLOAT);
    default:
      throw msl_runtime_error(where, "'/' expected two number arguments");
    }
  } break;

  case ValueTag::FLOAT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(leftval.as.FLOAT /
                          static_cast<double>(rightval.as.INT));
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.FLOAT / rightval.as.FLOAT);
    default:
      throw msl_runtime_error(where, "'/' expected two number arguments");
    }
  } break;
  default:
    throw msl_runtime_error(where, "'/' expected two number arguments");
  }
}
inline Value mslbuildin_op_add(LocationRef where, const Value &leftval,
                               const Value &rightval) {
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Int(leftval.as.INT + rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.INT + rightval.as.FLOAT);
    default:
      throw msl_runtime_error(where, "'+' expected two number arguments");
    }
  } break;

  case ValueTag::FLOAT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(leftval.as.FLOAT + rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.FLOAT + rightval.as.FLOAT);
    default:
      throw msl_runtime_error(where, "'+' expected two number arguments");
    }
  } break;
  default:
    throw msl_runtime_error(where, "'+' expected two number arguments");
  }
}
inline Value mslbuildin_op_sub(LocationRef where, const Value &leftval,
                               const Value &rightval) {
  switch (leftval.tag) {
  case ValueTag::INT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Int(leftval.as.INT - rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.INT - rightval.as.FLOAT);
    default:
      throw msl_runtime_error(where, "'-' expected two number arguments");
    }
  } break;

  case ValueTag::FLOAT: {
    switch (rightval.tag) {
    case ValueTag::INT:
      return Value::Float(leftval.as.FLOAT - rightval.as.INT);
    case ValueTag::FLOAT:
      return Value::Float(leftval.as.FLOAT - rightval.as.FLOAT);
    default:
      throw msl_runtime_error(where, "'-' expected two number arguments");
    }
  } break;
  default:
    throw msl_runtime_error(where, "'-' expected two number arguments");
  }
}
inline Value mslbuildin_op_invers(LocationRef where, const Value &val) {
  switch (val.tag) {
  case ValueTag::INT:
    return Value::Int(-1 * val.as.INT);
  case ValueTag::FLOAT:
    return Value::Float(-1 * val.as.FLOAT);
  default:
    throw msl_runtime_error(where, "'-' expected a number");
  }
}
