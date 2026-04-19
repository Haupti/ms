#include "core.hpp"
#include "../../lib/asap/util.hpp"
#include "../msl_runtime_error.hpp"
#include "../vendor/cpp-httplib/cpp_httplib.h"
#include "../vendor/picojson/picojson.h"
#include "core_utils.hpp"
#include "stack.hpp"
#include "vm.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <optional>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <thread>

#ifdef _WIN32
#include <io.h>
#define ISATTY _isatty
#define STDOUT_FILENO 1
#else
#include <unistd.h>
#define ISATTY isatty
#endif

namespace {

namespace chrono = std::chrono;
std::vector<std::string> msl_args;

Value response_to_table(VM *vm, const httplib::Result &res) {
  VMHIDX response_table = vm->heap.new_table();
  core_utils::table_add_entry(&vm->heap, response_table,
                              Value::FromSymbol(create_symbol("#status")),
                              Value::Int(static_cast<int64_t>(res->status)));
  core_utils::table_add_entry(&vm->heap, response_table,
                              Value::FromSymbol(Constants::SYM_BODY),
                              vm->heap.add_string(res->body));

  // Headers
  VMHIDX headers_table = vm->heap.new_table();
  for (const auto &header : res->headers) {
    core_utils::table_add_entry(&vm->heap, headers_table,
                                vm->heap.add_string(header.first),
                                vm->heap.add_string(header.second));
  }
  core_utils::table_add_entry(&vm->heap, response_table,
                              Value::FromSymbol(Constants::SYM_HEADERS),
                              vm->heap.at(headers_table));

  // Optional redirect field
  if (res->status >= 300 && res->status < 400 && res->has_header("Location")) {
    core_utils::table_add_entry(
        &vm->heap, response_table,
        Value::FromSymbol(create_symbol("#redirect")),
        vm->heap.add_string(res->get_header_value("Location")));
  }

  return vm->heap.at(response_table);
}

Value table_get_internal(VMHeap *heap, VMHIDX table_idx, Value key) {
  VMHIDX idx = heap->nth_child_idx(table_idx, 0);
  while (idx != INVALID) {
    Value current_key = heap->nth_child(idx, 0);
    if (core::values_equal(heap, key, current_key)) {
      return heap->nth_child(idx, 1);
    }
    idx = heap->next_child(idx);
  }
  return Value::None();
}

void map_msl_table_to_response(LocationRef where, VM *vm, Value msl_res,
                               httplib::Response &res) {
  if (msl_res.tag != ValueTag::TABLE) {
    throw msl_runtime_error(
        where, "http server request handler did not return a table");
  }

  VMHIDX table_idx = msl_res.as.TABLE;

  // Status
  Value status_val = table_get_internal(
      &vm->heap, table_idx, Value::FromSymbol(create_symbol("#status")));
  if (status_val.tag == ValueTag::INT) {
    res.status = static_cast<int>(status_val.as.INT);
  } else if (status_val.tag != ValueTag::NONE) {
    throw msl_runtime_error(where,
                            "http server handler status of invalid type:" +
                                core_utils::type_to_string(status_val.tag));
  } else {
    res.status = 200;
  }

  // Content-Type fallback
  std::string content_type = "text/plain";

  // Headers
  Value headers_val = table_get_internal(
      &vm->heap, table_idx, Value::FromSymbol(Constants::SYM_HEADERS));
  if (headers_val.tag == ValueTag::TABLE) {
    VMHIDX headers_idx = headers_val.as.TABLE;
    VMHIDX curr = vm->heap.node_at(headers_idx)->first_child;
    while (curr != INVALID) {
      Value key_val = vm->heap.nth_child(curr, 0);
      Value val_val = vm->heap.nth_child(curr, 1);

      std::string key =
          core_utils::value_to_string(&vm->stack, &vm->heap, key_val);
      std::string val =
          core_utils::value_to_string(&vm->stack, &vm->heap, val_val);

      if (key == "Content-Type") {
        content_type = val;
      } else {
        res.set_header(key, val);
      }
      curr = vm->heap.node_at(curr)->next_child;
    }
  } else if (headers_val.tag != ValueTag::NONE) {
    throw msl_runtime_error(where,
                            "expected a table for response headers but got " +
                                core_utils::type_to_string(headers_val.tag));
  }

  // Body
  Value body_val = table_get_internal(&vm->heap, table_idx,
                                      Value::FromSymbol(Constants::SYM_BODY));
  if (body_val.tag == ValueTag::STRING) {
    res.set_content(vm->heap.get_string(body_val.as.STRING), content_type);
  } else if (body_val.tag != ValueTag::NONE) {
    throw msl_runtime_error(where,
                            "http server handler body of invalid type: " +
                                core_utils::type_to_string(body_val.tag));
  }
}
}; // namespace

bool core::values_equal(VMHeap *heap, Value left, Value right) {
  switch (left.tag) {
  case ValueTag::INT:
    switch (right.tag) {
    case ValueTag::INT:
      return left.as.INT == right.as.INT;
    case ValueTag::FLOAT:
      return left.as.INT == right.as.FLOAT;
    default:
      return false;
    }
  case ValueTag::FLOAT:
    switch (right.tag) {
    case ValueTag::INT:
      return left.as.FLOAT == right.as.INT;
    case ValueTag::FLOAT:
      return left.as.FLOAT == right.as.FLOAT;
    default:
      return false;
    }
  case ValueTag::SYMBOL:
    return right.tag == ValueTag::SYMBOL &&
           left.as.SYMBOL.index == right.as.SYMBOL.index;
  case ValueTag::STRING:
    return right.tag == ValueTag::STRING &&
           (left.as.STRING == right.as.STRING ||
            (heap->get_string(left.as.STRING) ==
             heap->get_string(right.as.STRING)));
  case ValueTag::LIST:
    return right.tag == ValueTag::LIST && left.as.LIST == right.as.LIST;
  case ValueTag::ERROR:
    return right.tag == ValueTag::ERROR &&
           values_equal(heap, heap->at(left.as.ERROR),
                        heap->at(right.as.ERROR));
  case ValueTag::BOX:
    return right.tag == ValueTag::BOX &&
           values_equal(heap, heap->at(left.as.BOX), heap->at(right.as.BOX));
  case ValueTag::NONE:
    return right.tag == ValueTag::NONE;
  case ValueTag::ITERATOR:
    return right.tag == ValueTag::ITERATOR &&
           left.as.ITERATOR == right.as.ITERATOR;
  case ValueTag::TABLE:
    return right.tag == ValueTag::TABLE && left.as.TABLE == right.as.TABLE;
    break;
  case ValueTag::FN_REF:
    return right.tag == ValueTag::FN_REF &&
           values_equal(heap, heap->at(left.as.FN_REF),
                        heap->at(right.as.FN_REF));
  }
  return false;
}

Value core::print(LocationRef, VM *vm) {
  println(core_utils::value_to_string(&vm->stack, &vm->heap, vm->stack.pop()));
  return Value::None();
}
Value core::make_error(LocationRef, VM *vm) {
  Value inner = vm->stack.pop();
  return Value::Error(vm->heap.add(inner));
}
[[noreturn]] Value core::vmpanic(LocationRef where, VM *vm) {
  throw msl_runtime_error(where, core_utils::value_to_string(
                                     &vm->stack, &vm->heap, vm->stack.pop()));
}
// =====================
// ===== CONTAINER =====
// =====================

Value core::container_at(LocationRef where, VM *vm) {
  Value key_value = vm->stack.pop();
  Value container_value = vm->stack.pop();
  switch (container_value.tag) {
  case ValueTag::LIST:
    core_utils::assert_int(where, key_value);
    return vm->heap.nth_child(container_value.as.LIST, key_value.as.INT);
  case ValueTag::TABLE: {
    VMHIDX idx = vm->heap.nth_child_idx(container_value.as.TABLE, 0);
    while (idx != INVALID) {
      // that element is a list and the key is the first element in that list
      Value value = vm->heap.nth_child(idx, 0);
      if (values_equal(&vm->heap, key_value, value)) {
        // the second element is the value
        return vm->heap.nth_child(idx, 1);
      }
      idx = vm->heap.next_child(idx);
    }
    return Value::None();
  } break;
  default:
    throw msl_runtime_error(
        where, "expected a list or table but got a(n) " +
                   core_utils::type_to_string(container_value.tag));
  }
}

