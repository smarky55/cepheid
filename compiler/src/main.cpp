
#include <Compiler.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static std::string_view usage() {
  return "Usage: cepheid <input.cep> <output>";
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << usage() << std::endl;
    return EXIT_SUCCESS;
  }

  std::vector<std::string> args{argv + 1, argv + argc};

  if (args.size() != 2) {
    std::cerr << "Invalid number of arguments.\n" << usage() << std::endl;
    return EXIT_FAILURE;
  }

  std::string src;
  {
    std::ifstream infile(args[0]);
    std::stringstream ss;
    ss << infile.rdbuf();
    src = ss.str();
  }

  Cepheid::Compiler cep;

  std::string prog = cep.compile(src);

  {
    std::ofstream asmOut("out.asm");
    asmOut << prog;
  }
}