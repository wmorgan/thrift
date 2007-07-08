#include <stdlib.h>
#include <sys/stat.h>
#include <sstream>
#include "t_cpp_generator.h"
using namespace std;

/**
 * Prepares for file generation by opening up the necessary file output
 * streams.
 *
 * @param tprogram The program to generate
 */
void t_cpp_generator::init_generator() {
  // Make output directory
  mkdir(T_CPP_DIR, S_IREAD | S_IWRITE | S_IEXEC);

  // Make output file
  string f_types_name = string(T_CPP_DIR)+"/"+program_name_+"_types.h";
  f_types_.open(f_types_name.c_str());

  string f_types_impl_name = string(T_CPP_DIR)+"/"+program_name_+"_types.cpp";
  f_types_impl_.open(f_types_impl_name.c_str());

  // Print header
  f_types_ <<
    autogen_comment();
  f_types_impl_ <<
    autogen_comment();

  // Start ifndef
  f_types_ <<
    "#ifndef " << program_name_ << "_TYPES_H" << endl <<
    "#define " << program_name_ << "_TYPES_H" << endl <<
    endl;
  
  // Include base types
  f_types_ <<
    "#include <Thrift.h>" << endl <<
    "#include <protocol/TProtocol.h>" << endl <<
    "#include <transport/TTransport.h>" << endl <<
    endl;

  // Include other Thrift includes
  const vector<t_program*>& includes = program_->get_includes();
  for (size_t i = 0; i < includes.size(); ++i) {
    f_types_ <<
      "#include \"" << includes[i]->get_name() << "_types.h\"" << endl;
  }
  f_types_ << endl;

  // Include custom headers
  const vector<string>& cpp_includes = program_->get_cpp_includes();
  for (size_t i = 0; i < cpp_includes.size(); ++i) {
    f_types_ <<
      "#include \"" << cpp_includes[i] << "\"" << endl;
  }
  f_types_ <<
    endl;
  
  // Include the types file
  f_types_impl_ <<
    "#include \"" << program_name_ << "_types.h\"" << endl <<
    endl;

  // Open namespace
  ns_open_ = namespace_open(program_->get_cpp_namespace());
  ns_close_ = namespace_close(program_->get_cpp_namespace());

  f_types_ <<
    ns_open_ << endl <<
    endl;

  f_types_impl_ <<
    ns_open_ << endl <<
    endl;
}

/**
 * Closes the output files.
 */
void t_cpp_generator::close_generator() {
  // Close namespace
  f_types_ <<
    ns_close_ << endl <<
    endl;
  f_types_impl_ <<
    ns_close_ << endl;

  // Close ifndef
  f_types_ <<
    "#endif" << endl;
 
  // Close output file
  f_types_.close();
  f_types_impl_.close();
}

/**
 * Generates a typedef. This is just a simple 1-liner in C++
 *
 * @param ttypedef The type definition
 */
void t_cpp_generator::generate_typedef(t_typedef* ttypedef) {
  f_types_ <<
    indent() << "typedef " << type_name(ttypedef->get_type(), true) << " " << ttypedef->get_symbolic() << ";" << endl <<
    endl;
}

/**
 * Generates code for an enumerated type. In C++, this is essentially the same
 * as the thrift definition itself, using the enum keyword in C++.
 *
 * @param tenum The enumeration
 */
void t_cpp_generator::generate_enum(t_enum* tenum) {
  f_types_ <<
    indent() << "enum " << tenum->get_name() << " {" << endl;
  indent_up();

  vector<t_enum_value*> constants = tenum->get_constants();
  vector<t_enum_value*>::iterator c_iter;
  bool first = true;
  for (c_iter = constants.begin(); c_iter != constants.end(); ++c_iter) {
    if (first) {
      first = false;
    } else {
      f_types_ <<
        "," << endl;
    }
    f_types_ <<
      indent() << (*c_iter)->get_name();
    if ((*c_iter)->has_value()) {
      f_types_ <<
        " = " << (*c_iter)->get_value();
    }
  }

  indent_down();
  f_types_ <<
    endl <<
    "};" << endl <<
    endl;
}

/**
 * Generates a class that holds all the constants.
 */
void t_cpp_generator::generate_consts(std::vector<t_const*> consts) {
  string f_consts_name = string(T_CPP_DIR)+"/"+program_name_+"_constants.h";
  ofstream f_consts;
  f_consts.open(f_consts_name.c_str());

  string f_consts_impl_name = string(T_CPP_DIR)+"/"+program_name_+"_constants.cpp";
  ofstream f_consts_impl;
  f_consts_impl.open(f_consts_impl_name.c_str());

  // Print header
  f_consts <<
    autogen_comment();
  f_consts_impl <<
    autogen_comment();

  // Start ifndef
  f_consts <<
    "#ifndef " << program_name_ << "_CONSTANTS_H" << endl <<
    "#define " << program_name_ << "_CONSTANTS_H" << endl <<
    endl <<
    "#include \"" << program_name_ << "_types.h\"" << endl <<
    endl <<
    ns_open_ << endl <<
    endl;

  f_consts_impl <<
    "#include \"" << program_name_ << "_constants.h\"" << endl <<
    endl <<
    ns_open_ << endl <<
    endl;

  f_consts <<
    "class " << program_name_ << "Constants {" << endl <<
    " public:" << endl <<
    "  " << program_name_ << "Constants();" << endl <<
    endl;
  indent_up();
  vector<t_const*>::iterator c_iter;
  for (c_iter = consts.begin(); c_iter != consts.end(); ++c_iter) {
    string name = (*c_iter)->get_name();
    t_type* type = (*c_iter)->get_type();
    f_consts <<
      indent() << type_name(type) << " " << name << ";" << endl;
  }
  indent_down();
  f_consts <<
    "};" << endl;  

  f_consts_impl <<
    "const " << program_name_ << "Constants g_" << program_name_ << "_constants;" << endl <<
    endl <<
    program_name_ << "Constants::" << program_name_ << "Constants() {" << endl;
  indent_up();
  for (c_iter = consts.begin(); c_iter != consts.end(); ++c_iter) {
    print_const_value(f_consts_impl,
                      (*c_iter)->get_name(),
                      (*c_iter)->get_type(),
                      (*c_iter)->get_value());
  }
  indent_down();
  indent(f_consts_impl) <<
    "}" << endl;

  f_consts <<
    endl <<
    "extern const " << program_name_ << "Constants g_" << program_name_ << "_constants;" << endl <<
    endl <<
    ns_close_ << endl <<
    endl <<
    "#endif" << endl;
  f_consts.close();

  f_consts_impl <<
    endl <<
    ns_close_ << endl <<
    endl;
}

/**
 * Prints the value of a constant with the given type. Note that type checking
 * is NOT performed in this function as it is always run beforehand using the
 * validate_types method in main.cc
 */
void t_cpp_generator::print_const_value(ofstream& out, string name, t_type* type, t_const_value* value) {
  if (type->is_base_type()) {
    string v2 = render_const_value(out, name, type, value);
    indent(out) << name << " = " << v2 << ";" << endl <<
      endl;
  } else if (type->is_enum()) {
    indent(out) << name << " = (" << type->get_name() << ")" << value->get_integer() << ";" << endl <<
      endl;
  } else if (type->is_struct() || type->is_xception()) {
    const vector<t_field*>& fields = ((t_struct*)type)->get_members();
    vector<t_field*>::const_iterator f_iter;
    const map<t_const_value*, t_const_value*>& val = value->get_map();
    map<t_const_value*, t_const_value*>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      t_type* field_type = NULL;
      for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
        if ((*f_iter)->get_name() == v_iter->first->get_string()) {
          field_type = (*f_iter)->get_type();
        }
      }
      if (field_type == NULL) {
        throw "type error: " + type->get_name() + " has no field " + v_iter->first->get_string();
      }
      string val = render_const_value(out, name, field_type, v_iter->second);
      indent(out) << name << "." << v_iter->first->get_string() << " = " << val << ";" << endl;
      indent(out) << name << ".__isset." << v_iter->first->get_string() << " = true;" << endl;
    }
    out << endl;
  } else if (type->is_map()) {
    t_type* ktype = ((t_map*)type)->get_key_type();
    t_type* vtype = ((t_map*)type)->get_val_type();
    const map<t_const_value*, t_const_value*>& val = value->get_map();
    map<t_const_value*, t_const_value*>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      string key = render_const_value(out, name, ktype, v_iter->first);
      string val = render_const_value(out, name, vtype, v_iter->second);
      indent(out) << name << "[" << key << "] = " << val << ";" << endl;
    }
    out << endl;
  } else if (type->is_list()) {
    t_type* etype = ((t_list*)type)->get_elem_type();
    const vector<t_const_value*>& val = value->get_list();
    vector<t_const_value*>::const_iterator v_iter;
    int i = 0;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      string val = render_const_value(out, name, etype, *v_iter);
      indent(out) << name << "[" << (i++) << "] = " << val << ";" << endl;
    }
    out << endl;
  } else if (type->is_set()) {
    t_type* etype = ((t_set*)type)->get_elem_type();
    const vector<t_const_value*>& val = value->get_list();
    vector<t_const_value*>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      string val = render_const_value(out, name, etype, *v_iter);
      indent(out) << name << ".insert(" << val << ");" << endl;
    }
    out << endl;
  }
}

/**
 *
 */
