
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

  if (int ret = system("nasm -f win64 -o example.obj out.asm 2>&1"); ret != 0) {
    std::cout << "Assemble failed with code:" << ret << std::endl;
    return ret;
  }

  std::stringstream link;
  link << "\"C:\\Program Files\\Microsoft Visual "
          "Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat\" amd64 10.0.22000.0 && ";
  link << "link.exe .\\example.obj /subsystem:console /entry:_entry /out:" << args[1] << " kernel32.lib msvcrt.lib";

  if (int ret = system(link.str().c_str()); ret != 0) {
    std::cout << "Link failed with code:" << ret << std::endl;
    return ret;
  }
}