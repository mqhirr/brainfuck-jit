#ifndef CODEGEN_H
#define CODEGEN_H

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <sys/mman.h>
#include <unistd.h>

#include <vector>

#include <stack>

namespace BFJit {
    class Generator {
    public:
        enum eOperation {
            OP_INC,
            OP_DEC,
            OP_NEXT,
            OP_PREV,
            OP_PRINT,
            OP_LOOPBEGIN,
            OP_LOOPEND,
            OP_UNIMPL,
        };

    public:
        Generator();
        ~Generator() {
            if (mProgram)
                munmap(mProgram, 8192);
        }

    public:
        auto AddOperation(const eOperation op) -> void;
        auto GetFinal() -> std::vector<uint8_t>;

    private:
        uint8_t* mProgram;
        int64_t mPos;

        std::stack<int64_t> mLoopStack;

        int64_t mMemCell;
    };
}

#endif // CODEGEN_H