Value core::container_put(LocationRef where, VM *vm) {
  Value new_value = vm->stack.pop();
  Value key_value = vm->stack.pop();
  Value container_value = vm->stack.pop();
  switch (container_value.tag) {
  case ValueTag::LIST: {
    core_utils::assert_int(where, key_value);
    vm->heap.set_nth_child(container_value.as.LIST, key_value.as.INT,
                           new_value);
  } break;
  case ValueTag::TABLE: {
    VMHIDX idx = vm->heap.nth_child_idx(container_value.as.TABLE, 0);
    while (idx != INVALID) {
      Value value = vm->heap.nth_child(idx, 0);
      if (values_equal(&vm->heap, key_value, value)) {
        vm->heap.replace(vm->heap.nth_child_idx(idx, 1), new_value);
        return Value::None();
      }
      idx = vm->heap.next_child(idx);
    }
    core_utils::table_add_entry(&vm->heap, container_value.as.TABLE, key_value,
                                new_value);
    return Value::None();
  } break;
  default:
    throw msl_runtime_error(
        where, "expected a list or table but got a(n) " +
                   core_utils::type_to_string(container_value.tag));
  }
  return Value::None();
}
Value core::len(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  if (val.tag == ValueTag::STRING) {
    return Value::Int(vm->heap.get_string(val.as.STRING).length());
  } else if (val.tag == ValueTag::LIST || val.tag == ValueTag::TABLE) {
    uint64_t count = 0;
    VMHIDX curr =
        vm->heap
            .node_at(val.tag == ValueTag::LIST ? val.as.LIST : val.as.TABLE)
            ->first_child;
    while (curr != INVALID) {
      count++;
      curr = vm->heap.node_at(curr)->next_child;
    }
    return Value::Int(count);
  } else {
    throw msl_runtime_error(where, "expected string, list or table for len()");
  }
}

// =================
// ===== TABLE =====
// =================

Value core::table(LocationRef where, VM *vm) {
  int64_t args_count = vm->stack.pop().as.INT;
  if (args_count % 2 != 0) {
    throw msl_runtime_error(where, "table: expected key-value pairs");
  }
  VMHIDX table_head = vm->heap.new_table();
  // table(k1, v1, k2, v2)
  // stack: [k1, v1, k2, v2, 4]
  // after popping count: [k1, v1, k2, v2]
  // we need to pop k and v in pairs.
  // since it is a stack, it pops from right to left: v2, then k2, then v1, then
  // k1.
  for (int64_t i = 0; i < args_count / 2; ++i) {
    Value value = vm->stack.pop();
    Value key = vm->stack.pop();
    core_utils::table_add_entry_front(&vm->heap, table_head, key, value);
  }
  return vm->heap.at(table_head);
}

Value core::table_keys(LocationRef where, VM *vm) {
  Value tab_val = vm->stack.pop();
  core_utils::assert_table(where, tab_val);
  VMHNode *table_head = vm->heap.node_at(tab_val.as.TABLE);
  VMHIDX keys_list = vm->heap.new_list();
  VMHIDX tab_curr = table_head->first_child;
  while (tab_curr != INVALID) {
    vm->heap.add_child(keys_list, vm->heap.nth_child(tab_curr, 0));
    tab_curr = vm->heap.node_at(tab_curr)->next_child;
  }
  return vm->heap.at(keys_list);
}

Value core::table_values(LocationRef where, VM *vm) {
  Value tab_val = vm->stack.pop();
  core_utils::assert_table(where, tab_val);
  VMHNode *table_head = vm->heap.node_at(tab_val.as.TABLE);
  VMHIDX values_list = vm->heap.new_list();
  VMHIDX tab_curr = table_head->first_child;
  while (tab_curr != INVALID) {
    vm->heap.add_child(values_list, vm->heap.nth_child(tab_curr, 1));
    tab_curr = vm->heap.node_at(tab_curr)->next_child;
  }
  return vm->heap.at(values_list);
}

namespace {
picojson::value msl_to_picojson(VM *vm, Value val) {
  switch (val.tag) {
  case ValueTag::INT:
    return picojson::value((double)val.as.INT);
  case ValueTag::FLOAT:
    return picojson::value(val.as.FLOAT);
  case ValueTag::STRING:
    return picojson::value(vm->heap.get_string(val.as.STRING));
  case ValueTag::SYMBOL: {
    std::string s = resolve_symbol(val.as.SYMBOL);
    if (s == "#true")
      return picojson::value(true);
    if (s == "#false")
      return picojson::value(false);
    return picojson::value(s);
  }
  case ValueTag::NONE:
    return picojson::value();
  case ValueTag::LIST: {
    picojson::array arr;
    VMHIDX curr = vm->heap.node_at(val.as.LIST)->first_child;
    while (curr != INVALID) {
      arr.push_back(msl_to_picojson(vm, vm->heap.at(curr)));
      curr = vm->heap.node_at(curr)->next_child;
    }
    return picojson::value(arr);
  }
  case ValueTag::TABLE: {
    picojson::object obj;
    VMHIDX curr = vm->heap.node_at(val.as.TABLE)->first_child;
    while (curr != INVALID) {
      Value key_val = vm->heap.nth_child(curr, 0);
      Value value_val = vm->heap.nth_child(curr, 1);
      std::string key;
      if (key_val.tag == ValueTag::SYMBOL) {
        key = resolve_symbol(key_val.as.SYMBOL);
        if (key.size() > 0 && key[0] == '#')
          key = key.substr(1);
      } else {
        key = core_utils::value_to_string(&vm->stack, &vm->heap, key_val);
      }
      obj[key] = msl_to_picojson(vm, value_val);
      curr = vm->heap.node_at(curr)->next_child;
    }
    return picojson::value(obj);
  }
  default:
    return picojson::value();
  }
}

Value picojson_to_msl(VM *vm, const picojson::value &val) {
  if (val.is<picojson::null>())
    return Value::None();
  if (val.is<bool>())
    return core_utils::to_bool(val.get<bool>());
  if (val.is<double>()) {
    double d = val.get<double>();
    if (d == std::floor(d))
      return Value::Int((int64_t)d);
    return Value::Float(d);
  }
  if (val.is<std::string>())
    return vm->heap.add_string(val.get<std::string>());
  if (val.is<picojson::array>()) {
    const picojson::array &arr = val.get<picojson::array>();
    VMHIDX list_head = vm->heap.new_list();
    for (const auto &item : arr) {
      vm->heap.add_child(list_head, picojson_to_msl(vm, item));
    }
    return vm->heap.at(list_head);
  }
  if (val.is<picojson::object>()) {
    const picojson::object &obj = val.get<picojson::object>();
    VMHIDX table_head = vm->heap.new_table();
    for (const auto &kv : obj) {
      core_utils::table_add_entry(&vm->heap, table_head,
                                  vm->heap.add_string(kv.first),
                                  picojson_to_msl(vm, kv.second));
    }
    return vm->heap.at(table_head);
  }
  return Value::None();
}
} // namespace

Value core::table_to_json(LocationRef, VM *vm) {
  Value val = vm->stack.pop();
  picojson::value jval = msl_to_picojson(vm, val);
  return vm->heap.add_string(jval.serialize());
}

Value core::table_from_json(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  if (val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "table_from_json: expected a string");
  }
  std::string json_str = vm->heap.get_string(val.as.STRING);
  picojson::value jval;
  std::string err = picojson::parse(jval, json_str);
  if (!err.empty()) {
    return core_utils::create_error(&vm->heap, "json parse error: " + err);
  }
  return picojson_to_msl(vm, jval);
}

Value core::table_remove(LocationRef where, VM *vm) {
  Value key = vm->stack.pop();
  Value table_val = vm->stack.pop();
  core_utils::assert_table(where, table_val);

  VMHIDX prev = INVALID;
  VMHIDX curr = vm->heap.node_at(table_val.as.TABLE)->first_child;

  while (curr != INVALID) {
    Value current_key = vm->heap.nth_child(curr, 0);
    if (core::values_equal(&vm->heap, current_key, key)) {
      VMHIDX next = vm->heap.node_at(curr)->next_child;

      if (prev == INVALID) {
        vm->heap.node_at(table_val.as.TABLE)->first_child = next;
      } else {
        vm->heap.node_at(prev)->next_child = next;
      }

      if (next == INVALID) {
        vm->heap.node_at(table_val.as.TABLE)->last_child = prev;
      }

      return table_val;
    }

    prev = curr;
    curr = vm->heap.node_at(curr)->next_child;
  }

  return Value::None();
}

// ================
// ===== LIST =====
// ================