string t_cpp_generator::render_const_value(ofstream& out, string name, t_type* type, t_const_value* value) {
  std::ostringstream render;

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
    case t_base_type::TYPE_STRING:
      render << "\"" + value->get_string() + "\"";
      break;
    case t_base_type::TYPE_BOOL:
      render << ((value->get_integer() > 0) ? "true" : "false");
      break;
    case t_base_type::TYPE_BYTE:
    case t_base_type::TYPE_I16:
    case t_base_type::TYPE_I32:
    case t_base_type::TYPE_I64:
      render << value->get_integer();
      break;
    case t_base_type::TYPE_DOUBLE:
      if (value->get_type() == t_const_value::CV_INTEGER) {
        render << value->get_integer();
      } else {
        render << value->get_double();
      }
      break;
    default:
      throw "compiler error: no const of base type " + tbase;
    }
  } else if (type->is_enum()) {
    render << value->get_integer();
  } else {
    string t = tmp("tmp");
    indent(out) << type_name(type) << " " << t << ";" << endl;
    print_const_value(out, t, type, value);
    render << t;
  }

  return render.str();
}

/**
 * Generates a struct definition for a thrift data type. This is a class
 * with data members and a read/write() function, plus a mirroring isset
 * inner class.
 *
 * @param tstruct The struct definition
 */
void t_cpp_generator::generate_cpp_struct(t_struct* tstruct, bool is_exception) {
  generate_struct_definition(f_types_, tstruct, is_exception);
  generate_struct_reader(f_types_impl_, tstruct);
  generate_struct_writer(f_types_impl_, tstruct);
}

/**
 * Writes the struct definition into the header file
 *
 * @param out Output stream
 * @param tstruct The struct
 */
void t_cpp_generator::generate_struct_definition(ofstream& out,
                                                 t_struct* tstruct,
                                                 bool is_exception,
                                                 bool pointers,
                                                 bool read,
                                                 bool write) {
  string extends = "";
  if (is_exception) {
    extends = " : public facebook::thrift::TException";
  }

  // Open struct def
  out <<
    indent() << "class " << tstruct->get_name() << extends << " {" << endl <<
    indent() << " public:" << endl <<
    endl;
  indent_up();

  // Get members
  vector<t_field*>::const_iterator m_iter; 
  const vector<t_field*>& members = tstruct->get_members();
  
  if (!pointers) {
    // Default constructor
    indent(out) <<
      tstruct->get_name() << "()";

    bool init_ctor = false;

    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
      t_type* t = (*m_iter)->get_type();
      while (t->is_typedef()) {
        t = ((t_typedef*)t)->get_type();
      }
      if (t->is_base_type() || t->is_enum()) {
        string dval;
        if (t->is_enum()) {
          dval += "(" + t->get_name() + ")";
        }
        dval += t->is_string() ? "\"\"" : "0";
        t_const_value* cv = (*m_iter)->get_value();
        if (cv != NULL) {
          dval = render_const_value(out, (*m_iter)->get_name(), t, cv);
        }
        if (!init_ctor) {
          init_ctor = true;
          out << " : ";
          out << (*m_iter)->get_name() << "(" << dval << ")";
        } else {
          out << ", " << (*m_iter)->get_name() << "(" << dval << ")";
        }
      }
    }
    out << " {" << endl;
    indent_up();
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
      t_type* t = (*m_iter)->get_type();
      while (t->is_typedef()) {
        t = ((t_typedef*)t)->get_type();
      }
      if (!t->is_base_type()) {
        t_const_value* cv = (*m_iter)->get_value();
        if (cv != NULL) {
          print_const_value(out, (*m_iter)->get_name(), t, cv);
        }
      }
    }
    indent_down();
    indent(out) << "} " << endl;
  }
  
  out <<
    endl <<
    indent() << "virtual ~" << tstruct->get_name() << "() throw() {}" << endl << endl;

  // Declare all fields
  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    indent(out) <<
      declare_field(*m_iter, false, pointers && !(*m_iter)->get_type()->is_xception(), !read) << endl;
  }
  
  // Isset struct has boolean fields
  if (members.size() > 0 && (!pointers || read)) {
    out <<
      endl <<
      indent() <<"struct __isset {" << endl;
    indent_up();
      
      indent(out) <<
        "__isset() : ";
      bool first = true;
      for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
        if (first) {
          first = false;
          out <<
            (*m_iter)->get_name() << "(false)";
        } else {
          out <<
            ", " << (*m_iter)->get_name() << "(false)";
        }
      }
      out << " {}" << endl;
      
      for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
        indent(out) <<
          "bool " << (*m_iter)->get_name() << ";" << endl;
      }

    indent_down();
    indent(out) <<
      "} __isset;" << endl;  
  }

  out <<
    endl;
  if (read) {
    out <<
      indent() << "uint32_t read(facebook::thrift::protocol::TProtocol* iprot);" << endl;
  }
  if (write) {
    out <<
      indent() << "uint32_t write(facebook::thrift::protocol::TProtocol* oprot) const;" << endl;
  }
  out <<
    endl;

  indent_down(); 
  indent(out) <<
    "};" << endl <<
    endl;
}

/**
 * Makes a helper function to gen a struct reader.
 *
 * @param out Stream to write to
 * @param tstruct The struct
 */
void t_cpp_generator::generate_struct_reader(ofstream& out,
                                             t_struct* tstruct,
                                             bool pointers) {
  indent(out) <<
    "uint32_t " << tstruct->get_name() << "::read(facebook::thrift::protocol::TProtocol* iprot) {" << endl;
  indent_up();

  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;

  // Declare stack tmp variables
  out <<
    endl << 
    indent() << "uint32_t xfer = 0;" << endl <<
    indent() << "std::string fname;" << endl <<
    indent() << "facebook::thrift::protocol::TType ftype;" << endl <<
    indent() << "int16_t fid;" << endl <<
    endl <<
    indent() << "xfer += iprot->readStructBegin(fname);" << endl <<
    endl;
  
  // Loop over reading in fields
  indent(out) <<
    "while (true)" << endl;
    scope_up(out);
    
    // Read beginning field marker
    indent(out) <<
      "xfer += iprot->readFieldBegin(fname, ftype, fid);" << endl;
    
    // Check for field STOP marker
    out <<
      indent() << "if (ftype == facebook::thrift::protocol::T_STOP) { " << endl <<
      indent() << "  break;" << endl <<
      indent() << "}" << endl;
    
    // Switch statement on the field we are reading
    indent(out) <<
      "switch (fid)" << endl;

      scope_up(out);
    
      // Generate deserialization code for known cases
      for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
        indent(out) <<
          "case " << (*f_iter)->get_key() << ":" << endl;
        indent_up();
        indent(out) <<
          "if (ftype == " << type_to_enum((*f_iter)->get_type()) << ") {" << endl;
        indent_up();

        if (pointers && !(*f_iter)->get_type()->is_xception()) {
          generate_deserialize_field(out, *f_iter, "(*(this->", "))");
        } else {
          generate_deserialize_field(out, *f_iter, "this->");
        }
        out <<
          indent() << "this->__isset." << (*f_iter)->get_name() << " = true;" << endl;
        indent_down();
        out <<
          indent() << "} else {" << endl <<
          indent() << "  xfer += iprot->skip(ftype);" << endl <<
          indent() << "}" << endl <<
          indent() << "break;" << endl;
        indent_down();
      }
      
      // In the default case we skip the field
      out <<
        indent() << "default:" << endl <<
        indent() << "  xfer += iprot->skip(ftype);" << endl <<
        indent() << "  break;" << endl;
      
      scope_down(out);

    // Read field end marker
    indent(out) <<
      "xfer += iprot->readFieldEnd();" << endl;

    scope_down(out);

  out <<
    endl <<
    indent() << "xfer += iprot->readStructEnd();" << endl <<
    indent() <<"return xfer;" << endl;

  indent_down();
  indent(out) <<
    "}" << endl << endl;
}

/**
 * Generates the write function.
 *
 * @param out Stream to write to
 * @param tstruct The struct
 */
void t_cpp_generator::generate_struct_writer(ofstream& out,
                                             t_struct* tstruct,
                                             bool pointers) {
  string name = tstruct->get_name();
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;

  indent(out) <<
    "uint32_t " << tstruct->get_name() << "::write(facebook::thrift::protocol::TProtocol* oprot) const {" << endl;
  indent_up();

  out <<
    indent() << "uint32_t xfer = 0;" << endl;

  indent(out) <<
    "xfer += oprot->writeStructBegin(\"" << name << "\");" << endl;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    // Write field header
    out <<
      indent() << "xfer += oprot->writeFieldBegin(" <<
      "\"" << (*f_iter)->get_name() << "\", " <<
      type_to_enum((*f_iter)->get_type()) << ", " <<
      (*f_iter)->get_key() << ");" << endl;
    // Write field contents
    if (pointers) {
      generate_serialize_field(out, *f_iter, "(*(this->", "))");
    } else {
      generate_serialize_field(out, *f_iter, "this->");
    }
    // Write field closer
    indent(out) <<
      "xfer += oprot->writeFieldEnd();" << endl;
  }
  // Write the struct map
  out <<
    indent() << "xfer += oprot->writeFieldStop();" << endl <<
    indent() << "xfer += oprot->writeStructEnd();" << endl <<
    indent() << "return xfer;" << endl;

  indent_down();
  indent(out) <<
    "}" << endl <<
    endl;
}

