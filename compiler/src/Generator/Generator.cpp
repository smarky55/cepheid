#include "Generator.h"

using namespace Cepheid::Gen;

Cepheid::Gen::Generator::Generator(Parser::NodePtr root) : m_root(std::move(root)) {
}

std::string Cepheid::Gen::Generator::generate() {
  std::string prog = R"(bits 64
default rel

segment .text
global main
extern _CRT_INIT
extern ExitProcess

main:
  push rbp
  mov rbp, rsp
  sub rsp, 32

  call _CRT_INIT

  xor rax, rax
  mov rcx, 0
  call ExitProcess
)";
  return prog;
}
