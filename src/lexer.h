#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <string_view>

#include "codegen.h"

namespace BFJit {
    class Lexer {
    public:
        Lexer(std::string_view v)
            : mLex(v), mPos(0) {}

    public:
        auto Peek() { return mLex[mPos + 1]; }
        auto Get() { return mLex[mPos++]; }
        auto Current() { return mLex[mPos]; }

    public:
        auto GetOps() -> std::vector<Generator::eOperation> {
            using OpType = Generator::eOperation;
            std::vector<Generator::eOperation> v;
            while (Current() != '\0') {
                switch (Current()) {
                    case '+':
                        v.push_back(OpType::OP_INC);
                        break;
                    case '-':
                        v.push_back(OpType::OP_DEC);
                        break;
                    case '>':
                        v.push_back(OpType::OP_NEXT);
                        break;
                    case '<':
                        v.push_back(OpType::OP_PREV);
                        break;
                    case '.':
                        v.push_back(OpType::OP_PRINT);
                        break;
                    case '[':
                        v.push_back(OpType::OP_LOOPBEGIN);
                        break;
                    case ']':
                        v.push_back(OpType::OP_LOOPEND);
                        break;
                    default:
                        v.push_back(OpType::OP_UNIMPL);
                        break;
                }

                Get();
            }

            return v;
        }

    private:
        std::string mLex;
        uint64_t mPos;
    };
}

#endif // LEXER_H