/**
 * Struct writer for result of a function, which can have only one of its
 * fields set and does a conditional if else look up into the __isset field
 * of the struct.
 *
 * @param out Output stream
 * @param tstruct The result struct
 */
void t_cpp_generator::generate_struct_result_writer(ofstream& out,
                                                    t_struct* tstruct,
                                                    bool pointers) {
  string name = tstruct->get_name();
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;

  indent(out) <<
    "uint32_t " << tstruct->get_name() << "::write(facebook::thrift::protocol::TProtocol* oprot) const {" << endl;
  indent_up();

  out <<
    endl <<
    indent() << "uint32_t xfer = 0;" << endl <<
    endl;

  indent(out) <<
    "xfer += oprot->writeStructBegin(\"" << name << "\");" << endl;

  bool first = true;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
      out << 
        endl <<
        indent() << "if ";
    } else {
      out <<
        " else if ";
    }

    out << "(this->__isset." << (*f_iter)->get_name() << ") {" << endl;
    
    indent_up();

    // Write field header
    out <<
      indent() << "xfer += oprot->writeFieldBegin(" <<
      "\"" << (*f_iter)->get_name() << "\", " <<
      type_to_enum((*f_iter)->get_type()) << ", " <<
      (*f_iter)->get_key() << ");" << endl;
    // Write field contents
    if (pointers) {
      generate_serialize_field(out, *f_iter, "(*(this->", "))");
    } else {
      generate_serialize_field(out, *f_iter, "this->");
    }
    // Write field closer
    indent(out) << "xfer += oprot->writeFieldEnd();" << endl;

    indent_down();
    indent(out) << "}";
  }

  // Write the struct map
  out <<
    endl <<
    indent() << "xfer += oprot->writeFieldStop();" << endl <<
    indent() << "xfer += oprot->writeStructEnd();" << endl <<
    indent() << "return xfer;" << endl;

  indent_down();
  indent(out) <<
    "}" << endl <<
    endl;
}

/**
 * Generates a thrift service. In C++, this comprises an entirely separate
 * header and source file. The header file defines the methods and includes
 * the data types defined in the main header file, and the implementation
 * file contains implementations of the basic printer and default interfaces.
 *
 * @param tservice The service definition
 */
void t_cpp_generator::generate_service(t_service* tservice) {
  string svcname = tservice->get_name();
  
  // Make output files
  string f_header_name = string(T_CPP_DIR)+"/"+svcname+".h";
  f_header_.open(f_header_name.c_str());

  // Print header file includes
  f_header_ <<
    autogen_comment();
  f_header_ <<
    "#ifndef " << svcname << "_H" << endl <<
    "#define " << svcname << "_H" << endl <<
    endl <<
    "#include <TProcessor.h>" << endl <<
    "#include \"" << program_name_ << "_types.h\"" << endl;

  if (tservice->get_extends() != NULL) {
    f_header_ <<
      "#include \"" << tservice->get_extends()->get_name() << ".h\"" << endl;
  }

  f_header_ <<
    endl <<
    ns_open_ << endl <<
    endl;

  // Service implementation file includes
  string f_service_name = string(T_CPP_DIR)+"/"+svcname+".cpp";
  f_service_.open(f_service_name.c_str());
  f_service_ <<
    autogen_comment();
  f_service_ <<
    "#include \"" << svcname << ".h\"" << endl << 
    endl <<
    ns_open_ << endl <<
    endl;

  // Generate all the components
  generate_service_interface(tservice);
  generate_service_null(tservice);
  generate_service_helpers(tservice);
  generate_service_client(tservice);
  generate_service_processor(tservice);
  generate_service_multiface(tservice);
  generate_service_skeleton(tservice);

  // Close the namespace
  f_service_ <<
    ns_close_ << endl <<
    endl;
  f_header_ <<
    ns_close_ << endl <<
    endl;
  f_header_ <<
    "#endif" << endl;

  // Close the files
  f_service_.close();
  f_header_.close();
}

/**
 * Generates helper functions for a service. Basically, this generates types
 * for all the arguments and results to functions.
 *
 * @param tservice The service to generate a header definition for
 */
void t_cpp_generator::generate_service_helpers(t_service* tservice) {
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter; 
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    t_struct* ts = (*f_iter)->get_arglist();
    string name_orig = ts->get_name();

    ts->set_name(tservice->get_name() + "_" + (*f_iter)->get_name() + "_args");
    generate_struct_definition(f_header_, ts, false);
    generate_struct_reader(f_service_, ts);
    generate_struct_writer(f_service_, ts);
    ts->set_name(tservice->get_name() + "_" + (*f_iter)->get_name() + "_pargs");
    generate_struct_definition(f_header_, ts, false, true, false, true);
    generate_struct_writer(f_service_, ts, true);
    ts->set_name(name_orig);

    generate_function_helpers(tservice, *f_iter);
  }
}

/**
 * Generates a service interface definition.
 *
 * @param tservice The service to generate a header definition for
 */
void t_cpp_generator::generate_service_interface(t_service* tservice) {
  string extends = "";
  if (tservice->get_extends() != NULL) {
    extends = " : virtual public " + type_name(tservice->get_extends()) + "If";
  }
  f_header_ <<
    "class " << service_name_ << "If" << extends << " {" << endl <<
    " public: " << endl;
  indent_up(); 
  f_header_ <<
    indent() << "virtual ~" << service_name_ << "If() {}" << endl;
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter; 
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    f_header_ <<
      indent() << "virtual " << function_signature(*f_iter) << " = 0;" << endl;
  }
  indent_down();
  f_header_ <<
    "}; " << endl << endl;
}

/**
 * Generates a null implementation of the service.
 *
 * @param tservice The service to generate a header definition for
 */
void t_cpp_generator::generate_service_null(t_service* tservice) {
  string extends = "";
  if (tservice->get_extends() != NULL) {
    extends = " , virtual public " + type_name(tservice->get_extends()) + "Null";
  }
  f_header_ <<
    "class " << service_name_ << "Null : virtual public " << service_name_ << "If" << extends << " {" << endl <<
    " public: " << endl;
  indent_up(); 
  f_header_ <<
    indent() << "virtual ~" << service_name_ << "Null() {}" << endl;
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter; 
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    f_header_ <<
      indent() << function_signature(*f_iter) << " {" << endl;
    indent_up();
    t_type* returntype = (*f_iter)->get_returntype();
    if (returntype->is_void()) {
      f_header_ <<
        indent() << "return;" << endl;
    } else {
      if (is_complex_type(returntype)) {
        f_header_ <<
          indent() << "return;" << endl;
      } else {
        t_field returnfield(returntype, "_return");
        f_header_ <<
          indent() << declare_field(&returnfield, true) << endl <<
          indent() << "return _return;" << endl;
      }
    }
    indent_down();
    f_header_ <<
      indent() << "}" << endl;
  }
  indent_down();
  f_header_ <<
    "}; " << endl << endl;
}


/**
 * Generates a multiface, which is a single server that just takes a set
 * of objects implementing the interface and calls them all, returning the
 * value of the last one to be called.
 *
 * @param tservice The service to generate a multiserver for.
 */
void t_cpp_generator::generate_service_multiface(t_service* tservice) {
  // Generate the dispatch methods
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter; 

  string extends = "";
  string extends_multiface = "";
  if (tservice->get_extends() != NULL) {
    extends = type_name(tservice->get_extends());
    extends_multiface = ", public " + extends + "Multiface";
  }

  string list_type = string("std::vector<boost::shared_ptr<") + service_name_ + "If> >";

  // Generate the header portion
  f_header_ <<
    "class " << service_name_ << "Multiface : " <<
    "virtual public " << service_name_ << "If" <<
    extends_multiface << " {" << endl <<
    " public: " << endl;
  indent_up();
  f_header_ << 
    indent() << service_name_ << "Multiface(" << list_type << "& ifaces) : ifaces_(ifaces) {" << endl;
  if (!extends.empty()) {
    f_header_ <<
      indent() << "  std::vector<boost::shared_ptr<" + service_name_ + "If> >::iterator iter;" << endl <<
      indent() << "  for (iter = ifaces.begin(); iter != ifaces.end(); ++iter) {" << endl <<
      indent() << "    " << extends << "Multiface::add(*iter);" << endl <<
      indent() << "  }" << endl;
  }
  f_header_ <<
    indent() << "}" << endl <<
    indent() << "virtual ~" << service_name_ << "Multiface() {}" << endl;
  indent_down();

  // Protected data members
  f_header_ <<
    " protected:" << endl;
  indent_up();
  f_header_ <<
    indent() << list_type << " ifaces_;" << endl <<
    indent() << service_name_ << "Multiface() {}" << endl <<
    indent() << "void add(boost::shared_ptr<" << service_name_ << "If> iface) { " << endl;
  if (!extends.empty()) {
    f_header_ <<
      indent() << "  " << extends << "Multiface::add(iface);" << endl;
  }
  f_header_ <<
    indent() << "  ifaces_.push_back(iface);" << endl <<
    indent() << "}" << endl;
  indent_down();

  f_header_ <<
    indent() << " public:" << endl;
  indent_up();

  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    t_struct* arglist = (*f_iter)->get_arglist();
    const vector<t_field*>& args = arglist->get_members();
    vector<t_field*>::const_iterator a_iter;

    string call = string("ifaces_[i]->") + (*f_iter)->get_name() + "(";
    bool first = true;
    if (is_complex_type((*f_iter)->get_returntype())) {
      call += "_return";
      first = false;
    }
    for (a_iter = args.begin(); a_iter != args.end(); ++a_iter) {
      if (first) {
        first = false;
      } else {
        call += ", ";
      }
      call += (*a_iter)->get_name();
    }
    call += ")";

    f_header_ <<
      indent() << function_signature(*f_iter) << " {" << endl;
    indent_up();
    f_header_ <<
      indent() << "uint32_t sz = ifaces_.size();" << endl <<
      indent() << "for (uint32_t i = 0; i < sz; ++i) {" << endl;
    if (!(*f_iter)->get_returntype()->is_void()) {
      f_header_ <<
        indent() << "  if (i == sz - 1) {" << endl;
      if (is_complex_type((*f_iter)->get_returntype())) {
        f_header_ <<
          indent() << "    " << call << ";" << endl <<
          indent() << "    return;" << endl;
      } else {
        f_header_ <<
          indent() << "    return " << call << ";" << endl;
      }
      f_header_ <<
        indent() << "  } else {" << endl <<
        indent() << "    " << call << ";" << endl <<
        indent() << "  }" << endl;
    } else {
      f_header_ <<
        indent() << "  " << call << ";" << endl;
    }

    f_header_ <<
      indent() << "}" << endl;
    
    indent_down();
    f_header_ <<
      indent() << "}" << endl <<
      endl;
  }

  indent_down();
  f_header_ <<
    indent() << "};" << endl <<
    endl;
}

