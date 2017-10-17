/*******************************************************************************
 *
 * Main driver of ARBOS
 *
 * Authors: Nija Shi
 *          Arnaud J. Venet
 *
 * Contributors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2017 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Disclaimers:
 *
 * No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
 * ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS,
 * ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE
 * ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO
 * THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
 * ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
 * RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
 * RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
 * DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE,
 * IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."
 *
 * Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST
 * THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL
 * AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS
 * IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH
 * USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM,
 * RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD
 * HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS,
 * AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.
 * RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE,
 * UNILATERAL TERMINATION OF THIS AGREEMENT.
 *
 ******************************************************************************/

#include <dlfcn.h>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef LINUX
#include <cstdlib>
#endif

#include <arbos/semantics/ar.hpp>
#include <arbos/semantics/builders.hpp>

using namespace arbos;

namespace arbos {

const std::string AR_EXT = ".ar";

/**
 * ImportAR implements the ARBuilder that imports an AR definitions represented
 * in s-expression.
 */
class ImportAR : public ARBuilder {
public:
  virtual ARModel* build() {
    try {
      ARModel::Instance()->generate(std::cin);
    } catch (parse_error& e) {
      std::cerr << e << std::endl;
    } catch (error& e) {
      std::cerr << e << std::endl;
    } catch (...) {
      std::cerr << "Unknown error occurred during AR model construction."
                << std::endl;
    }
    return ARModel::Instance();
  }

  virtual std::string ext() { return AR_EXT; }
  virtual ~ImportAR() {}
}; // class ImportAR

class DynamicLibrary {
private:
  std::string _path;
  int _flag;
  void* _handle;

public:
  DynamicLibrary() : _path(), _flag(0), _handle(nullptr) {}

  DynamicLibrary(const DynamicLibrary&) = delete;

  ~DynamicLibrary() { this->close(); }

  bool open(const std::string& path, int flag) {
    this->close();

    _path = path;
    _flag = flag;
    _handle = dlopen(_path.c_str(), _flag);
    return _handle != nullptr;
  }

  void close() {
    if (_handle == nullptr)
      return;

    dlclose(_handle);
    _handle = nullptr;
  }

  inline const std::string& getPath() const { return _path; }

  inline int getFlag() const { return _flag; }

  inline bool isOpened() const { return _handle != nullptr; }

  char* getError() const { return dlerror(); }

  template < typename T >
  T lookup(const std::string& symbol) const {
    assert(_handle != nullptr);
    return reinterpret_cast< T >(dlsym(_handle, symbol.c_str()));
  }

}; // class DynamicLibrary

} // namespace arbos

