#include "codegen.h"
#include "utils.h"
#include <codecvt>
#include <cstring>
#include <fstream>
#include <ios>
#include <iostream>

#include <vector>

namespace BFJit {
    [[clang::optnone]]
    Generator::Generator()
        : mPos(0), mMemCell(0) {
        mProgram = (uint8_t*)mmap(NULL, 8192, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        // Memory allocation
        // below code is
        // void *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        mProgram[mPos++] = 0xb8;
        std::memcpy(mProgram + mPos, ConvertU8<uint32_t>(9).data(), 4);
        mPos += 4;

        mProgram[mPos++] = 0xbf;
        std::memcpy(mProgram + mPos, ConvertU8<uint32_t>(0).data(), 4);
        mPos += 4;

        mProgram[mPos++] = 0xbe;
        std::memcpy(mProgram + mPos, ConvertU8<uint32_t>(128).data(), 4);
        mPos += 4;

        mProgram[mPos++] = 0xba;
        std::memcpy(mProgram + mPos, ConvertU8<uint32_t>(PROT_READ | PROT_WRITE | PROT_EXEC).data(), 4);
        mPos += 4;

        mProgram[mPos++] = 0x41;
        mProgram[mPos++] = 0xba;
        std::memcpy(mProgram + mPos, ConvertU8<uint32_t>(MAP_PRIVATE | MAP_ANONYMOUS).data(), 4);
        mPos += 4;

        mProgram[mPos++] = 0x49;
        mProgram[mPos++] = 0xc7;
        mProgram[mPos++] = 0xc0;
        mProgram[mPos++] = 0xff;
        mProgram[mPos++] = 0xff;
        mProgram[mPos++] = 0xff;
        mProgram[mPos++] = 0xff;

        mProgram[mPos++] = 0x0f;
        mProgram[mPos++] = 0x05;
    }
    
    auto Generator::AddOperation(const Generator::eOperation op) -> void {
        switch (op) {
            case OP_INC: {
                mProgram[mPos++] = 0x41;
                mProgram[mPos++] = 0x80;
                if (mMemCell == 0) {
                    mProgram[mPos++] = 0x01;
                } else {
                    mProgram[mPos++] = 0x41;
                    mProgram[mPos++] = mMemCell;
                }

                mProgram[mPos++] = 0x1;
                break;
            }

            case OP_DEC: {
                mProgram[mPos++] = 0x41;
                mProgram[mPos++] = 0x80;
                if (mMemCell == 0) {
                    mProgram[mPos++] = 0x29;
                } else {
                    mProgram[mPos++] = 0x69;
                    mProgram[mPos++] = mMemCell;
                }

                mProgram[mPos++] = 0x1;
                break;
            }

            case OP_NEXT: {
                mMemCell++;
                break;
            }

            case OP_PREV: {
                mMemCell--;
                break;
            }

            case OP_PRINT: {
                mProgram[mPos++] = 0xb8; // mov eax
                memcpy(mProgram + mPos, ConvertU8(1ul).data(), 4);
                mPos += 4;

                mProgram[mPos++] = 0xbf; // mov edi
                memcpy(mProgram + mPos, ConvertU8(1ul).data(), 4);
                mPos += 4;

                mProgram[mPos++] = 0x49; // lea rsi
                mProgram[mPos++] = 0x8d; // lea rsi

                if (mMemCell == 0) {
                    mProgram[mPos++] = 0x31; // lea rsi
                } else {
                    mProgram[mPos++] = 0x71;
                    mProgram[mPos++] = mMemCell;
                }

                mProgram[mPos++] = 0xba; // mov edx
                memcpy(mProgram + mPos, ConvertU8(1ul).data(), 4);
                mPos += 4;

                mProgram[mPos++] = 0x0f;
                mProgram[mPos++] = 0x05;
                break;
            }

            case OP_LOOPBEGIN: {
                mLoopStack.push(mPos);
                std::cout << std::hex << "[?] Setting jump to 0x" << mPos << "\n";
                break;
            }

            case OP_LOOPEND: {
                if (mLoopStack.empty()) {
                    std::cerr << "[!] Error: Unmatched loop end ']' found.\n";
                    exit(1);
                }

                int64_t loopStart = mLoopStack.top();
                mLoopStack.pop();

                if (loopStart >= mPos) {
                    std::cerr << "[!] Error: Invalid loop start position.\n";
                    exit(1);
                }

                uint64_t jmpV = loopStart - mPos - 12;
                std::cout << std::hex << "[?] Jumping to 0x" << jmpV << " whilst at 0x" << mPos << "\n";
                std::cout << std::dec << "[?] aka. jumping back " << (int64_t)(mPos - loopStart + 12) << " bytes\n";

                mProgram[mPos++] = 0x41;
                mProgram[mPos++] = 0x80;

                if (mMemCell == 0) {
                    mProgram[mPos++] = 0x39;
                } else {
                    mProgram[mPos++] = 0x79;
                    mProgram[mPos++] = mMemCell;
                }
                
                mProgram[mPos++] = 0x0;

                mProgram[mPos++] = 0x0f;
                mProgram[mPos++] = 0x85;
                std::memcpy(mProgram + mPos, ConvertU8<uint32_t>(jmpV).data(), 4);
                mPos += 4;

                break;
            }

            default: {
                break;
            }
        }
    }

    auto Generator::GetFinal() -> std::vector<uint8_t> {
        mProgram[mPos++] = 0x49;
        mProgram[mPos++] = 0x89;
        mProgram[mPos++] = 0xc1;
        mProgram[mPos++] = 0xc3;

        // typedef uint8_t* (*fn)();
        typedef void (*fn)();
        fn res = (fn) mProgram; 

        if (!res) {
            std::cerr << "[!] Invalid function pointer\n";
            exit(1);
        }

        // std::vector<uint8_t> programCopy(mPos);
        // memcpy(programCopy.data(), mProgram, mPos);

        // std::ofstream of("program.bin", std::ios::binary | std::ios::trunc);
        // of.write((char*)programCopy.data(), mPos);
        // of.close();

        std::vector<uint8_t> data;
        data.resize(128);

        asm(
            "callq *%0\n\t"
            "movq %%rax, %1"
            :
            : "r" (res), "r" (data.data())
        );

        // memcpy(v.data(), res(), 10000);
        return data;
    }
}