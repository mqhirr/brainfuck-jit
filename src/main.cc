#include "codegen.h"
#include "lexer.h"
#include <fstream>
#include <iostream>

#include <filesystem>
#include <iterator>

auto main(int argc, char** argv) -> int {
    // std::ios_base::sync_with_stdio(false);
    std::string filename = "test.bf";
    if (argc >= 2) {
        filename = argv[1];
    }

    std::ifstream fs(filename);
    std::string content{
        std::istreambuf_iterator<char>(fs),
        std::istreambuf_iterator<char>()
    };

    BFJit::Lexer lex(content);
    auto ops = lex.GetOps();

    BFJit::Generator *gen = new BFJit::Generator();
    for (const auto& it : ops) {
        gen->AddOperation(it);
    }

    auto cells = gen->GetFinal();
   
    std::stringstream str;
    str << "1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\n";
    for (int i = 0; i < 16; i++) {
        str << (uint16_t)cells[i] << "\t";
    }

    std::cout << str.str() << "\n";

    delete gen;

    return 0;
}