int main(int argc, char** argv) {
  /*
   * Command usage:
   */
  if (argc < 2) {
    std::cerr << "Usage: arbos -load=<path-to-dylib> "
              << "-<pass-short-name> [options] < <path-to-ar-file>.ar"
              << std::endl;
    return 1;
  }

  /**
   * Command options: first, look for -load= and -<pass-short-name>
   */
  const std::string LOAD_OPT = "-load=";
  const std::string PASS_PREFIX = "-";

  std::unique_ptr< ARModel > ar_model = nullptr;
  std::vector< std::unique_ptr< DynamicLibrary > > dyn_libraries;
  std::unordered_map< std::string, std::unique_ptr< Pass > > ext_passes;
  std::vector< std::string > pass_seq;
  std::vector< std::string > pass_opts;

  /* -load= arguments are required to appear at the beginning, to handle
   * correctly optional options */
  for (int i = 0; i < argc; i++) {
    std::string opt(argv[i]);

    if (opt.length() > LOAD_OPT.length() &&
        opt.compare(0, LOAD_OPT.length(), LOAD_OPT) == 0) {
      std::size_t sep = opt.find('=');
      std::string dlib_path = opt.substr(sep + 1);
      std::unique_ptr< DynamicLibrary > dlib =
          std::make_unique< DynamicLibrary >();

      if (!dlib->open(dlib_path, RTLD_LAZY)) {
        std::cerr << "Error opening " << dlib->getPath() << std::endl;
        std::cerr << dlib->getError() << std::endl;
        return 2;
      } else {
        std::cout << "dlopen successful on " << dlib->getPath() << std::endl;

        Pass* (*fptr)();
        fptr = dlib->lookup< Pass* (*)() >("init");

        if (!fptr) {
          std::cerr << "Could not find init() in " << dlib->getPath()
                    << std::endl;
          return 1;
        }

        std::unique_ptr< Pass > pass(fptr());

        if (!pass) {
          std::cerr << "init() returned NULL in " << dlib->getPath()
                    << std::endl;
          return 1;
        }

        std::cout << "Loaded ARBOS pass: " << pass->getShortName() << " - "
                  << pass->getDescription() << std::endl;

        ext_passes[pass->getShortName()] = std::move(pass);
        dyn_libraries.push_back(std::move(dlib));
      }
    } else if (opt.compare(0, PASS_PREFIX.length(), PASS_PREFIX) == 0 &&
               ext_passes.find(opt.substr(PASS_PREFIX.length())) !=
                   ext_passes.end()) { // pass name
      pass_seq.push_back(opt.substr(PASS_PREFIX.length()));
    } else { // considered as an option for passes
      pass_opts.push_back(opt);
    }
  }

  /*
   * Command options: parse options provided by ARBOS passes
   */
  OptionParser parser;
  std::unordered_set< std::string > name_conflicts;
  Option< bool > help("help,h", "print this help message");

  // add all global options (note that it also includes help)
  for (OptionRegistry::iterator it = OptionRegistry::Get()->begin();
       it != OptionRegistry::Get()->end();
       ++it) {
    if (!parser.add(*it)) {
      // the option is still added, but it is also used by another pass.
      name_conflicts.insert(it->getLongName());
    }
  }

  if (name_conflicts.size() > 0) {
    std::cerr
        << "Warning: the following options are used by different passes: ";

    for (std::unordered_set< std::string >::const_iterator it =
             name_conflicts.begin();
         it != name_conflicts.end();) {
      std::cerr << *it++;
      if (it != name_conflicts.end()) {
        std::cerr << ", ";
      }
    }
    std::cerr << std::endl;
  }

  try {
    parser.parse(pass_opts);
  } catch (option_error& e) {
    std::cerr << "parse error: " << e.message() << std::endl;
    return 1;
  }

  if (help) {
    if (ext_passes.size() > 0 || name_conflicts.size() > 0) {
      std::cout << std::endl;
    }

    std::cout << "Usage: arbos -load=<path-to-dylib> "
              << "-<pass-short-name> [options] < <path-to-ar-file>.ar"
              << std::endl
              << std::endl;

    std::cout << "Modular AR optimizer and analyzer. "
              << "It takes an AR source file as input, " << std::endl
              << "and runs the specified passes on it." << std::endl
              << std::endl;

    if (ext_passes.size() > 0) {
      std::cout << "ARBOS Passes:" << std::endl;

      std::size_t padding = 0;
      for (std::unordered_map< std::string,
                               std::unique_ptr< Pass > >::const_iterator it =
               ext_passes.begin();
           it != ext_passes.end();
           ++it) {
        padding = std::max(padding, it->first.size() + 4);
      }

      for (std::unordered_map< std::string,
                               std::unique_ptr< Pass > >::const_iterator it =
               ext_passes.begin();
           it != ext_passes.end();
           ++it) {
        std::cout << "  -" << it->first
                  << std::string(padding - it->first.size() - 3, ' ')
                  << it->second->getDescription() << std::endl;
      }
      std::cout << std::endl;
    }

    std::cout << parser;
    return 0;
  }

  if (dyn_libraries.size() == 0) {
    std::cerr << "No passes loaded. Exiting..." << std::endl;
    return 1;
  }

  if (pass_seq.size() == 0) {
    std::cerr << "No passes specified for execution. Exiting..." << std::endl;
    return 1;
  }

  std::cout << pass_seq.size() << " pass(es) registered." << std::endl;

  /**
   * Load built-in AR builder strategies
   */
  ARBuilderRegistry ar_builder;
  ar_builder.registerBuilder(std::shared_ptr< ARBuilder >(new ImportAR()));

  ar_model = std::unique_ptr< ARModel >(ar_builder.build(AR_EXT));

  if (ar_model == nullptr || !ar_model->isModelComplete()) {
    std::cerr << "The internal AR representation cannot be created"
              << std::endl;
    return 1;
  }

  /**
   * Launch passes
   */
  try {
    for (std::vector< std::string >::iterator it = pass_seq.begin();
         it != pass_seq.end();
         it++) {
      Pass* pass = ext_passes[*it].get();
      std::cout << "Executing pass - " << pass->getShortName() << " "
                << pass->getDescription() << std::endl;
      pass->execute(
          ar_model->getBundle()); // method "execute" can throw exceptions
    }
  } catch (error& e) {
    std::cerr << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown error occurred " << std::endl;
    return 1;
  }

  return 0;
}
