#ifndef ARMJIT_COMPILER_H
#define ARMJIT_COMPILER_H

#include "../dolphin/x64Emitter.h"

#include "../ARMJIT.h"
#include "../ARMJIT_RegisterCache.h"

namespace ARMJIT
{

const Gen::X64Reg RCPU = Gen::RBP;
const Gen::X64Reg RCPSR = Gen::R15;

const Gen::X64Reg RSCRATCH = Gen::EAX;
const Gen::X64Reg RSCRATCH2 = Gen::EDX;
const Gen::X64Reg RSCRATCH3 = Gen::ECX;


class Compiler : public Gen::XEmitter
{
public:
    Compiler();

    void Reset();

    CompiledBlock CompileBlock(ARM* cpu, FetchedInstr instrs[], int instrsCount);

    void LoadReg(int reg, Gen::X64Reg nativeReg);
    void SaveReg(int reg, Gen::X64Reg nativeReg);

    bool CanCompile(bool thumb, u16 kind);

    typedef void (Compiler::*CompileFunc)();

    void Comp_JumpTo(Gen::X64Reg addr, bool restoreCPSR = false);
    void Comp_JumpTo(u32 addr, bool forceNonConstantCycles = false);

    void Comp_AddCycles_C(bool forceNonConstant = false);
    void Comp_AddCycles_CI(u32 i);
    void Comp_AddCycles_CI(Gen::X64Reg i, int add);

    enum
    {
        opSetsFlags = 1 << 0,
        opSymmetric = 1 << 1,
        opRetriveCV = 1 << 2,
        opInvertCarry = 1 << 3,
        opSyncCarry = 1 << 4,
        opInvertOp2 = 1 << 5,
    };

    void A_Comp_Arith();
    void A_Comp_MovOp();
    void A_Comp_CmpOp();

    void A_Comp_MUL_MLA();
    void A_Comp_SMULL_SMLAL();

    void A_Comp_CLZ();
    
    void A_Comp_MemWB();
    void A_Comp_MemHalf();
    void A_Comp_LDM_STM();

    void A_Comp_BranchImm();
    void A_Comp_BranchXchangeReg();

    void T_Comp_ShiftImm();
    void T_Comp_AddSub_();
    void T_Comp_ALU_Imm8();
    void T_Comp_ALU();
    void T_Comp_ALU_HiReg();
    void T_Comp_MUL();

    void T_Comp_RelAddr();
    void T_Comp_AddSP();

    void T_Comp_MemReg();
    void T_Comp_MemImm();
    void T_Comp_MemRegHalf();
    void T_Comp_MemImmHalf();
    void T_Comp_LoadPCRel();
    void T_Comp_MemSPRel();
    void T_Comp_PUSH_POP();
    void T_Comp_LDMIA_STMIA();

    void T_Comp_BCOND();
    void T_Comp_B();
    void T_Comp_BranchXchangeReg();
    void T_Comp_BL_LONG_1();
    void T_Comp_BL_LONG_2();
    void T_Comp_BL_Merged(FetchedInstr prefix);

    void Comp_MemAccess(Gen::OpArg rd, bool signExtend, bool store, int size);
    s32 Comp_MemAccessBlock(int rn, BitSet16 regs, bool store, bool preinc, bool decrement, bool usermode);

    void Comp_ArithTriOp(void (Compiler::*op)(int, const Gen::OpArg&, const Gen::OpArg&), 
        Gen::OpArg rd, Gen::OpArg rn, Gen::OpArg op2, bool carryUsed, int opFlags);
    void Comp_ArithTriOpReverse(void (Compiler::*op)(int, const Gen::OpArg&, const Gen::OpArg&),
        Gen::OpArg rd, Gen::OpArg rn, Gen::OpArg op2, bool carryUsed, int opFlags);
    void Comp_CmpOp(int op, Gen::OpArg rn, Gen::OpArg op2, bool carryUsed);

    void Comp_MulOp(bool S, bool add, Gen::OpArg rd, Gen::OpArg rm, Gen::OpArg rs, Gen::OpArg rn);

    void Comp_RetriveFlags(bool sign, bool retriveCV, bool carryUsed);

    void* Gen_MemoryRoutine9(bool store, int size);
    void* Gen_MemoryRoutine7(bool store, bool codeMainRAM, int size);

    void* Gen_MemoryRoutineSeq9(bool store, bool preinc);
    void* Gen_MemoryRoutineSeq7(bool store, bool preinc, bool codeMainRAM);

    void* Gen_ChangeCPSRRoutine();

    Gen::OpArg Comp_RegShiftImm(int op, int amount, Gen::OpArg rm, bool S, bool& carryUsed);
    Gen::OpArg Comp_RegShiftReg(int op, Gen::OpArg rs, Gen::OpArg rm, bool S, bool& carryUsed);

    Gen::OpArg A_Comp_GetALUOp2(bool S, bool& carryUsed);
    Gen::OpArg A_Comp_GetMemWBOffset();

    void LoadCPSR();
    void SaveCPSR();

    bool FlagsNZRequired()
    { return CurInstr.SetFlags & 0xC; }

    Gen::FixupBranch CheckCondition(u32 cond);

    Gen::OpArg MapReg(int reg)
    {
        if (reg == 15 && RegCache.Mapping[reg] == Gen::INVALID_REG)
            return Gen::Imm32(R15);

        assert(RegCache.Mapping[reg] != Gen::INVALID_REG);
        return Gen::R(RegCache.Mapping[reg]);
    }

    u8* ResetStart;
    u32 CodeMemSize;

    void* MemoryFuncs9[3][2];
    void* MemoryFuncs7[3][2][2];

    void* MemoryFuncsSeq9[2][2];
    void* MemoryFuncsSeq7[2][2][2];

    void* ReadBanked;
    void* WriteBanked;

    bool CPSRDirty = false;

    FetchedInstr CurInstr;

    RegisterCache<Compiler, Gen::X64Reg> RegCache;

    bool Thumb;
    u32 Num;
    u32 R15;
    u32 CodeRegion;

    u32 ConstantCycles;

    ARM* CurCPU;
};

}

#endif