Value core::list(LocationRef, VM *vm) {
  int64_t args_count = vm->stack.pop().as.INT;
  VMHIDX list_head = vm->heap.new_list();
  for (int64_t i = 0; i < args_count; ++i) {
    vm->heap.add_child_front(list_head, vm->stack.pop());
  }
  return vm->heap.at(list_head);
}
Value core::list_append(LocationRef where, VM *vm) {
  Value value = vm->stack.pop();
  Value list = vm->stack.pop();
  core_utils::assert_list(where, list);
  vm->heap.add_child(list.as.LIST, value);
  return list;
}
Value core::list_prepend(LocationRef where, VM *vm) {
  Value value = vm->stack.pop();
  Value list = vm->stack.pop();
  core_utils::assert_list(where, list);
  vm->heap.add_child_front(list.as.LIST, value);
  return list;
}
Value core::list_link(LocationRef where, VM *vm) {
  Value right_list = vm->stack.pop();
  core_utils::assert_list(where, right_list);
  Value left_list = vm->stack.pop();
  core_utils::assert_list(where, left_list);
  if (left_list.as.LIST == right_list.as.LIST) {
    throw msl_runtime_error(
        where,
        "linking a list to itself is undefined behaviour. in this specific "
        "case i caught it, but i cannot save you from every mistake.");
  }
  vm->heap.link_lists(left_list.as.LIST, right_list.as.LIST);
  return left_list;
}

Value core::list_slice(LocationRef where, VM *vm) {
  Value end_val = vm->stack.pop();
  Value start_val = vm->stack.pop();
  Value list_val = vm->stack.pop();

  core_utils::assert_list(where, list_val);
  core_utils::assert_int(where, start_val);
  core_utils::assert_int(where, end_val);

  int64_t start = start_val.as.INT;
  int64_t end = end_val.as.INT;

  if (start < 0 || end < 0 || start > end) {
    throw msl_runtime_error(where, "invalid slice indices");
  }

  VMHIDX new_list = vm->heap.new_list();
  VMHIDX curr = vm->heap.node_at(list_val.as.LIST)->first_child;
  int64_t i = 0;
  while (curr != INVALID && i < end) {
    if (i >= start) {
      vm->heap.add_child(new_list, vm->heap.at(curr));
    }
    curr = vm->heap.node_at(curr)->next_child;
    i++;
  }
  return vm->heap.at(new_list);
}

Value core::list_remove(LocationRef where, VM *vm) {
  Value index_val = vm->stack.pop();
  Value list_val = vm->stack.pop();

  core_utils::assert_list(where, list_val);
  core_utils::assert_int(where, index_val);

  int64_t index = index_val.as.INT;
  if (index < 0) {
    throw msl_runtime_error(where, "index out of bounds");
  }

  VMHIDX list_ref = list_val.as.LIST;
  VMHNode *list_node = vm->heap.node_at(list_ref);
  VMHIDX curr = list_node->first_child;
  VMHIDX prev = INVALID;
  int64_t i = 0;

  while (curr != INVALID && i < index) {
    prev = curr;
    curr = vm->heap.node_at(curr)->next_child;
    i++;
  }

  if (curr == INVALID) {
    throw msl_runtime_error(where, "index out of bounds");
  }

  if (prev == INVALID) {
    // removing first child
    list_node->first_child = vm->heap.node_at(curr)->next_child;
    if (list_node->first_child == INVALID) {
      list_node->last_child = INVALID;
    }
  } else {
    vm->heap.node_at(prev)->next_child = vm->heap.node_at(curr)->next_child;
    if (vm->heap.node_at(prev)->next_child == INVALID) {
      list_node->last_child = prev;
    }
  }

  return list_val;
}

Value core::list_contains(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  Value list_val = vm->stack.pop();

  core_utils::assert_list(where, list_val);

  VMHIDX curr = vm->heap.node_at(list_val.as.LIST)->first_child;
  while (curr != INVALID) {
    if (values_equal(&vm->heap, vm->heap.at(curr), val)) {
      return Value::FromSymbol(Constants::SYM_TRUE);
    }
    curr = vm->heap.node_at(curr)->next_child;
  }
  return Value::FromSymbol(Constants::SYM_FALSE);
}

Value core::range(LocationRef where, VM *vm) {
  Value end_val = vm->stack.pop();
  Value start_val = vm->stack.pop();

  core_utils::assert_int(where, start_val);
  core_utils::assert_int(where, end_val);

  int64_t start = start_val.as.INT;
  int64_t end = end_val.as.INT;

  VMHIDX list_head = vm->heap.new_list();
  for (int64_t i = start; i < end; ++i) {
    vm->heap.add_child(list_head, Value::Int(i));
  }
  return vm->heap.at(list_head);
}

// =================
// ===== OTHER =====
// =================
//
Value core::http_get(LocationRef where, VM *vm) {
  std::string path = core_utils::as_string(where, vm->stack.pop(), &vm->heap);
  std::string host_address =
      core_utils::as_string(where, vm->stack.pop(), &vm->heap);
  httplib::Client cli(host_address);
  httplib::Result res = cli.Get(path);
  if (!res) {
    return core_utils::create_error(&vm->heap, "request to " + host_address +
                                                   path + " failed");
  }
  return response_to_table(vm, res);
}

Value core::http_post(LocationRef where, VM *vm) {
  std::string body = core_utils::as_string(where, vm->stack.pop(), &vm->heap);
  std::string path = core_utils::as_string(where, vm->stack.pop(), &vm->heap);
  std::string host_address =
      core_utils::as_string(where, vm->stack.pop(), &vm->heap);
  httplib::Client cli(host_address);
  httplib::Result res = cli.Post(path, body, "text/plain");
  if (!res) {
    return core_utils::create_error(&vm->heap, "request to " + host_address +
                                                   path + " failed");
  }
  return response_to_table(vm, res);
}

Value core::http_put(LocationRef where, VM *vm) {
  std::string body = core_utils::as_string(where, vm->stack.pop(), &vm->heap);
  std::string path = core_utils::as_string(where, vm->stack.pop(), &vm->heap);
  std::string host_address =
      core_utils::as_string(where, vm->stack.pop(), &vm->heap);
  httplib::Client cli(host_address);
  httplib::Result res = cli.Put(path, body, "text/plain");
  if (!res) {
    return core_utils::create_error(&vm->heap, "request to " + host_address +
                                                   path + " failed");
  }
  return response_to_table(vm, res);
}

Value core::http_patch(LocationRef where, VM *vm) {
  std::string body = core_utils::as_string(where, vm->stack.pop(), &vm->heap);
  std::string path = core_utils::as_string(where, vm->stack.pop(), &vm->heap);
  std::string host_address =
      core_utils::as_string(where, vm->stack.pop(), &vm->heap);
  httplib::Client cli(host_address);
  httplib::Result res = cli.Patch(path, body, "text/plain");
  if (!res) {
    return core_utils::create_error(&vm->heap, "request to " + host_address +
                                                   path + " failed");
  }
  return response_to_table(vm, res);
}

Value core::http_delete(LocationRef where, VM *vm) {
  std::string path = core_utils::as_string(where, vm->stack.pop(), &vm->heap);
  std::string host_address =
      core_utils::as_string(where, vm->stack.pop(), &vm->heap);
  httplib::Client cli(host_address);
  httplib::Result res = cli.Delete(path);
  if (!res) {
    return core_utils::create_error(&vm->heap, "request to " + host_address +
                                                   path + " failed");
  }
  return response_to_table(vm, res);
}
namespace Global {
class SingleThreadedTaskQueue : public httplib::TaskQueue {
public:
  SingleThreadedTaskQueue() {};
  virtual ~SingleThreadedTaskQueue() {};
  virtual bool enqueue(std::function<void()> fn) override {
    fn();
    return true;
  }
  virtual void shutdown() override {}
};
struct GlobalState {
  std::optional<httplib::Server> server;
};
static GlobalState state;

}; // namespace Global

Value core::http_on(LocationRef where, VM *vm) {
  Value handler_ref = vm->stack.pop();
  Value path_str = vm->stack.pop();
  Value method_sym = vm->stack.pop();
  core_utils::assert_sym(where, method_sym);
  core_utils::assert_str(where, path_str);
  core_utils::assert_fn_ref(where, handler_ref);
  if (!Global::state.server.has_value()) {
    Global::state.server.emplace();
    Global::state.server->new_task_queue = []() {
      return new Global::SingleThreadedTaskQueue();
    };
  }
  if (method_sym.as.SYMBOL.index == Constants::SYM_GET.index) {
    Global::state.server->Get(
        vm->heap.get_string(path_str.as.STRING),
        [vm, handler_ref, where](const httplib::Request &req,
                                 httplib::Response &res) {
          VMHIDX req_table_idx = vm->heap.new_table();
          core_utils::table_add_entry(&vm->heap, req_table_idx,
                                      Value::FromSymbol(Constants::SYM_METHOD),
                                      vm->heap.add_string(req.method));
          core_utils::table_add_entry(&vm->heap, req_table_idx,
                                      Value::FromSymbol(Constants::SYM_BODY),
                                      vm->heap.add_string(req.body));
          VMHIDX params_table = vm->heap.new_table();
          for (auto [key, values] : req.params) {
            core_utils::table_add_entry(&vm->heap, params_table,
                                        Value::FromSymbol(create_symbol(key)),
                                        vm->heap.add_string(values));
          }
          core_utils::table_add_entry(&vm->heap, req_table_idx,
                                      Value::FromSymbol(Constants::SYM_PARAMS),
                                      vm->heap.at(params_table));

          Value handler_result =
              vm->call_reference(handler_ref, {vm->heap.at(req_table_idx)});
          map_msl_table_to_response(where, vm, handler_result, res);
        });
  }

  return Value::None();
}