/**
 * Generates a service client definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_cpp_generator::generate_service_client(t_service* tservice) {
  string extends = "";
  string extends_client = "";
  if (tservice->get_extends() != NULL) {
    extends = type_name(tservice->get_extends());
    extends_client = ", public " + extends + "Client";
  }

  // Generate the header portion
  f_header_ <<
    "class " << service_name_ << "Client : " <<
    "virtual public " << service_name_ << "If" <<
    extends_client << " {" << endl <<
    " public:" << endl; 

  indent_up();
  f_header_ <<
    indent() << service_name_ << "Client(boost::shared_ptr<facebook::thrift::protocol::TProtocol> prot) : " << endl;
  if (extends.empty()) {
    f_header_ <<
      indent() << "  piprot_(prot)," << endl <<
      indent() << "  poprot_(prot) {" << endl <<
      indent() << "  iprot_ = prot.get();" << endl <<
      indent() << "  oprot_ = prot.get();" << endl <<
      indent() << "}" << endl;
  } else {
    f_header_ <<
      indent() << "  " << extends << "Client(prot, prot) {}" << endl;
  }

  f_header_ <<
    indent() << service_name_ << "Client(boost::shared_ptr<facebook::thrift::protocol::TProtocol> iprot, boost::shared_ptr<facebook::thrift::protocol::TProtocol> oprot) : " << endl;
  if (extends.empty()) {
    f_header_ <<
      indent() << "  piprot_(iprot)," << endl <<
      indent() << "  poprot_(oprot) {" << endl <<
      indent() << "  iprot_ = iprot.get();" << endl <<
      indent() << "  oprot_ = oprot.get();" << endl <<
      indent() << "}" << endl;
  } else {
    f_header_ <<
      indent() << "  " << extends << "Client(iprot, oprot) {}" << endl;
  }

  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::const_iterator f_iter; 
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    t_function send_function(g_type_void,
                             string("send_") + (*f_iter)->get_name(),
                             (*f_iter)->get_arglist());
    indent(f_header_) << function_signature(*f_iter) << ";" << endl;
    indent(f_header_) << function_signature(&send_function) << ";" << endl;
    if (!(*f_iter)->is_async()) {
      t_struct noargs(program_);
      t_function recv_function((*f_iter)->get_returntype(),
                               string("recv_") + (*f_iter)->get_name(),
                               &noargs);
      indent(f_header_) << function_signature(&recv_function) << ";" << endl;
    }
  }
  indent_down();
  
  if (extends.empty()) {
    f_header_ <<
      " protected:" << endl;
    indent_up();
    f_header_ <<
      indent() << "boost::shared_ptr<facebook::thrift::protocol::TProtocol> piprot_;"  << endl <<
      indent() << "boost::shared_ptr<facebook::thrift::protocol::TProtocol> poprot_;"  << endl <<
      indent() << "facebook::thrift::protocol::TProtocol* iprot_;"  << endl <<
      indent() << "facebook::thrift::protocol::TProtocol* oprot_;"  << endl;
    indent_down();  
  }

  f_header_ <<
    "};" << endl <<
    endl;
  
  string scope = service_name_ + "Client::";

  // Generate client method implementations
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    string funname = (*f_iter)->get_name();

    // Open function
    indent(f_service_) <<
      function_signature(*f_iter, scope) << endl;
    scope_up(f_service_);
    indent(f_service_) <<
      "send_" << funname << "(";

    // Get the struct of function call params
    t_struct* arg_struct = (*f_iter)->get_arglist();
    
    // Declare the function arguments
    const vector<t_field*>& fields = arg_struct->get_members();
    vector<t_field*>::const_iterator fld_iter;
    bool first = true;
    for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
      if (first) {
        first = false;
      } else {
        f_service_ << ", ";
      }
      f_service_ << (*fld_iter)->get_name();
    }
    f_service_ << ");" << endl;

    if (!(*f_iter)->is_async()) {
      f_service_ << indent();
      if (!(*f_iter)->get_returntype()->is_void()) {
        if (is_complex_type((*f_iter)->get_returntype())) {
          f_service_ << "recv_" << funname << "(_return);" << endl;
        } else {
          f_service_ << "return recv_" << funname << "();" << endl;
        }
      } else {
        f_service_ <<
          "recv_" << funname << "();" << endl;
      }
    }
    scope_down(f_service_);
    f_service_ << endl;

    // Function for sending
    t_function send_function(g_type_void,
                             string("send_") + (*f_iter)->get_name(),
                             (*f_iter)->get_arglist());

    // Open the send function
    indent(f_service_) <<
      function_signature(&send_function, scope) << endl;
    scope_up(f_service_);

    // Function arguments and results
    string argsname = tservice->get_name() + "_" + (*f_iter)->get_name() + "_pargs";
    string resultname = tservice->get_name() + "_" + (*f_iter)->get_name() + "_presult";

    // Serialize the request
    f_service_ <<
      indent() << "int32_t cseqid = 0;" << endl <<
      indent() << "oprot_->writeMessageBegin(\"" << (*f_iter)->get_name() << "\", facebook::thrift::protocol::T_CALL, cseqid);" << endl <<
      endl <<
      indent() << argsname << " args;" << endl;
    
    for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
      f_service_ <<
        indent() << "args." << (*fld_iter)->get_name() << " = &" << (*fld_iter)->get_name() << ";" << endl;
    }
      
    f_service_ <<
      indent() << "args.write(oprot_);" << endl <<
      endl <<
      indent() << "oprot_->writeMessageEnd();" << endl <<
      indent() << "oprot_->getTransport()->flush();" << endl;
       
    scope_down(f_service_);
    f_service_ << endl;

    // Generate recv function only if not an async function
    if (!(*f_iter)->is_async()) {
      t_struct noargs(program_);
      t_function recv_function((*f_iter)->get_returntype(),
                               string("recv_") + (*f_iter)->get_name(),
                               &noargs);
      // Open function
      indent(f_service_) <<
        function_signature(&recv_function, scope) << endl;
      scope_up(f_service_);

      f_service_ <<
        endl <<
        indent() << "int32_t rseqid = 0;" << endl <<
        indent() << "std::string fname;" << endl <<
        indent() << "facebook::thrift::protocol::TMessageType mtype;" << endl <<
        endl <<
        indent() << "iprot_->readMessageBegin(fname, mtype, rseqid);" << endl <<
        indent() << "if (mtype == facebook::thrift::protocol::T_EXCEPTION) {" << endl <<
        indent() << "  facebook::thrift::TApplicationException x;" << endl <<
        indent() << "  x.read(iprot_);" << endl <<
        indent() << "  iprot_->readMessageEnd();" << endl <<
        indent() << "  iprot_->getTransport()->readEnd();" << endl <<
        indent() << "  throw x;" << endl <<
        indent() << "}" << endl <<
        indent() << "if (mtype != facebook::thrift::protocol::T_REPLY) {" << endl <<
        indent() << "  iprot_->skip(facebook::thrift::protocol::T_STRUCT);" << endl <<
        indent() << "  iprot_->readMessageEnd();" << endl <<
        indent() << "  iprot_->getTransport()->readEnd();" << endl <<
        indent() << "  throw facebook::thrift::TApplicationException(facebook::thrift::TApplicationException::INVALID_MESSAGE_TYPE);" << endl <<
        indent() << "}" << endl <<
        indent() << "if (fname.compare(\"" << (*f_iter)->get_name() << "\") != 0) {" << endl <<
        indent() << "  iprot_->skip(facebook::thrift::protocol::T_STRUCT);" << endl <<
        indent() << "  iprot_->readMessageEnd();" << endl <<
        indent() << "  iprot_->getTransport()->readEnd();" << endl <<
        indent() << "  throw facebook::thrift::TApplicationException(facebook::thrift::TApplicationException::WRONG_METHOD_NAME);" << endl <<
        indent() << "}" << endl;

      if (!(*f_iter)->get_returntype()->is_void() &&
          !is_complex_type((*f_iter)->get_returntype())) {
        t_field returnfield((*f_iter)->get_returntype(), "_return");
        f_service_ <<
          indent() << declare_field(&returnfield) << endl;
      }

      f_service_ <<
        indent() << resultname << " result;" << endl;

      if (!(*f_iter)->get_returntype()->is_void()) {
        f_service_ <<
          indent() << "result.success = &_return;" << endl;
      }

      f_service_ <<
        indent() << "result.read(iprot_);" << endl <<
        indent() << "iprot_->readMessageEnd();" << endl <<
        indent() << "iprot_->getTransport()->readEnd();" << endl <<
        endl;

      // Careful, only look for _result if not a void function
      if (!(*f_iter)->get_returntype()->is_void()) {
        if (is_complex_type((*f_iter)->get_returntype())) {
          f_service_ <<
            indent() << "if (result.__isset.success) {" << endl <<
            indent() << "  // _return pointer has now been filled" << endl <<
            indent() << "  return;" << endl <<
            indent() << "}" << endl;
        } else {
          f_service_ <<
            indent() << "if (result.__isset.success) {" << endl <<
            indent() << "  return _return;" << endl <<
            indent() << "}" << endl;
        }
      }

      t_struct* xs = (*f_iter)->get_xceptions();
      const std::vector<t_field*>& xceptions = xs->get_members();
      vector<t_field*>::const_iterator x_iter;
      for (x_iter = xceptions.begin(); x_iter != xceptions.end(); ++x_iter) {
        f_service_ <<
          indent() << "if (result.__isset." << (*x_iter)->get_name() << ") {" << endl <<
          indent() << "  throw result." << (*x_iter)->get_name() << ";" << endl <<
          indent() << "}" << endl;
      }

      // We only get here if we are a void function
      if ((*f_iter)->get_returntype()->is_void()) {
        indent(f_service_) <<
          "return;" << endl;
      } else {
        f_service_ <<
          indent() << "throw facebook::thrift::TApplicationException(facebook::thrift::TApplicationException::MISSING_RESULT, \"" << (*f_iter)->get_name() << " failed: unknown result\");" << endl;
      }
             
      // Close function
      scope_down(f_service_);
      f_service_ << endl;
    }
  }
}

/**
 * Generates a service server definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_cpp_generator::generate_service_processor(t_service* tservice) {
  // Generate the dispatch methods
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter; 

  string extends = "";
  string extends_processor = "";
  if (tservice->get_extends() != NULL) {
    extends = type_name(tservice->get_extends());
    extends_processor = ", public " + extends + "Processor";
  }

  // Generate the header portion
  f_header_ <<
    "class " << service_name_ << "Processor : " <<
    "virtual public facebook::thrift::TProcessor" << 
    extends_processor << " {" << endl;

  // Protected data members
  f_header_ <<
    " protected:" << endl;
  indent_up();
  f_header_ <<
    indent() << "boost::shared_ptr<" << service_name_ << "If> iface_;" << endl;
  f_header_ <<
    indent() << "virtual bool process_fn(facebook::thrift::protocol::TProtocol* iprot, facebook::thrift::protocol::TProtocol* oprot, std::string& fname, int32_t seqid);" << endl;
  indent_down();

  // Process function declarations
  f_header_ <<
    " private:" << endl;
  indent_up();
  f_header_ <<
    indent() << "std::map<std::string, void (" << service_name_ << "Processor::*)(int32_t, facebook::thrift::protocol::TProtocol*, facebook::thrift::protocol::TProtocol*)> processMap_;" << endl;
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    indent(f_header_) <<
      "void process_" << (*f_iter)->get_name() << "(int32_t seqid, facebook::thrift::protocol::TProtocol* iprot, facebook::thrift::protocol::TProtocol* oprot);" << endl;
  }
  indent_down();

  indent_up();
  string declare_map = "";
  indent_up();

  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    declare_map += indent();
    declare_map += "processMap_[\"";
    declare_map += (*f_iter)->get_name();
    declare_map += "\"] = &";
    declare_map += service_name_;
    declare_map += "Processor::process_";
    declare_map += (*f_iter)->get_name();
    declare_map += ";\n";
  }
  indent_down();

  f_header_ << 
    " public: " << endl <<
    indent() << service_name_ << "Processor(boost::shared_ptr<" << service_name_ << "If> iface) :" << endl;
  if (extends.empty()) {
    f_header_ <<
      indent() << "  iface_(iface) {" << endl;
  } else {
    f_header_ <<
      indent() << "  " << extends << "Processor(iface)," << endl <<
      indent() << "  iface_(iface) {" << endl;
  }
  f_header_ <<
    declare_map <<
    indent() << "}" << endl <<
    endl <<
    indent() << "virtual bool process(boost::shared_ptr<facebook::thrift::protocol::TProtocol> piprot, boost::shared_ptr<facebook::thrift::protocol::TProtocol> poprot);" << endl <<
    indent() << "virtual ~" << service_name_ << "Processor() {}" << endl;
  indent_down();
  f_header_ <<
    "};" << endl << endl;

  // Generate the server implementation
  f_service_ <<
    "bool " << service_name_ << "Processor::process(boost::shared_ptr<facebook::thrift::protocol::TProtocol> piprot, boost::shared_ptr<facebook::thrift::protocol::TProtocol> poprot) {" << endl;
  indent_up();

  f_service_ <<
    endl <<
    indent() << "facebook::thrift::protocol::TProtocol* iprot = piprot.get();" << endl <<
    indent() << "facebook::thrift::protocol::TProtocol* oprot = poprot.get();" << endl <<    
    indent() << "std::string fname;" << endl <<
    indent() << "facebook::thrift::protocol::TMessageType mtype;" << endl <<
    indent() << "int32_t seqid;" << endl <<
    endl <<
    indent() << "iprot->readMessageBegin(fname, mtype, seqid);" << endl <<
    endl <<
    indent() << "if (mtype != facebook::thrift::protocol::T_CALL) {" << endl <<
    indent() << "  iprot->skip(facebook::thrift::protocol::T_STRUCT);" << endl <<
    indent() << "  iprot->readMessageEnd();" << endl <<
    indent() << "  iprot->getTransport()->readEnd();" << endl <<
    indent() << "  facebook::thrift::TApplicationException x(facebook::thrift::TApplicationException::INVALID_MESSAGE_TYPE);" << endl <<
    indent() << "  oprot->writeMessageBegin(fname, facebook::thrift::protocol::T_EXCEPTION, seqid);" << endl <<
    indent() << "  x.write(oprot);" << endl <<
    indent() << "  oprot->writeMessageEnd();" << endl <<
    indent() << "  oprot->getTransport()->flush();" << endl <<
    indent() << "  return true;" << endl <<
    indent() << "}" << endl <<
    endl <<
    indent() << "return process_fn(iprot, oprot, fname, seqid);" <<
    endl;

  indent_down();
  f_service_ <<
    indent() << "}" << endl <<
    endl;

  f_service_ <<
    "bool " << service_name_ << "Processor::process_fn(facebook::thrift::protocol::TProtocol* iprot, facebook::thrift::protocol::TProtocol* oprot, std::string& fname, int32_t seqid) {" << endl;
  indent_up();
    
  // HOT: member function pointer map
  f_service_ <<
    indent() << "std::map<std::string, void (" << service_name_ << "Processor::*)(int32_t, facebook::thrift::protocol::TProtocol*, facebook::thrift::protocol::TProtocol*)>::iterator pfn;" << endl <<
    indent() << "pfn = processMap_.find(fname);" << endl <<
    indent() << "if (pfn == processMap_.end()) {" << endl;
  if (extends.empty()) {
    f_service_ <<
      indent() << "  iprot->skip(facebook::thrift::protocol::T_STRUCT);" << endl <<
      indent() << "  iprot->readMessageEnd();" << endl <<
      indent() << "  iprot->getTransport()->readEnd();" << endl <<
      indent() << "  facebook::thrift::TApplicationException x(facebook::thrift::TApplicationException::UNKNOWN_METHOD, \"Invalid method name: '\"+fname+\"'\");" << endl <<
      indent() << "  oprot->writeMessageBegin(fname, facebook::thrift::protocol::T_EXCEPTION, seqid);" << endl <<
      indent() << "  x.write(oprot);" << endl <<
      indent() << "  oprot->writeMessageEnd();" << endl <<
      indent() << "  oprot->getTransport()->flush();" << endl <<
      indent() << "  return true;" << endl;
  } else {
    f_service_ <<
      indent() << "  return " << extends << "Processor::process_fn(iprot, oprot, fname, seqid);" << endl;
  }
  f_service_ <<
    indent() << "} else {" << endl <<
    indent() << "  (this->*(pfn->second))(seqid, iprot, oprot);" << endl <<
    indent() << "}" << endl;  

  // Read end of args field, the T_STOP, and the struct close
  f_service_ <<
    indent() << "return true;" << endl;

  indent_down();
  f_service_ <<
    "}" << endl <<
    endl;

  // Generate the process subfunctions
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    generate_process_function(tservice, *f_iter);
  }
}

/**
 * Generates a struct and helpers for a function.
 *
 * @param tfunction The function
 */
