// Copyright (c) 2006- Facebook
// Distributed under the Thrift Software License
//
// See accompanying file LICENSE or visit the Thrift site at:
// http://developers.facebook.com/thrift/

#ifndef T_PHP_GENERATOR_H
#define T_PHP_GENERATOR_H

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include "t_oop_generator.h"

/**
 * PHP code generator.
 *
 * @author Mark Slee <mcslee@facebook.com>
 */
class t_php_generator : public t_oop_generator {
 public:
  t_php_generator(t_program* program,
                  bool binary_inline=false,
                  bool rest=false,
                  bool phps=false,
                  bool autoload=false,
                  bool oop=false) :
    t_oop_generator(program),
    binary_inline_(binary_inline),
    rest_(rest),
    phps_(phps),
    autoload_(autoload),
    oop_(oop) {
    out_dir_base_ = (binary_inline_ ? "gen-phpi" : "gen-php");
  }

  /**
   * Init and close methods
   */

  void init_generator();
  void close_generator();

  /**
   * Program-level generation functions
   */

  void generate_typedef  (t_typedef*  ttypedef);
  void generate_enum     (t_enum*     tenum);
  void generate_const    (t_const*    tconst);
  void generate_struct   (t_struct*   tstruct);
  void generate_xception (t_struct*   txception);
  void generate_service  (t_service*  tservice);

  std::string render_const_value(t_type* type, t_const_value* value);

  /**
   * Structs!
   */

  void generate_php_struct(t_struct* tstruct, bool is_exception);
  void generate_php_struct_definition(std::ofstream& out, t_struct* tstruct, bool is_xception=false);
  void _generate_php_struct_definition(std::ofstream& out, t_struct* tstruct, bool is_xception=false);
  void generate_php_struct_reader(std::ofstream& out, t_struct* tstruct);
  void generate_php_struct_writer(std::ofstream& out, t_struct* tstruct);
  void generate_php_function_helpers(t_function* tfunction);

  void generate_php_type_spec(std::ofstream &out, t_type* t);
  void generate_php_struct_spec(std::ofstream &out, t_struct* tstruct);

  /**
   * Service-level generation functions
   */

  void generate_service_helpers   (t_service* tservice);
  void generate_service_interface (t_service* tservice);
  void generate_service_rest      (t_service* tservice);
  void generate_service_client    (t_service* tservice);
  void _generate_service_client   (std::ofstream &out, t_service* tservice);
  void generate_service_processor (t_service* tservice);
  void generate_process_function  (t_service* tservice, t_function* tfunction);

  /**
   * Serialization constructs
   */

  void generate_deserialize_field        (std::ofstream &out,
                                          t_field*    tfield,
                                          std::string prefix="",
                                          bool inclass=false);

  void generate_deserialize_struct       (std::ofstream &out,
                                          t_struct*   tstruct,
                                          std::string prefix="");

  void generate_deserialize_container    (std::ofstream &out,
                                          t_type*     ttype,
                                          std::string prefix="");

  void generate_deserialize_set_element  (std::ofstream &out,
                                          t_set*      tset,
                                          std::string prefix="");

  void generate_deserialize_map_element  (std::ofstream &out,
                                          t_map*      tmap,
                                          std::string prefix="");

  void generate_deserialize_list_element (std::ofstream &out,
                                          t_list*     tlist,
                                          std::string prefix="");

  void generate_serialize_field          (std::ofstream &out,
                                          t_field*    tfield,
                                          std::string prefix="");

  void generate_serialize_struct         (std::ofstream &out,
                                          t_struct*   tstruct,
                                          std::string prefix="");

  void generate_serialize_container      (std::ofstream &out,
                                          t_type*     ttype,
                                          std::string prefix="");

  void generate_serialize_map_element    (std::ofstream &out,
                                          t_map*      tmap,
                                          std::string kiter,
                                          std::string viter);

  void generate_serialize_set_element    (std::ofstream &out,
                                          t_set*      tmap,
                                          std::string iter);

  void generate_serialize_list_element   (std::ofstream &out,
                                          t_list*     tlist,
                                          std::string iter);

  /**
   * Helper rendering functions
   */

  std::string php_includes();
  std::string declare_field(t_field* tfield, bool init=false, bool obj=false);
  std::string function_signature(t_function* tfunction, std::string prefix="");
  std::string argument_list(t_struct* tstruct);
  std::string type_to_cast(t_type* ttype);
  std::string type_to_enum(t_type* ttype);

  std::string php_namespace(t_program* p) {
    std::string ns = p->get_php_namespace();
    return ns.size() ? (ns + "_") : "";
  }

 private:

  /**
   * File streams
   */
  std::ofstream f_types_;
  std::ofstream f_consts_;
  std::ofstream f_helpers_;
  std::ofstream f_service_;

  /**
   * Generate protocol-independent template? Or Binary inline code?
   */
  bool binary_inline_;

  /**
   * Generate a REST handler class
   */
  bool rest_;

  /**
   * Generate stubs for a PHP server
   */
  bool phps_;

  /**
   * Generate PHP code that uses autoload
   */
  bool autoload_;

  /**
   * Whether to use OOP base class TBase
   */
  bool oop_;

};

#endif