Value core::http_listen(LocationRef where, VM *vm) {
  Value port = vm->stack.pop();
  core_utils::assert_int(where, port);
  if (!Global::state.server.has_value()) {
    Global::state.server.emplace();
    Global::state.server->new_task_queue = []() {
      return new Global::SingleThreadedTaskQueue();
    };
  }
  Global::state.server->listen("127.0.0.1", port.as.INT);
  return Value::Int(0);
}

Value core::value_copy(LocationRef, VM *vm) {
  Value value = vm->stack.pop();
  return core_utils::copy_value(&vm->stack, &vm->heap, value);
}
Value core::vmtypeof(LocationRef, VM *vm) {
  Value value = vm->stack.pop();
  return core_utils::type_to_symbol(value);
}
Value core::string_concat(LocationRef where, VM *vm) {
  Value right = vm->stack.pop();
  Value left = vm->stack.pop();
  if (left.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected string as left argument");
  }
  if (right.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings as right argument");
  }
  std::string s1 = vm->heap.get_string(left.as.STRING);
  std::string s2 = vm->heap.get_string(right.as.STRING);
  return vm->heap.add_string(s1 + s2);
}

Value core::value_to_int(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  switch (val.tag) {
  case ValueTag::INT:
    return val;
  case ValueTag::FLOAT:
    return Value::Int(static_cast<int64_t>(val.as.FLOAT));
  case ValueTag::STRING: {
    try {
      return Value::Int(std::stoll(vm->heap.get_string(val.as.STRING)));
    } catch (...) {
      return Value::Error(
          vm->heap.add(vm->heap.add_string("could not convert string to int")));
    }
  }
  default:
    throw msl_runtime_error(where, "cannot convert " +
                                       core_utils::type_to_string(val.tag) +
                                       " to int");
  }
}

Value core::value_to_float(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  switch (val.tag) {
  case ValueTag::INT:
    return Value::Float(static_cast<double>(val.as.INT));
  case ValueTag::FLOAT:
    return val;
  case ValueTag::STRING: {
    try {
      return Value::Float(std::stod(vm->heap.get_string(val.as.STRING)));
    } catch (...) {
      return core_utils::create_error(
          &vm->heap, "could not convert '" +
                         vm->heap.get_string(val.as.STRING) + "' to float");
    }
  }
  default:
    throw msl_runtime_error(where, "cannot convert " +
                                       core_utils::type_to_string(val.tag) +
                                       " to float");
  }
}

Value core::value_to_string_fn(LocationRef, VM *vm) {
  Value val = vm->stack.pop();
  return vm->heap.add_string(
      core_utils::value_to_string(&vm->stack, &vm->heap, val));
}

Value core::fs_read(LocationRef where, VM *vm) {
  Value path_val = vm->stack.pop();
  if (path_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected a string for file path");
  }
  std::string path = vm->heap.get_string(path_val.as.STRING);
  std::ifstream f(path);
  if (!f.is_open()) {
    return Value::Error(
        vm->heap.add(vm->heap.add_string("could not open file: " + path)));
  }
  std::stringstream buffer;
  buffer << f.rdbuf();
  return vm->heap.add_string(buffer.str());
}

Value core::fs_write(LocationRef where, VM *vm) {
  Value content_val = vm->stack.pop();
  Value path_val = vm->stack.pop();
  if (path_val.tag != ValueTag::STRING || content_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings for path and content");
  }
  std::string path = vm->heap.get_string(path_val.as.STRING);
  std::string content = vm->heap.get_string(content_val.as.STRING);
  std::ofstream f(path);
  if (!f.is_open()) {
    return Value::Error(
        vm->heap.add(vm->heap.add_string("could not open file: " + path)));
  }
  f << content;
  return Value::None();
}

Value core::fs_append(LocationRef where, VM *vm) {
  Value content_val = vm->stack.pop();
  Value path_val = vm->stack.pop();
  if (path_val.tag != ValueTag::STRING || content_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings for path and content");
  }
  std::string path = vm->heap.get_string(path_val.as.STRING);
  std::string content = vm->heap.get_string(content_val.as.STRING);
  std::ofstream f(path, std::ios::app);
  if (!f.is_open()) {
    return Value::Error(
        vm->heap.add(vm->heap.add_string("could not open file: " + path)));
  }
  f << content;
  return Value::None();
}

Value core::fs_stat(LocationRef where, VM *vm) {
  Value path_val = vm->stack.pop();
  if (path_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected a string for path");
  }
  std::string path_str = vm->heap.get_string(path_val.as.STRING);
  std::filesystem::path path(path_str);

  VMHIDX table_idx = vm->heap.new_table();

  try {
    if (!std::filesystem::exists(path)) {
      return core_utils::create_error(&vm->heap, "no such file or directory");
    }

    Value type_sym = Value::FromSymbol(Constants::SYM_FS_OTHER);
    if (std::filesystem::is_regular_file(path)) {
      type_sym = Value::FromSymbol(Constants::SYM_FS_FILE);
    } else if (std::filesystem::is_directory(path)) {
      type_sym = Value::FromSymbol(Constants::SYM_FS_DIRECTORY);
    } else if (std::filesystem::is_symlink(path)) {
      type_sym = Value::FromSymbol(Constants::SYM_FS_SYMLINK);
    }
    core_utils::table_add_entry(&vm->heap, table_idx,
                                Value::FromSymbol(create_symbol("#type")),
                                type_sym);

    uint64_t size = 0;
    if (std::filesystem::is_regular_file(path)) {
      size = std::filesystem::file_size(path);
    }
    core_utils::table_add_entry(&vm->heap, table_idx,
                                Value::FromSymbol(create_symbol("#size")),
                                Value::Int(size));

    // calculate the difference between now in system time and now in file time
    auto ftime = std::filesystem::last_write_time(path);
    auto wall_now = std::chrono::system_clock::now();
    auto file_now = std::filesystem::file_time_type::clock::now();
    auto diff = wall_now.time_since_epoch() - file_now.time_since_epoch();

    // apply that difference to the specific file time
    auto stime = ftime.time_since_epoch() + diff;
    auto seconds =
        std::chrono::duration_cast<std::chrono::seconds>(stime).count();
    core_utils::table_add_entry(&vm->heap, table_idx,
                                Value::FromSymbol(create_symbol("#last_write")),
                                Value::Int(seconds));

  } catch (const std::filesystem::filesystem_error &e) {
    return core_utils::create_error(&vm->heap,
                                    "fs_stat failed: " + std::string(e.what()));
  }

  return vm->heap.at(table_idx);
}

Value core::sys_env_get(LocationRef where, VM *vm) {
  Value name_val = vm->stack.pop();
  if (name_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where,
                            "expected a string for environment variable name");
  }
  const char *val =
      std::getenv(vm->heap.get_string(name_val.as.STRING).c_str());
  if (val == nullptr) {
    return Value::Error(vm->heap.add(
        vm->heap.add_string("environment variable not found: " +
                            vm->heap.get_string(name_val.as.STRING))));
  }
  return vm->heap.add_string(std::string(val));
}

Value core::sys_sleep(LocationRef where, VM *vm) {
  Value ms_val = vm->stack.pop();
  std::this_thread::sleep_for(
      chrono::milliseconds(core_utils::as_int(where, ms_val)));
  return Value::None();
}

Value core::process_args(LocationRef, VM *vm) {
  VMHIDX list_head = vm->heap.new_list();
  for (const auto &arg : ::msl_args) {
    vm->heap.add_child(list_head, vm->heap.add_string(arg));
  }
  return vm->heap.at(list_head);
}

void core::set_args(const std::vector<std::string> &args) { ::msl_args = args; }

Value core::sys_exit(LocationRef where, VM *vm) {
  Value code_val = vm->stack.pop();
  if (code_val.tag != ValueTag::INT) {
    throw msl_runtime_error(where, "expected an integer for exit code");
  }
  std::exit(static_cast<int>(code_val.as.INT));
}

Value core::sys_exec(LocationRef, VM *vm) {
  int64_t args_count = vm->stack.pop().as.INT;
  std::string command = "";
  for (int64_t i = 0; i < args_count; ++i) {
    command =
        core_utils::value_to_string(&vm->stack, &vm->heap, vm->stack.pop())
            .append(" ")
            .append(command);
  }
  int result = std::system(command.c_str());
  return Value::Int(result);
}