void t_cpp_generator::generate_function_helpers(t_service* tservice,
                                                t_function* tfunction) {
  if (tfunction->is_async()) {
    return;
  }

  t_struct result(program_, tservice->get_name() + "_" + tfunction->get_name() + "_result");
  t_field success(tfunction->get_returntype(), "success", 0);
  if (!tfunction->get_returntype()->is_void()) {
    result.append(&success);
  }

  t_struct* xs = tfunction->get_xceptions();
  const vector<t_field*>& fields = xs->get_members();
  vector<t_field*>::const_iterator f_iter;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    result.append(*f_iter);
  }

  generate_struct_definition(f_header_, &result, false);
  generate_struct_reader(f_service_, &result);
  generate_struct_result_writer(f_service_, &result);

  result.set_name(tservice->get_name() + "_" + tfunction->get_name() + "_presult");
  generate_struct_definition(f_header_, &result, false, true, true, false);
  generate_struct_reader(f_service_, &result, true);

}

/**
 * Generates a process function definition.
 *
 * @param tfunction The function to write a dispatcher for
 */
void t_cpp_generator::generate_process_function(t_service* tservice,
                                                t_function* tfunction) {
  // Open function
  f_service_ <<
    "void " << tservice->get_name() << "Processor::" <<
    "process_" << tfunction->get_name() <<
    "(int32_t seqid, facebook::thrift::protocol::TProtocol* iprot, facebook::thrift::protocol::TProtocol* oprot)" << endl;
  scope_up(f_service_);

  string argsname = tservice->get_name() + "_" + tfunction->get_name() + "_args";
  string resultname = tservice->get_name() + "_" + tfunction->get_name() + "_result";

  f_service_ <<
    indent() << argsname << " args;" << endl <<
    indent() << "args.read(iprot);" << endl <<
    indent() << "iprot->readMessageEnd();" << endl <<
    indent() << "iprot->getTransport()->readEnd();" << endl <<
    endl;

  t_struct* xs = tfunction->get_xceptions();
  const std::vector<t_field*>& xceptions = xs->get_members();
  vector<t_field*>::const_iterator x_iter;

  // Declare result
  if (!tfunction->is_async()) {
    f_service_ <<
      indent() << resultname << " result;" << endl;
  }

  // Try block for functions with exceptions
  if (xceptions.size() > 0) {
    f_service_ <<
      indent() << "try {" << endl;
    indent_up();
  }

  // Generate the function call
  t_struct* arg_struct = tfunction->get_arglist();
  const std::vector<t_field*>& fields = arg_struct->get_members();
  vector<t_field*>::const_iterator f_iter;

  bool first = true;
  f_service_ << indent();
  if (!tfunction->is_async() && !tfunction->get_returntype()->is_void()) {
    if (is_complex_type(tfunction->get_returntype())) {
      first = false;
      f_service_ << "iface_->" << tfunction->get_name() << "(result.success";
    } else {
      f_service_ << "result.success = iface_->" << tfunction->get_name() << "(";
    }
  } else {
    f_service_ <<
      "iface_->" << tfunction->get_name() << "(";
  }
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
    } else {
      f_service_ << ", ";
    }
    f_service_ << "args." << (*f_iter)->get_name();
  }
  f_service_ << ");" << endl;

  // Set isset on success field
  if (!tfunction->is_async() && !tfunction->get_returntype()->is_void()) {
    f_service_ <<
      indent() << "result.__isset.success = true;" << endl;
  }

  if (!tfunction->is_async() && xceptions.size() > 0) {
    indent_down();
    f_service_ << indent() << "}";
    for (x_iter = xceptions.begin(); x_iter != xceptions.end(); ++x_iter) {
      f_service_ << " catch (" << (*x_iter)->get_type()->get_name() << " &" << (*x_iter)->get_name() << ") {" << endl;
      if (!tfunction->is_async()) {
        indent_up();
        f_service_ <<
          indent() << "result." << (*x_iter)->get_name() << " = " << (*x_iter)->get_name() << ";" << endl <<
          indent() << "result.__isset." << (*x_iter)->get_name() << " = true;" << endl;
        indent_down();
        f_service_ << indent() << "}";
      } else {
        f_service_ << "}";
      }
    }
    f_service_ << endl;
  }

  // Shortcut out here for async functions
  if (tfunction->is_async()) {
    f_service_ <<
      indent() << "return;" << endl;
    indent_down();
    f_service_ << "}" << endl <<
      endl;
    return;
  }

  // Serialize the result into a struct
  f_service_ <<
    endl <<
    indent() << "oprot->writeMessageBegin(\"" << tfunction->get_name() << "\", facebook::thrift::protocol::T_REPLY, seqid);" << endl <<
    indent() << "result.write(oprot);" << endl <<
    indent() << "oprot->writeMessageEnd();" << endl <<
    indent() << "oprot->getTransport()->flush();" << endl <<
    indent() << "oprot->getTransport()->writeEnd();" << endl;
    
  // Close function
  scope_down(f_service_);
  f_service_ << endl;
}