Value core::random_int(LocationRef where, VM *vm) {
  Value max_val = vm->stack.pop();
  Value min_val = vm->stack.pop();
  if (min_val.tag != ValueTag::INT || max_val.tag != ValueTag::INT) {
    throw msl_runtime_error(where, "expected integers for random range");
  }
  if (min_val.as.INT > max_val.as.INT) {
    throw msl_runtime_error(where, "min must be less than or equal to max");
  }

  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<int64_t> distrib(min_val.as.INT,
                                                 max_val.as.INT);

  return Value::Int(distrib(gen));
}

Value core::time_epoch_ms(LocationRef, VM *) {
  auto now = chrono::system_clock::now();
  auto ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch())
                .count();
  return Value::Int(ms);
}

Value core::time_epoch_sec(LocationRef, VM *) {
  auto now = chrono::system_clock::now();
  auto sec =
      chrono::duration_cast<chrono::seconds>(now.time_since_epoch()).count();
  return Value::Int(sec);
}

Value core::time_iso8601(LocationRef, VM *vm) {
  auto now = chrono::system_clock::now();
  std::time_t now_c = chrono::system_clock::to_time_t(now);
  std::tm now_tm;
#ifdef _WIN32
  gmtime_s(&now_tm, &now_c);
#else
  gmtime_r(&now_c, &now_tm);
#endif
  std::stringstream ss;
  ss << std::put_time(&now_tm, "%Y-%m-%dT%H:%M:%SZ");
  return vm->heap.add_string(ss.str());
}

Value core::str_split(LocationRef where, VM *vm) {
  Value delim_val = vm->stack.pop();
  Value str_val = vm->stack.pop();
  if (str_val.tag != ValueTag::STRING || delim_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings for str_split");
  }
  std::string s = vm->heap.get_string(str_val.as.STRING);
  std::string delim = vm->heap.get_string(delim_val.as.STRING);
  VMHIDX list_head = vm->heap.new_list();
  if (delim.empty()) {
    for (char c : s) {
      vm->heap.add_child(list_head, vm->heap.add_string(std::string(1, c)));
    }
  } else {
    size_t start = 0;
    size_t end = s.find(delim);
    while (end != std::string::npos) {
      vm->heap.add_child(list_head,
                         vm->heap.add_string(s.substr(start, end - start)));
      start = end + delim.length();
      end = s.find(delim, start);
    }
    vm->heap.add_child(list_head, vm->heap.add_string(s.substr(start)));
  }
  return vm->heap.at(list_head);
}

Value core::str_replace(LocationRef where, VM *vm) {
  Value replacement_val = vm->stack.pop();
  Value search_val = vm->stack.pop();
  Value str_val = vm->stack.pop();
  if (str_val.tag != ValueTag::STRING || search_val.tag != ValueTag::STRING ||
      replacement_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings for str_replace");
  }
  std::string s = vm->heap.get_string(str_val.as.STRING);
  std::string search = vm->heap.get_string(search_val.as.STRING);
  std::string replacement = vm->heap.get_string(replacement_val.as.STRING);
  if (search.empty())
    return str_val;
  size_t pos = 0;
  while ((pos = s.find(search, pos)) != std::string::npos) {
    s.replace(pos, search.length(), replacement);
    pos += replacement.length();
  }
  return vm->heap.add_string(s);
}

Value core::str_contains(LocationRef where, VM *vm) {
  Value sub_val = vm->stack.pop();
  Value str_val = vm->stack.pop();
  if (str_val.tag != ValueTag::STRING || sub_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings for str_contains");
  }
  std::string s = vm->heap.get_string(str_val.as.STRING);
  std::string sub = vm->heap.get_string(sub_val.as.STRING);
  bool found = s.find(sub) != std::string::npos;
  return core_utils::to_bool(found);
}

Value core::str_has_prefix(LocationRef where, VM *vm) {
  Value prefix_val = vm->stack.pop();
  Value str_val = vm->stack.pop();
  if (str_val.tag != ValueTag::STRING || prefix_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings for str_has_prefix");
  }
  std::string s = vm->heap.get_string(str_val.as.STRING);
  std::string prefix = vm->heap.get_string(prefix_val.as.STRING);
  bool res = s.compare(0, prefix.length(), prefix) == 0;
  return core_utils::to_bool(res);
}

Value core::str_has_suffix(LocationRef where, VM *vm) {
  Value suffix_val = vm->stack.pop();
  Value str_val = vm->stack.pop();
  if (str_val.tag != ValueTag::STRING || suffix_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings for str_has_suffix");
  }
  std::string s = vm->heap.get_string(str_val.as.STRING);
  std::string suffix = vm->heap.get_string(suffix_val.as.STRING);
  bool res = false;
  if (s.length() >= suffix.length()) {
    res = s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0;
  }
  return core_utils::to_bool(res);
}

Value core::str_lower(LocationRef where, VM *vm) {
  Value str_val = vm->stack.pop();
  if (str_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected string for str_lower");
  }
  std::string s = vm->heap.get_string(str_val.as.STRING);
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return vm->heap.add_string(s);
}

Value core::str_upper(LocationRef where, VM *vm) {
  Value str_val = vm->stack.pop();
  if (str_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected string for str_upper");
  }
  std::string s = vm->heap.get_string(str_val.as.STRING);
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return vm->heap.add_string(s);
}

Value core::str_trim(LocationRef where, VM *vm) {
  Value str_val = vm->stack.pop();
  if (str_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected string for str_trim");
  }
  std::string s = vm->heap.get_string(str_val.as.STRING);
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
  return vm->heap.add_string(s);
}
Value core::str_trim_left(LocationRef where, VM *vm) {
  Value str_val = vm->stack.pop();
  if (str_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected string for str_trim_left");
  }
  std::string s = vm->heap.get_string(str_val.as.STRING);
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
  return vm->heap.add_string(s);
}
Value core::str_trim_right(LocationRef where, VM *vm) {
  Value str_val = vm->stack.pop();
  if (str_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected string for str_trim_right");
  }
  std::string s = vm->heap.get_string(str_val.as.STRING);
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
  return vm->heap.add_string(s);
}

Value core::vmassert(LocationRef where, VM *vm) {
  Value value = vm->stack.pop();
  if (!core_utils::as_bool(where, value)) {
    throw msl_runtime_error(where, "assertion failed");
  }
  return Value::None();
}
Value core::vmassert_type(LocationRef where, VM *vm) {
  Value typeval = vm->stack.pop();
  Value val = vm->stack.pop();
  if (typeval.tag != ValueTag::SYMBOL) {
    throw msl_runtime_error(
        where, "expected a symbol as second argument but got a(n) " +
                   core_utils::type_to_string(typeval.tag));
  }
  ValueTag expected = core_utils::symbol_to_type(where, typeval.as.SYMBOL);
  if (val.tag != expected) {
    throw msl_runtime_error(
        where, "expected a " + core_utils::type_to_string(expected) +
                   " but got " + core_utils::type_to_string(val.tag) + "");
  }
  return Value::None();
}

Value core::math_abs(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  switch (val.tag) {
  case ValueTag::INT:
    return Value::Int(std::abs(val.as.INT));
  case ValueTag::FLOAT:
    return Value::Float(std::abs(val.as.FLOAT));
  default:
    throw msl_runtime_error(where, "expected a number for math_abs");
  }
}

Value core::math_floor(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  switch (val.tag) {
  case ValueTag::INT:
    return val;
  case ValueTag::FLOAT:
    return Value::Float(std::floor(val.as.FLOAT));
  default:
    throw msl_runtime_error(where, "expected a number for math_floor");
  }
}

Value core::math_ceil(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  switch (val.tag) {
  case ValueTag::INT:
    return val;
  case ValueTag::FLOAT:
    return Value::Float(std::ceil(val.as.FLOAT));
  default:
    throw msl_runtime_error(where, "expected a number for math_ceil");
  }
}

Value core::math_round(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  switch (val.tag) {
  case ValueTag::INT:
    return val;
  case ValueTag::FLOAT:
    return Value::Float(std::round(val.as.FLOAT));
  default:
    throw msl_runtime_error(where, "expected a number for math_round");
  }
}

Value core::math_sqrt(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  double d;
  switch (val.tag) {
  case ValueTag::INT:
    d = static_cast<double>(val.as.INT);
    break;
  case ValueTag::FLOAT:
    d = val.as.FLOAT;
    break;
  default:
    throw msl_runtime_error(where, "expected a number for math_sqrt");
  }
  return Value::Float(std::sqrt(d));
}

Value core::math_pow(LocationRef where, VM *vm) {
  Value y_val = vm->stack.pop();
  Value x_val = vm->stack.pop();
  double x, y;

  switch (x_val.tag) {
  case ValueTag::INT:
    x = static_cast<double>(x_val.as.INT);
    break;
  case ValueTag::FLOAT:
    x = x_val.as.FLOAT;
    break;
  default:
    throw msl_runtime_error(where, "expected a number for math_pow x");
  }

  switch (y_val.tag) {
  case ValueTag::INT:
    y = static_cast<double>(y_val.as.INT);
    break;
  case ValueTag::FLOAT:
    y = y_val.as.FLOAT;
    break;
  default:
    throw msl_runtime_error(where, "expected a number for math_pow y");
  }

  return Value::Float(std::pow(x, y));
}

Value core::math_sin(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  double d;
  switch (val.tag) {
  case ValueTag::INT:
    d = static_cast<double>(val.as.INT);
    break;
  case ValueTag::FLOAT:
    d = val.as.FLOAT;
    break;
  default:
    throw msl_runtime_error(where, "expected a number for math_sin");
  }
  return Value::Float(std::sin(d));
}

Value core::math_cos(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  double d;
  switch (val.tag) {
  case ValueTag::INT:
    d = static_cast<double>(val.as.INT);
    break;
  case ValueTag::FLOAT:
    d = val.as.FLOAT;
    break;
  default:
    throw msl_runtime_error(where, "expected a number for math_cos");
  }
  return Value::Float(std::cos(d));
}

Value core::math_tan(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  double d;
  switch (val.tag) {
  case ValueTag::INT:
    d = static_cast<double>(val.as.INT);
    break;
  case ValueTag::FLOAT:
    d = val.as.FLOAT;
    break;
  default:
    throw msl_runtime_error(where, "expected a number for math_tan");
  }
  return Value::Float(std::tan(d));
}

Value core::math_log(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  double d;
  switch (val.tag) {
  case ValueTag::INT:
    d = static_cast<double>(val.as.INT);
    break;
  case ValueTag::FLOAT:
    d = val.as.FLOAT;
    break;
  default:
    throw msl_runtime_error(where, "expected a number for math_log");
  }
  return Value::Float(std::log(d));
}

Value core::math_exp(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  double d;
  switch (val.tag) {
  case ValueTag::INT:
    d = static_cast<double>(val.as.INT);
    break;
  case ValueTag::FLOAT:
    d = val.as.FLOAT;
    break;
  default:
    throw msl_runtime_error(where, "expected a number for math_exp");
  }
  return Value::Float(std::exp(d));
}

Value core::str_slice(LocationRef where, VM *vm) {
  Value end_val = vm->stack.pop();
  Value start_val = vm->stack.pop();
  Value str_val = vm->stack.pop();

  if (str_val.tag != ValueTag::STRING || start_val.tag != ValueTag::INT ||
      end_val.tag != ValueTag::INT) {
    throw msl_runtime_error(where, "expected str_slice(string, int, int)");
  }

  int64_t start = start_val.as.INT;
  int64_t end = end_val.as.INT;

  if (start > end) {
    throw msl_runtime_error(where, "start must be larger than end");
  }
  if (start < 0 || end < 0) {
    throw msl_runtime_error(where, "start and end must be larger than 0");
  }
  return vm->heap.add_string(
      vm->heap.get_string(str_val.as.STRING).substr(start, end - start));
}

Value core::str_find(LocationRef where, VM *vm) {
  Value sub_val = vm->stack.pop();
  Value str_val = vm->stack.pop();

  if (str_val.tag != ValueTag::STRING || sub_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected str_find(string, string)");
  }

  std::string s = vm->heap.get_string(str_val.as.STRING);
  std::string sub = vm->heap.get_string(sub_val.as.STRING);
  size_t pos = s.find(sub);

  if (pos == std::string::npos) {
    return Value::None();
  }
  return Value::Int(pos);
}

Value core::str_index(LocationRef where, VM *vm) {
  Value index_val = vm->stack.pop();
  Value str_val = vm->stack.pop();

  if (str_val.tag != ValueTag::STRING || index_val.tag != ValueTag::INT) {
    throw msl_runtime_error(where, "expected str_index(string, int)");
  }

  std::string s = vm->heap.get_string(str_val.as.STRING);
  int64_t i = index_val.as.INT;
  int64_t len = s.length();

  if (i < 0) {
    i += len;
  }

  if (i < 0 || i >= len) {
    throw msl_runtime_error(where, "string index out of bounds");
  }

  return vm->heap.add_string(std::string(1, s[i]));
}

Value core::str_fmt(LocationRef where, VM *vm) {
  int64_t args_count = vm->stack.pop().as.INT;
  if (args_count == 0) {
    throw msl_runtime_error(where, "str_fmt expects at least a format string");
  }

  std::vector<Value> args;
  args.reserve(args_count);
  for (int64_t i = 0; i < args_count; ++i) {
    args.push_back(vm->stack.pop());
  }
  std::reverse(args.begin(), args.end());

  Value fmt_str_val = args[0];
  if (fmt_str_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(
        where, "str_fmt expects the first argument to be a format string");
  }
  std::string fmt_str = vm->heap.get_string(fmt_str_val.as.STRING);

  std::string result = "";
  size_t arg_idx = 1;
  for (size_t i = 0; i < fmt_str.length(); ++i) {
    if (fmt_str[i] == '{' && i + 1 < fmt_str.length() &&
        fmt_str[i + 1] == '}') {
      if (arg_idx >= args.size()) {
        throw msl_runtime_error(where,
                                "not enough arguments for format string");
      }
      result +=
          core_utils::value_to_string(&vm->stack, &vm->heap, args[arg_idx]);
      ++arg_idx;
      ++i; // Skip the '}'
    } else {
      result += fmt_str[i];
    }
  }

  if (arg_idx < args.size()) {
    throw msl_runtime_error(where, "too many arguments for format string");
  }

  return vm->heap.add_string(result);
}

Value core::str_url_encode(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  if (val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "str_url_encode: expected a string");
  }
  std::string str = vm->heap.get_string(val.as.STRING);
  std::string res = "";
  for (unsigned char c : str) {
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      res += c;
    } else {
      char buf[4];
      SNPRINTF(buf, sizeof(buf), "%%%02X", c);
      res += buf;
    }
  }
  return vm->heap.add_string(res);
}

Value core::str_url_decode(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  if (val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "str_url_decode: expected a string");
  }
  std::string str = vm->heap.get_string(val.as.STRING);
  std::string res = "";
  for (size_t i = 0; i < str.length(); ++i) {
    if (str[i] == '%' && i + 2 < str.length()) {
      std::string hex = str.substr(i + 1, 2);
      try {
        res += (char)std::stoi(hex, nullptr, 16);
        i += 2;
      } catch (...) {
        res += str[i];
      }
    } else if (str[i] == '+') {
      res += ' ';
    } else {
      res += str[i];
    }
  }
  return vm->heap.add_string(res);
}

Value core::fs_exists(LocationRef where, VM *vm) {
  Value path_val = vm->stack.pop();
  if (path_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected a string for file path");
  }
  std::string path = vm->heap.get_string(path_val.as.STRING);
  bool exists = std::filesystem::exists(path);
  return core_utils::to_bool(exists);
}

Value core::fs_mkdir(LocationRef where, VM *vm) {
  Value path_val = vm->stack.pop();
  if (path_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected a string for directory path");
  }
  std::string path = vm->heap.get_string(path_val.as.STRING);
  try {
    if (!std::filesystem::create_directory(path)) {
      return core_utils::create_error(&vm->heap, "failed to create directory");
    }
  } catch (const std::filesystem::filesystem_error &e) {
    return core_utils::create_error(&vm->heap, "failed to create directory: " +
                                                   std::string(e.what()));
  }
  return Value::None();
}

Value core::fs_rm(LocationRef where, VM *vm) {
  Value path_val = vm->stack.pop();
  if (path_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected a string for path");
  }
  std::string path = vm->heap.get_string(path_val.as.STRING);
  try {
    if (!std::filesystem::remove(path)) {
      return core_utils::create_error(&vm->heap, "file or directory not found");
    }
  } catch (const std::filesystem::filesystem_error &e) {
    return core_utils::create_error(&vm->heap, "failed to remove: " +
                                                   std::string(e.what()));
  }
  return Value::None();
}