/**
 * Generates a skeleton file of a server
 *
 * @param tservice The service to generate a server for.
 */
void t_cpp_generator::generate_service_skeleton(t_service* tservice) {
  string svcname = tservice->get_name();
  
  // Service implementation file includes
  string f_skeleton_name = string(T_CPP_DIR)+"/"+svcname+"_server.skeleton.cpp";

  string ns = namespace_prefix(tservice->get_program()->get_cpp_namespace());

  ofstream f_skeleton;
  f_skeleton.open(f_skeleton_name.c_str());
  f_skeleton <<
    "// This autogenerated skeleton file illustrates how to build a server." << endl <<
    "// You should copy it to another filename to avoid overwriting it." << endl <<
    endl <<
    "#include \"" << svcname << ".h\"" << endl << 
    "#include <protocol/TBinaryProtocol.h>" << endl <<
    "#include <server/TSimpleServer.h>" << endl <<
    "#include <transport/TServerSocket.h>" << endl <<
    "#include <transport/TTransportUtils.h>" << endl <<
    endl <<
    "using namespace facebook::thrift;" << endl <<
    "using namespace facebook::thrift::protocol;" << endl <<
    "using namespace facebook::thrift::transport;" << endl <<
    "using namespace facebook::thrift::server;" << endl <<
    endl;
  
  if (!ns.empty()) {
    f_skeleton <<
      "using namespace " << string(ns, 0, ns.size()-2) << ";" << endl <<
      endl;
  }

  f_skeleton <<
    "class " << svcname << "Handler : virtual public " << svcname << "If {" << endl <<
    " public:" << endl;
  indent_up();
  f_skeleton <<
    indent() << svcname << "Handler() {" << endl <<
    indent() << "  // Your initialization goes here" << endl <<
    indent() << "}" << endl <<
    endl;

  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter;
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    f_skeleton <<
      indent() << function_signature(*f_iter) << " {" << endl <<
      indent() << "  // Your implementation goes here" << endl <<
      indent() << "  printf(\"" << (*f_iter)->get_name() << "\\n\");" << endl <<
      indent() << "}" << endl <<
      endl;
  }

  indent_down();
  f_skeleton <<
    "};" << endl <<
    endl;

  f_skeleton <<
    indent() << "int main(int argc, char **argv) {" << endl;
  indent_up();
  f_skeleton <<
    indent() << "int port = 9090;" << endl <<
    indent() << "shared_ptr<" << svcname << "Handler> handler(new " << svcname << "Handler());" << endl <<
    indent() << "shared_ptr<TProcessor> processor(new " << svcname << "Processor(handler));" << endl <<
    indent() << "shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));" << endl <<
    indent() << "shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());" << endl <<
    indent() << "shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());" << endl <<
    endl <<
    indent() << "TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);" << endl <<
    indent() << "server.serve();" << endl <<
    indent() << "return 0;" << endl;
  indent_down();
  f_skeleton <<
    "}" << endl <<
    endl;
  
  // Close the files
  f_skeleton.close();
}

/**
 * Deserializes a field of any type.
 */
void t_cpp_generator::generate_deserialize_field(ofstream& out,
                                                 t_field* tfield,
                                                 string prefix,
                                                 string suffix) {
  t_type* type = tfield->get_type();
  while (type->is_typedef()) {
    type = ((t_typedef*)type)->get_type();
  }

  if (type->is_void()) {
    throw "CANNOT GENERATE DESERIALIZE CODE FOR void TYPE: " +
      prefix + tfield->get_name();
  }

  string name = prefix + tfield->get_name() + suffix;

  if (type->is_struct() || type->is_xception()) {
    generate_deserialize_struct(out, (t_struct*)type, name);
  } else if (type->is_container()) {
    generate_deserialize_container(out, type, name);
  } else if (type->is_base_type() || type->is_enum()) {
    indent(out) <<
      "xfer += iprot->";
    
    if (type->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
      switch (tbase) {
      case t_base_type::TYPE_VOID:
        throw "compiler error: cannot serialize void field in a struct: " +
          name;
        break;
      case t_base_type::TYPE_STRING:
        out << "readString(" << name << ");";
        break;
      case t_base_type::TYPE_BOOL:
        out << "readBool(" << name << ");";
        break;
      case t_base_type::TYPE_BYTE:
        out << "readByte(" << name << ");";
        break;
      case t_base_type::TYPE_I16:
        out << "readI16(" << name << ");";
        break;
      case t_base_type::TYPE_I32:
        out << "readI32(" << name << ");";
        break;
      case t_base_type::TYPE_I64:
        out << "readI64(" << name << ");";
        break;
      case t_base_type::TYPE_DOUBLE:
        out << "readDouble(" << name << ");";
        break;
      default:
        throw "compiler error: no C++ reader for base type " + tbase + name;
      }
    } else if (type->is_enum()) {
      out << "readI32((int32_t&)" << name << ");";
    }
    out <<
      endl;
  } else {
    printf("DO NOT KNOW HOW TO DESERIALIZE FIELD '%s' TYPE '%s'\n",
           tfield->get_name().c_str(), type_name(type).c_str());
  }
}