Value core::fs_ls(LocationRef where, VM *vm) {
  Value path_val = vm->stack.pop();
  if (path_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected a string for path");
  }
  std::string path = vm->heap.get_string(path_val.as.STRING);
  VMHIDX list_head = vm->heap.new_list();
  try {
    for (const auto &entry : std::filesystem::directory_iterator(path)) {
      vm->heap.add_child(list_head, vm->heap.add_string(entry.path().string()));
    }
  } catch (const std::filesystem::filesystem_error &e) {
    return core_utils::create_error(&vm->heap,
                                    "failed to list directory contents: " +
                                        std::string(e.what()));
  }
  return vm->heap.at(list_head);
}

Value core::fs_copy(LocationRef where, VM *vm) {
  Value dst_val = vm->stack.pop();
  Value src_val = vm->stack.pop();
  if (src_val.tag != ValueTag::STRING || dst_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings for src and dst paths");
  }
  std::string src = vm->heap.get_string(src_val.as.STRING);
  std::string dst = vm->heap.get_string(dst_val.as.STRING);
  try {
    std::filesystem::copy(src, dst, std::filesystem::copy_options::recursive);
  } catch (const std::filesystem::filesystem_error &e) {
    return core_utils::create_error(&vm->heap,
                                    "failed to copy: " + std::string(e.what()));
  }
  return Value::None();
}

Value core::fs_move(LocationRef where, VM *vm) {
  Value dst_val = vm->stack.pop();
  Value src_val = vm->stack.pop();
  if (src_val.tag != ValueTag::STRING || dst_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "expected strings for src and dst paths");
  }
  std::string src = vm->heap.get_string(src_val.as.STRING);
  std::string dst = vm->heap.get_string(dst_val.as.STRING);
  try {
    std::filesystem::rename(src, dst);
  } catch (const std::filesystem::filesystem_error &e) {
    return core_utils::create_error(&vm->heap,
                                    "failed to move: " + std::string(e.what()));
  }
  return Value::None();
}

Value core::sys_now(LocationRef, VM *) {
  auto now = chrono::high_resolution_clock::now();
  auto nanos =
      chrono::duration_cast<chrono::nanoseconds>(now.time_since_epoch())
          .count();
  return Value::Int(nanos);
}

Value core::bit_shift_left(LocationRef where, VM *vm) {
  Value n = vm->stack.pop(); // right
  Value i = vm->stack.pop(); // left
  core_utils::assert_int(where, i);
  core_utils::assert_int(where, n);
  return Value::Int(i.as.INT << n.as.INT);
}

Value core::bit_shift_right(LocationRef where, VM *vm) {
  Value n = vm->stack.pop(); // right
  Value i = vm->stack.pop(); // left
  core_utils::assert_int(where, i);
  core_utils::assert_int(where, n);
  return Value::Int(i.as.INT >> n.as.INT);
}

Value core::bit_or(LocationRef where, VM *vm) {
  Value j = vm->stack.pop(); // right
  Value i = vm->stack.pop(); // left
  core_utils::assert_int(where, i);
  core_utils::assert_int(where, j);
  return Value::Int(i.as.INT | j.as.INT);
}

Value core::bit_and(LocationRef where, VM *vm) {
  Value j = vm->stack.pop(); // right
  Value i = vm->stack.pop(); // left
  core_utils::assert_int(where, i);
  core_utils::assert_int(where, j);
  return Value::Int(i.as.INT & j.as.INT);
}

Value core::bit_xor(LocationRef where, VM *vm) {
  Value j = vm->stack.pop(); // right
  Value i = vm->stack.pop(); // left
  core_utils::assert_int(where, i);
  core_utils::assert_int(where, j);
  return Value::Int(i.as.INT ^ j.as.INT);
}

Value core::ansi_color(LocationRef where, VM *vm) {
  Value color_val = vm->stack.pop();
  Value type_val = vm->stack.pop();

  if (type_val.tag != ValueTag::SYMBOL || color_val.tag != ValueTag::SYMBOL) {
    throw msl_runtime_error(where, "ansi_color expects two symbols");
  }

  bool is_fg = type_val.as.SYMBOL.index == Constants::SYM_ANSI_FG.index;
  bool is_bg = type_val.as.SYMBOL.index == Constants::SYM_ANSI_BG.index;

  if (!is_fg && !is_bg) {
    throw msl_runtime_error(where,
                            "ansi_color: first argument must be #fg or #bg");
  }

  int color_code = -1;
  uint64_t sym_idx = color_val.as.SYMBOL.index;

  if (sym_idx == Constants::SYM_ANSI_BLACK.index)
    color_code = 0;
  else if (sym_idx == Constants::SYM_ANSI_RED.index)
    color_code = 1;
  else if (sym_idx == Constants::SYM_ANSI_GREEN.index)
    color_code = 2;
  else if (sym_idx == Constants::SYM_ANSI_YELLOW.index)
    color_code = 3;
  else if (sym_idx == Constants::SYM_ANSI_BLUE.index)
    color_code = 4;
  else if (sym_idx == Constants::SYM_ANSI_MAGENTA.index)
    color_code = 5;
  else if (sym_idx == Constants::SYM_ANSI_CYAN.index)
    color_code = 6;
  else if (sym_idx == Constants::SYM_ANSI_WHITE.index)
    color_code = 7;
  else if (sym_idx == Constants::SYM_ANSI_BRIGHT_BLACK.index)
    color_code = 60;
  else if (sym_idx == Constants::SYM_ANSI_BRIGHT_RED.index)
    color_code = 61;
  else if (sym_idx == Constants::SYM_ANSI_BRIGHT_GREEN.index)
    color_code = 62;
  else if (sym_idx == Constants::SYM_ANSI_BRIGHT_YELLOW.index)
    color_code = 63;
  else if (sym_idx == Constants::SYM_ANSI_BRIGHT_BLUE.index)
    color_code = 64;
  else if (sym_idx == Constants::SYM_ANSI_BRIGHT_MAGENTA.index)
    color_code = 65;
  else if (sym_idx == Constants::SYM_ANSI_BRIGHT_CYAN.index)
    color_code = 66;
  else if (sym_idx == Constants::SYM_ANSI_BRIGHT_WHITE.index)
    color_code = 67;

  if (color_code == -1) {
    throw msl_runtime_error(where, "ansi_color: unknown color symbol: " +
                                       resolve_symbol(color_val.as.SYMBOL));
  }

  int final_code = (is_fg ? 30 : 40) + color_code;
  std::cout << "\033[" << final_code << "m";

  return Value::None();
}

Value core::ansi_reset(LocationRef, VM *) {
  std::cout << "\033[0m";
  return Value::None();
}

Value core::sys_is_tty(LocationRef, VM *) {
  bool tty = ISATTY(STDOUT_FILENO);
  return core_utils::to_bool(tty);
}

Value core::sys_has_color(LocationRef, VM *) {
  if (!ISATTY(STDOUT_FILENO)) {
    return Value::FromSymbol(Constants::SYM_FALSE);
  }

  const char *colorterm = std::getenv("COLORTERM");
  if (colorterm != nullptr) {
    return Value::FromSymbol(Constants::SYM_TRUE);
  }

  const char *term = std::getenv("TERM");
  if (term != nullptr) {
    std::string term_str(term);
    if (term_str == "dumb") {
      return Value::FromSymbol(Constants::SYM_FALSE);
    }
    return Value::FromSymbol(Constants::SYM_TRUE);
  }

  return Value::FromSymbol(Constants::SYM_FALSE);
}

Value core::sys_term_width(LocationRef, VM *vm) {
  core_utils::TerminalSize size = core_utils::get_terminal_size();
  if (size.success) {
    return Value::Int(size.width);
  } else {
    return core_utils::create_error(&vm->heap,
                                    "failed to get terminal dimensions");
  }
}
Value core::sys_term_height(LocationRef, VM *vm) {
  core_utils::TerminalSize size = core_utils::get_terminal_size();
  if (size.success) {
    return Value::Int(size.height);
  } else {
    return core_utils::create_error(&vm->heap,
                                    "failed to get terminal dimensions");
  }
}

Value core::ansi_set_cursor(LocationRef where, VM *vm) {
  Value col_val = vm->stack.pop();
  Value row_val = vm->stack.pop();

  if (row_val.tag == ValueTag::INT && col_val.tag == ValueTag::INT) {
    std::cout << "\033[" << (row_val.as.INT + 1) << ";" << (col_val.as.INT + 1)
              << "H";
  } else if (row_val.tag == ValueTag::INT && col_val.tag == ValueTag::NONE) {
    std::cout << "\033[" << (row_val.as.INT + 1) << ";1H";
  } else if (row_val.tag == ValueTag::NONE && col_val.tag == ValueTag::INT) {
    std::cout << "\033[" << (col_val.as.INT + 1) << "G";
  } else if (row_val.tag != ValueTag::NONE || col_val.tag != ValueTag::NONE) {
    throw msl_runtime_error(where,
                            "ansi_set_cursor expects (int|none, int|none)");
  }
  std::cout.flush();
  return Value::None();
}