/**
 * Generates an unserializer for a variable. This makes two key assumptions,
 * first that there is a const char* variable named data that points to the
 * buffer for deserialization, and that there is a variable protocol which
 * is a reference to a TProtocol serialization object.
 */
void t_cpp_generator::generate_deserialize_struct(ofstream& out,
                                                  t_struct* tstruct,
                                                  string prefix) {
  indent(out) <<
    "xfer += " << prefix << ".read(iprot);" << endl;
}

void t_cpp_generator::generate_deserialize_container(ofstream& out,
                                                     t_type* ttype,
                                                     string prefix) {
  scope_up(out);
  
  string size = tmp("_size");
  string ktype = tmp("_ktype");
  string vtype = tmp("_vtype");
  string etype = tmp("_etype");
  
  indent(out) <<
    prefix << ".clear();" << endl <<
    "uint32_t " << size << ";" << endl;
  
  // Declare variables, read header
  if (ttype->is_map()) {
    out <<
      indent() << "facebook::thrift::protocol::TType " << ktype << ";" << endl <<
      indent() << "facebook::thrift::protocol::TType " << vtype << ";" << endl <<
      indent() << "iprot->readMapBegin(" <<
                   ktype << ", " << vtype << ", " << size << ");" << endl;
  } else if (ttype->is_set()) {
    out <<
      indent() << "facebook::thrift::protocol::TType " << etype << ";" << endl <<
      indent() << "iprot->readSetBegin(" <<
                   etype << ", " << size << ");" << endl;
  } else if (ttype->is_list()) {
    out <<
      indent() << "facebook::thrift::protocol::TType " << etype << ";" << endl <<
      indent() << "iprot->readListBegin(" <<
                   etype << ", " << size << ");" << endl;
  }


  // For loop iterates over elements
  string i = tmp("_i");
  out <<
    indent() << "uint32_t " << i << ";" << endl <<
    indent() << "for (" << i << " = 0; " << i << " < " << size << "; ++" << i << ")" << endl;
  
    scope_up(out);
    
    if (ttype->is_map()) {
      generate_deserialize_map_element(out, (t_map*)ttype, prefix);
    } else if (ttype->is_set()) {
      generate_deserialize_set_element(out, (t_set*)ttype, prefix);
    } else if (ttype->is_list()) {
      generate_deserialize_list_element(out, (t_list*)ttype, prefix);
    }
    
    scope_down(out);

  // Read container end
  if (ttype->is_map()) {
    indent(out) << "iprot->readMapEnd();" << endl;
  } else if (ttype->is_set()) {
    indent(out) << "iprot->readSetEnd();" << endl;
  } else if (ttype->is_list()) {
    indent(out) << "iprot->readListEnd();" << endl;
  }

  scope_down(out);
}


/**
 * Generates code to deserialize a map
 */
void t_cpp_generator::generate_deserialize_map_element(ofstream& out,
                                                       t_map* tmap,
                                                       string prefix) {
  string key = tmp("_key");
  string val = tmp("_val");
  t_field fkey(tmap->get_key_type(), key);
  t_field fval(tmap->get_val_type(), val);

  out <<
    indent() << declare_field(&fkey) << endl <<
    indent() << declare_field(&fval) << endl;

  generate_deserialize_field(out, &fkey);
  generate_deserialize_field(out, &fval);

  indent(out) <<
    prefix << ".insert(std::make_pair(" << key << ", " << val << "));" << endl;
}

void t_cpp_generator::generate_deserialize_set_element(ofstream& out,
                                                       t_set* tset,
                                                       string prefix) {
  string elem = tmp("_elem");
  t_field felem(tset->get_elem_type(), elem);

  indent(out) <<
    declare_field(&felem) << endl;

  generate_deserialize_field(out, &felem);

  indent(out) <<
    prefix << ".insert(" << elem << ");" << endl;
}

void t_cpp_generator::generate_deserialize_list_element(ofstream& out,
                                                        t_list* tlist,
                                                        string prefix) {
  string elem = tmp("_elem");
  t_field felem(tlist->get_elem_type(), elem);

  indent(out) <<
    declare_field(&felem) << endl;

  generate_deserialize_field(out, &felem);

  indent(out) <<
    prefix << ".push_back(" << elem << ");" << endl;
}


/**
 * Serializes a field of any type.
 *
 * @param tfield The field to serialize
 * @param prefix Name to prepend to field name
 */
void t_cpp_generator::generate_serialize_field(ofstream& out,
                                               t_field* tfield,
                                               string prefix,
                                               string suffix) {
  t_type* type = tfield->get_type();
  while (type->is_typedef()) {
    type = ((t_typedef*)type)->get_type();
  }

  string name = prefix + tfield->get_name() + suffix;

  // Do nothing for void types
  if (type->is_void()) {
    throw "CANNOT GENERATE SERIALIZE CODE FOR void TYPE: " + name;
  }

  
  
  if (type->is_struct() || type->is_xception()) {
    generate_serialize_struct(out,
                              (t_struct*)type,
                              name);
  } else if (type->is_container()) {
    generate_serialize_container(out, type, name);
  } else if (type->is_base_type() || type->is_enum()) {

    indent(out) <<
      "xfer += oprot->";
    
    if (type->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
      switch (tbase) {
      case t_base_type::TYPE_VOID:
        throw
          "compiler error: cannot serialize void field in a struct: " + name;
        break;
      case t_base_type::TYPE_STRING:
        out << "writeString(" << name << ");";
        break;
      case t_base_type::TYPE_BOOL:
        out << "writeBool(" << name << ");";
        break;
      case t_base_type::TYPE_BYTE:
        out << "writeByte(" << name << ");";
        break;
      case t_base_type::TYPE_I16:
        out << "writeI16(" << name << ");";
        break;
      case t_base_type::TYPE_I32:
        out << "writeI32(" << name << ");";
        break;
      case t_base_type::TYPE_I64:
        out << "writeI64(" << name << ");";
        break;
      case t_base_type::TYPE_DOUBLE:
        out << "writeDouble(" << name << ");";
        break;
      default:
        throw "compiler error: no C++ writer for base type " + tbase + name;
      }
    } else if (type->is_enum()) {
      out << "writeI32((int32_t)" << name << ");";
    }
    out << endl;
  } else {
    printf("DO NOT KNOW HOW TO SERIALIZE FIELD '%s' TYPE '%s'\n",
           name.c_str(),
           type_name(type).c_str());
  }
}

/**
 * Serializes all the members of a struct.
 *
 * @param tstruct The struct to serialize
 * @param prefix  String prefix to attach to all fields
 */
void t_cpp_generator::generate_serialize_struct(ofstream& out,
                                                t_struct* tstruct,
                                                string prefix) {
  indent(out) <<
    "xfer += " << prefix << ".write(oprot);" << endl;
}

void t_cpp_generator::generate_serialize_container(ofstream& out,
                                                   t_type* ttype,
                                                   string prefix) {
  scope_up(out);
  
  if (ttype->is_map()) {
    indent(out) <<
      "xfer += oprot->writeMapBegin(" <<
      type_to_enum(((t_map*)ttype)->get_key_type()) << ", " <<
      type_to_enum(((t_map*)ttype)->get_val_type()) << ", " <<
      prefix << ".size());" << endl;
  } else if (ttype->is_set()) {
    indent(out) <<
      "xfer += oprot->writeSetBegin(" <<
      type_to_enum(((t_set*)ttype)->get_elem_type()) << ", " <<
      prefix << ".size());" << endl;
  } else if (ttype->is_list()) {
    indent(out) <<
      "xfer += oprot->writeListBegin(" <<
      type_to_enum(((t_list*)ttype)->get_elem_type()) << ", " <<
      prefix << ".size());" << endl;
  }

  string iter = tmp("_iter");
  out <<
    indent() << type_name(ttype) << "::const_iterator " << iter << ";" << endl <<
    indent() << "for (" << iter << " = " << prefix  << ".begin(); " << iter << " != " << prefix << ".end(); ++" << iter << ")" << endl;
  scope_up(out);
    if (ttype->is_map()) {
      generate_serialize_map_element(out, (t_map*)ttype, iter);
    } else if (ttype->is_set()) {
      generate_serialize_set_element(out, (t_set*)ttype, iter);
    } else if (ttype->is_list()) {
      generate_serialize_list_element(out, (t_list*)ttype, iter);
    }
  scope_down(out);
    
  if (ttype->is_map()) {
    indent(out) <<
      "xfer += oprot->writeMapEnd();" << endl;
  } else if (ttype->is_set()) {
    indent(out) <<
      "xfer += oprot->writeSetEnd();" << endl;
  } else if (ttype->is_list()) {
    indent(out) <<
      "xfer += oprot->writeListEnd();" << endl;
  }

  scope_down(out);  
}

/**
 * Serializes the members of a map.
 *
 */
void t_cpp_generator::generate_serialize_map_element(ofstream& out,
                                                     t_map* tmap,
                                                     string iter) {
  t_field kfield(tmap->get_key_type(), iter + "->first");
  generate_serialize_field(out, &kfield, "");

  t_field vfield(tmap->get_val_type(), iter + "->second");
  generate_serialize_field(out, &vfield, "");
}

/**
 * Serializes the members of a set.
 */
void t_cpp_generator::generate_serialize_set_element(ofstream& out,
                                                     t_set* tset,
                                                     string iter) {
  t_field efield(tset->get_elem_type(), "(*" + iter + ")");
  generate_serialize_field(out, &efield, "");
}

/**
 * Serializes the members of a list.
 */
void t_cpp_generator::generate_serialize_list_element(ofstream& out,
                                                      t_list* tlist,
                                                      string iter) {
  t_field efield(tlist->get_elem_type(), "(*" + iter + ")");
  generate_serialize_field(out, &efield, "");
}

/**
 * Makes a :: prefix for a namespace
 *
 * @param ns The namepsace, w/ periods in it
 * @return Namespaces
 */
string t_cpp_generator::namespace_prefix(string ns) {
  if (ns.size() == 0) {
    return "";
  }
  string result = "";
  string::size_type loc;
  while ((loc = ns.find(".")) != string::npos) {
    result += ns.substr(0, loc);
    result += "::";
    ns = ns.substr(loc+1);
  }
  if (ns.size() > 0) {
    result += ns + "::";
  }
  return result;  
}

/**
 * Opens namespace.
 *
 * @param ns The namepsace, w/ periods in it
 * @return Namespaces
 */
string t_cpp_generator::namespace_open(string ns) {
  if (ns.size() == 0) {
    return "";
  }
  string result = "";
  string::size_type loc;
  while ((loc = ns.find(".")) != string::npos) {
    result += "namespace ";
    result += ns.substr(0, loc);
    result += " { ";
    ns = ns.substr(loc+1);
  }
  if (ns.size() > 0) {
    result += "namespace " + ns + " { ";
  }
  return result;
}

/**
 * Closes namespace.
 *
 * @param ns The namepsace, w/ periods in it
 * @return Namespaces
 */
string t_cpp_generator::namespace_close(string ns) {
  if (ns.size() == 0) {
    return "";
  }
  string result = "}";
  string::size_type loc;
  while ((loc = ns.find(".")) != string::npos) {
    result += "}";
    ns = ns.substr(loc+1);
  }
  result += " // namespace";
  return result;
}

/**
 * Returns a C++ type name
 *
 * @param ttype The type
 * @return String of the type name, i.e. std::set<type>
 */
string t_cpp_generator::type_name(t_type* ttype, bool in_typedef, bool arg) {
  if (ttype->is_base_type()) {
    string bname = base_type_name(((t_base_type*)ttype)->get_base());
    if (!arg) {
      return bname;
    }

    if (((t_base_type*)ttype)->get_base() == t_base_type::TYPE_STRING) {
      return "const " + bname + "&";
    } else {
      return "const " + bname;
    }
  }
 
  // Check for a custom overloaded C++ name
  if (ttype->is_container()) {
    string cname;

    t_container* tcontainer = (t_container*) ttype;
    if (tcontainer->has_cpp_name()) {
      cname = tcontainer->get_cpp_name();
    } else if (ttype->is_map()) {
      t_map* tmap = (t_map*) ttype;
      cname = "std::map<" +
        type_name(tmap->get_key_type(), in_typedef) + ", " +
        type_name(tmap->get_val_type(), in_typedef) + "> ";
    } else if (ttype->is_set()) {
      t_set* tset = (t_set*) ttype;
      cname = "std::set<" + type_name(tset->get_elem_type(), in_typedef) + "> ";
    } else if (ttype->is_list()) {
      t_list* tlist = (t_list*) ttype;
      cname = "std::vector<" + type_name(tlist->get_elem_type(), in_typedef) + "> ";
    }
    
    if (arg) {
      return "const " + cname + "&";
    } else {
      return cname;
    }
  }

  string class_prefix;
  if (in_typedef && (ttype->is_struct() || ttype->is_xception())) {
    class_prefix = "class ";
  }

  // Check if it needs to be namespaced
  string pname;
  t_program* program = ttype->get_program();
  if (program != NULL && program != program_) {
    pname = 
      class_prefix +
      namespace_prefix(program->get_cpp_namespace()) +
      ttype->get_name();
  } else {
    pname = class_prefix + ttype->get_name();
  }

  if (arg) {
    if (is_complex_type(ttype)) {
      return "const " + pname + "&";
    } else {
      return "const " + pname;
    }
  } else {
    return pname;
  }
}

/**
 * Returns the C++ type that corresponds to the thrift type.
 *
 * @param tbase The base type
 * @return Explicit C++ type, i.e. "int32_t"
 */
string t_cpp_generator::base_type_name(t_base_type::t_base tbase) {
  switch (tbase) {
  case t_base_type::TYPE_VOID:
    return "void";
  case t_base_type::TYPE_STRING:
    return "std::string";
  case t_base_type::TYPE_BOOL:
    return "bool";
  case t_base_type::TYPE_BYTE:
    return "int8_t";
  case t_base_type::TYPE_I16:
    return "int16_t";
  case t_base_type::TYPE_I32:
    return "int32_t";
  case t_base_type::TYPE_I64:
    return "int64_t";
  case t_base_type::TYPE_DOUBLE:
    return "double";
  default:
    throw "compiler error: no C++ base type name for base type " + tbase;
  }
}

/**
 * Declares a field, which may include initialization as necessary.
 *
 * @param ttype The type
 * @return Field declaration, i.e. int x = 0;
 */
string t_cpp_generator::declare_field(t_field* tfield, bool init, bool pointer, bool constant) {
  // TODO(mcslee): do we ever need to initialize the field?
  string result = "";
  if (constant) {
    result += "const ";
  }
  result += type_name(tfield->get_type());
  if (pointer) {
    result += "*";
  }
  result += " " + tfield->get_name();
  if (init) {
    t_type* type = tfield->get_type();
    while (type->is_typedef()) {
      type = ((t_typedef*)type)->get_type();
    }

    if (type->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
      switch (tbase) {
      case t_base_type::TYPE_VOID:
        break;
      case t_base_type::TYPE_STRING:
        result += " = \"\"";
        break;
      case t_base_type::TYPE_BOOL:
        result += " = false";
        break;
      case t_base_type::TYPE_BYTE:
      case t_base_type::TYPE_I16:
      case t_base_type::TYPE_I32:
      case t_base_type::TYPE_I64:
        result += " = 0";
        break;
      case t_base_type::TYPE_DOUBLE:
        result += " = (double)0";
        break;
      default:
        throw "compiler error: no C++ initializer for base type " + tbase;
      }
    } else if (type->is_enum()) {
      result += " = (" + type_name(type) + ")0";
    }
  }
  return result + ";";
}

/**
 * Renders a function signature of the form 'type name(args)'
 *
 * @param tfunction Function definition
 * @return String of rendered function definition
 */
string t_cpp_generator::function_signature(t_function* tfunction,
                                           string prefix) {
  t_type* ttype = tfunction->get_returntype();
  t_struct* arglist = tfunction->get_arglist();

  if (is_complex_type(ttype)) {
    bool empty = arglist->get_members().size() == 0;
    return
      "void " + prefix + tfunction->get_name() +
      "(" + type_name(ttype) + "& _return" +
      (empty ? "" : (", " + argument_list(arglist))) + ")";
  } else {
    return
      type_name(ttype) + " " + prefix + tfunction->get_name() +
      "(" + argument_list(arglist) + ")";
  }
}

/**
 * Renders a field list
 *
 * @param tstruct The struct definition
 * @return Comma sepearated list of all field names in that struct
 */
string t_cpp_generator::argument_list(t_struct* tstruct) {
  string result = "";

  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;
  bool first = true;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
    } else {
      result += ", ";
    }
    result += type_name((*f_iter)->get_type(), false, true) + " " + (*f_iter)->get_name();
  }
  return result;
}

/**
 * Converts the parse type to a C++ enum string for the given type.
 *
 * @param type Thrift Type
 * @return String of C++ code to definition of that type constant
 */
string t_cpp_generator::type_to_enum(t_type* type) {
  while (type->is_typedef()) {
    type = ((t_typedef*)type)->get_type();
  }
  
  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
    case t_base_type::TYPE_VOID:
      throw "NO T_VOID CONSTRUCT";
    case t_base_type::TYPE_STRING:
      return "facebook::thrift::protocol::T_STRING";
    case t_base_type::TYPE_BOOL:
      return "facebook::thrift::protocol::T_BOOL";
    case t_base_type::TYPE_BYTE:
      return "facebook::thrift::protocol::T_BYTE";
    case t_base_type::TYPE_I16:
      return "facebook::thrift::protocol::T_I16";
    case t_base_type::TYPE_I32:
      return "facebook::thrift::protocol::T_I32";
    case t_base_type::TYPE_I64:
      return "facebook::thrift::protocol::T_I64";
    case t_base_type::TYPE_DOUBLE:
      return "facebook::thrift::protocol::T_DOUBLE";
    }
  } else if (type->is_enum()) {
    return "facebook::thrift::protocol::T_I32";
  } else if (type->is_struct()) {
    return "facebook::thrift::protocol::T_STRUCT";
  } else if (type->is_xception()) {
    return "facebook::thrift::protocol::T_STRUCT";
  } else if (type->is_map()) {
    return "facebook::thrift::protocol::T_MAP";
  } else if (type->is_set()) {
    return "facebook::thrift::protocol::T_SET";
  } else if (type->is_list()) {
    return "facebook::thrift::protocol::T_LIST";
  }

  throw "INVALID TYPE IN type_to_enum: " + type->get_name();
}