Value core::ansi_move_cursor(LocationRef where, VM *vm) {
  Value dc_val = vm->stack.pop();
  Value dr_val = vm->stack.pop();

  if (dr_val.tag == ValueTag::INT) {
    int64_t dr = dr_val.as.INT;
    if (dr > 0)
      std::cout << "\033[" << dr << "B";
    else if (dr < 0)
      std::cout << "\033[" << -dr << "A";
  } else if (dr_val.tag != ValueTag::NONE) {
    throw msl_runtime_error(where,
                            "ansi_move_cursor expects (int|none, int|none)");
  }

  if (dc_val.tag == ValueTag::INT) {
    int64_t dc = dc_val.as.INT;
    if (dc > 0)
      std::cout << "\033[" << dc << "C";
    else if (dc < 0)
      std::cout << "\033[" << -dc << "D";
  } else if (dc_val.tag != ValueTag::NONE) {
    throw msl_runtime_error(where,
                            "ansi_move_cursor expects (int|none, int|none)");
  }

  std::cout.flush();
  return Value::None();
}

Value core::ansi_clear_line(LocationRef, VM *) {
  std::cout << "\033[2K"; // Clear entire line
  std::cout << "\033[G";  // Move to beginning of line
  std::cout.flush();
  return Value::None();
}

Value core::ansi_clear_screen(LocationRef, VM *) {
  std::cout << "\033[2J"; // Clear entire screen
  std::cout << "\033[H";  // Move to home (1,1)
  std::cout.flush();
  return Value::None();
}

Value core::ansi_clear(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  if (val.tag != ValueTag::SYMBOL) {
    throw msl_runtime_error(where, "ansi_clear expects a symbol");
  }

  uint64_t sym_idx = val.as.SYMBOL.index;

  if (sym_idx == Constants::SYM_ANSI_LINE.index) {
    std::cout << "\033[2K";
  } else if (sym_idx == Constants::SYM_ANSI_LINE_FROM_START.index) {
    std::cout << "\033[1K";
  } else if (sym_idx == Constants::SYM_ANSI_LINE_TO_END.index) {
    std::cout << "\033[0K";
  } else if (sym_idx == Constants::SYM_ANSI_SCREEN.index) {
    std::cout << "\033[2J";
  } else if (sym_idx == Constants::SYM_ANSI_SCREEN_FROM_START.index) {
    std::cout << "\033[1J";
  } else if (sym_idx == Constants::SYM_ANSI_SCREEN_TO_END.index) {
    std::cout << "\033[0J";
  } else {
    throw msl_runtime_error(where, "ansi_clear: unknown clear symbol: " +
                                       resolve_symbol(val.as.SYMBOL));
  }

  std::cout.flush();
  return Value::None();
}

Value core::regex_has_match(LocationRef where, VM *vm) {
  Value regexp_val = vm->stack.pop();
  Value str_val = vm->stack.pop();
  if (str_val.tag != ValueTag::STRING || regexp_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "regex_has_match: expected two strings");
  }
  bool has_match =
      std::regex_search(vm->heap.get_string(str_val.as.STRING),
                        std::regex(vm->heap.get_string(regexp_val.as.STRING)));
  return core_utils::to_bool(has_match);
}

Value core::regex_match(LocationRef where, VM *vm) {
  Value regexp_val = vm->stack.pop();
  Value str_val = vm->stack.pop();
  if (str_val.tag != ValueTag::STRING || regexp_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "regex_has_match: expected two strings");
  }
  std::smatch result;
  std::string string_to_search = vm->heap.get_string(str_val.as.STRING);
  std::regex regex = std::regex(vm->heap.get_string(regexp_val.as.STRING));
  std::regex_search(string_to_search, result, regex);

  VMHIDX list_head = vm->heap.new_list();
  for (size_t i = 0; i < result.size(); ++i) {
    vm->heap.add_child(list_head, vm->heap.add_string(result[i]));
  }
  return Value::List(list_head);
}

Value core::regex_replace(LocationRef where, VM *vm) {
  Value replacement_val = vm->stack.pop();
  Value regexp_val = vm->stack.pop();
  Value str_val = vm->stack.pop();
  if (replacement_val.tag != ValueTag::STRING ||
      str_val.tag != ValueTag::STRING || regexp_val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where,
                            "regex_replace: expected three string arguments");
  }
  std::string str = vm->heap.get_string(str_val.as.STRING);
  std::string replacement = vm->heap.get_string(replacement_val.as.STRING);
  std::regex regex = std::regex(vm->heap.get_string(regexp_val.as.STRING));
  std::string result = std::regex_replace(str, regex, replacement);
  return Value::String(vm->heap.ref_add_string(result));
}

namespace _base64 {

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";
std::string base64_encode_str(const std::string &in) {
  std::string out;
  uint32_t val = 0;
  int valb = -6;
  for (unsigned char c : in) {
    val = (val << 8) + c;
    valb += 8;
    while (valb >= 0) {
      out.push_back(base64_chars[(val >> valb) & 0x3F]);
      valb -= 6;
    }
  }
  if (valb > -6)
    out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
  while (out.size() % 4)
    out.push_back('=');
  return out;
}

std::string base64_decode_str(const std::string &in) {
  std::vector<int> T(256, -1);
  for (int i = 0; i < 64; i++)
    T[(unsigned char)base64_chars[i]] = i;

  std::string out;
  uint32_t val = 0;
  int valb = -8;
  for (unsigned char c : in) {
    if (T[c] == -1)
      continue;
    val = (val << 6) + T[c];
    valb += 6;
    if (valb >= 0) {
      out.push_back(char((val >> valb) & 0xFF));
      valb -= 8;
    }
  }
  return out;
}
} // namespace _base64

Value core::base64_encode(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  if (val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "base64_encode: expected a string");
  }
  std::string str = vm->heap.get_string(val.as.STRING);
  return vm->heap.add_string(_base64::base64_encode_str(str));
}

Value core::base64_decode(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  if (val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "base64_decode: expected a string");
  }
  std::string str = vm->heap.get_string(val.as.STRING);
  return vm->heap.add_string(_base64::base64_decode_str(str));
}

Value core::hex_encode(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  if (val.tag != ValueTag::INT) {
    throw msl_runtime_error(where, "hex_encode: expected an integer");
  }
  if (val.as.INT < 0) {
    throw msl_runtime_error(where, "hex_encode: expected a positive integer");
  }
  std::stringstream ss;
  ss << std::hex << val.as.INT;
  return vm->heap.add_string(ss.str());
}

Value core::hex_decode(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  if (val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "hex_decode: expected a string");
  }
  std::string str = vm->heap.get_string(val.as.STRING);
  try {
    return Value::Int(std::stoll(str, nullptr, 16));
  } catch (...) {
    throw msl_runtime_error(where, "hex_decode: invalid hex string: " + str);
  }
}

Value core::binary_encode(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  if (val.tag != ValueTag::INT) {
    throw msl_runtime_error(where, "binary_encode: expected an integer");
  }
  if (val.as.INT < 0) {
    throw msl_runtime_error(where,
                            "binary_encode: expected a positive integer");
  }
  if (val.as.INT == 0) {
    return vm->heap.add_string("0");
  }
  std::string res = "";
  uint64_t n = static_cast<uint64_t>(val.as.INT);
  while (n > 0) {
    res = ((n % 2 == 0) ? "0" : "1") + res;
    n /= 2;
  }
  return vm->heap.add_string(res);
}

Value core::binary_decode(LocationRef where, VM *vm) {
  Value val = vm->stack.pop();
  if (val.tag != ValueTag::STRING) {
    throw msl_runtime_error(where, "binary_decode: expected a string");
  }
  std::string str = vm->heap.get_string(val.as.STRING);
  try {
    return Value::Int(std::stoll(str, nullptr, 2));
  } catch (...) {
    throw msl_runtime_error(where,
                            "binary_decode: invalid binary string: " + str);
  }
}

Value core::box_create(LocationRef, VM *vm) {
  Value val = vm->stack.pop();
  return vm->heap.new_box(val);
}
Value core::box_pack(LocationRef where, VM *vm) {
  Value value = vm->stack.pop();
  Value box = vm->stack.pop();
  if (box.tag != ValueTag::BOX) {
    throw msl_runtime_error(where,
                            "box_pack: expected a box as first argument");
  }
  return vm->heap.box_pack(box.as.BOX, value);
}
Value core::box_unpack(LocationRef where, VM *vm) {
  Value box = vm->stack.pop();
  if (box.tag != ValueTag::BOX) {
    throw msl_runtime_error(where,
                            "box_pack: expected a box as first argument");
  }
  return vm->heap.box_unpack(box.as.BOX);
}
