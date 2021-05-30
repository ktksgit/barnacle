#include "polyhook2/Detour/PyCallback.hpp"

asmjit::CallConv::Id PLH::PyCallback::getCallConv(const std::string &conv)
{
    if (conv == "cdecl")
    {
        return asmjit::CallConv::kIdCDecl;
    }
    else if (conv == "stdcall")
    {
        return asmjit::CallConv::kIdStdCall;
    }
    else if (conv == "fastcall")
    {
        return asmjit::CallConv::kIdFastCall;
    }
    else if (conv == "thiscall")
    {
        return asmjit::CallConv::kIdThisCall;
    }

    throw std::invalid_argument("Unknown calling convention" + conv);
}

uint64_t PLH::PyCallback::getJitFunc(const uint32_t native_address, const asmjit::FuncSignature &sig, const asmjit::Environment::Arch arch)
{
    asmjit::CodeHolder code;
    auto env = asmjit::hostEnvironment();
    env.setArch(arch);
    code.init(env);

    // initialize function
    asmjit::x86::Compiler cc(&code);

    asmjit::FuncNode *func = cc.addFunc(asmjit::FuncSignatureT<void, Parameters *, ReturnValue *>());

    asmjit::StringLogger log;
    uint32_t kFormatFlags = asmjit::FormatOptions::kFlagMachineCode | asmjit::FormatOptions::kFlagExplainImms | asmjit::FormatOptions::kFlagRegCasts | asmjit::FormatOptions::kFlagAnnotations | asmjit::FormatOptions::kFlagDebugPasses | asmjit::FormatOptions::kFlagDebugRA | asmjit::FormatOptions::kFlagHexImms | asmjit::FormatOptions::kFlagHexOffsets | asmjit::FormatOptions::kFlagPositions;
    log.addFlags(kFormatFlags);
    code.setLogger(&log);

    // too small to really need it
    func->frame().resetPreservedFP();

    constexpr uint32_t QWORD_SIZE = sizeof(uint64_t);

    asmjit::x86::Gp argStruct = cc.newUIntPtr("argStruct");
    cc.setArg(0, argStruct);

    // asmjit::x86::Mem argsStack = cc.newStack(stackSize, 16);
    asmjit::x86::Mem argsStack = asmjit::x86::ptr(argStruct);

    asmjit::x86::Gp retStruct = cc.newUIntPtr("retStruct");
    cc.setArg(1, retStruct);

    std::vector<asmjit::x86::Reg> argRegisters;
    for (uint8_t argIdx = 0; argIdx < sig.argCount(); argIdx++)
    {
        const uint8_t argType = sig.args()[argIdx];

        asmjit::x86::Reg arg;
        if (isGeneralReg(argType))
        {
            arg = cc.newUIntPtr();
        }
        else if (isXmmReg(argType))
        {
            arg = cc.newXmm();
        }
        else
        {
            Log::log("Parameters wider than 64bits not supported", ErrorLevel::SEV);
            return 0;
        }

        argRegisters.push_back(arg);
    }

    // mov from stack to arguments
    for (uint8_t argIdx = 0; argIdx < sig.argCount(); argIdx++)
    {
        const uint8_t argType = sig.args()[argIdx];

        // have to cast back to explicit register types to gen right mov type
        if (isGeneralReg(argType))
        {
            cc.mov(argRegisters[argIdx].as<asmjit::x86::Gp>(), argsStack);
        }
        else if (isXmmReg(argType))
        {
            cc.movq(argRegisters[argIdx].as<asmjit::x86::Xmm>(), argsStack);
        }
        else
        {
            Log::log("Parameters wider than 64bits not supported", ErrorLevel::SEV);
            return 0;
        }

        argsStack.addOffset(QWORD_SIZE);
    }

    asmjit::InvokeNode *invokeNode;
    cc.invoke(&invokeNode, native_address, sig);
    for (uint8_t argIdx = 0; argIdx < sig.argCount(); argIdx++)
    {
        invokeNode->setArg(argIdx, argRegisters[argIdx]);
    }

    asmjit::x86::Reg trampoline_ret;
    if (sig.hasRet())
    {
        if (asmjit::Type::isFloat(sig.ret()))
        {
            trampoline_ret = cc.newXmm();
        }
        else
        {
            trampoline_ret = cc.newUInt32();
        }

        // float and double do not need extra code because return value is in FPU's ST0
        invokeNode->setRet(0, trampoline_ret);

        asmjit::x86::Mem ret_memory = asmjit::x86::ptr(retStruct);

        const uint8_t retType = sig.ret();

        // have to cast back to explicit register types to gen right mov type
        if (isGeneralReg(retType))
        {
            cc.mov(ret_memory, trampoline_ret.as<asmjit::x86::Gp>());
        }
        else if (isXmmReg(retType))
        {
            cc.movq(ret_memory, trampoline_ret.as<asmjit::x86::Xmm>());
        }
        else
        {
            Log::log("Return type wider than 64bits not supported", ErrorLevel::SEV);
            return 0;
        }
    }

    cc.endFunc();
    // write to buffer
    cc.finalize();

    // worst case, overestimates for case trampolines needed
    code.flatten();
    size_t size = code.codeSize();

    // Allocate a virtual memory (executable).
    m_callbackBuf = (uint64_t)m_mem.getBlock(size);
    if (!m_callbackBuf)
    {
        __debugbreak();
        return 0;
    }

    // if multiple sections, resolve linkage (1 atm)
    if (code.hasUnresolvedLinks())
    {
        code.resolveUnresolvedLinks();
    }

    // Relocate to the base-address of the allocated memory.
    code.relocateToBase(m_callbackBuf);
    code.copyFlattenedData((unsigned char *)m_callbackBuf, size);

    Log::log("JIT Stub:\n" + std::string(log.data()), ErrorLevel::INFO);
    return m_callbackBuf;
}

uint64_t PLH::PyCallback::getJitFunc(const uint32_t unique_id, const asmjit::FuncSignature &sig,
                                     const asmjit::Environment::Arch arch, const PLH::PyCallback::tUserCallback callback, bool use_trampoline)
{
    asmjit::CodeHolder code;
    auto env = asmjit::hostEnvironment();
    env.setArch(arch);
    code.init(env);

    // initialize function
    asmjit::x86::Compiler cc(&code);
    asmjit::FuncNode *func = cc.addFunc(sig);

    asmjit::StringLogger log;
    uint32_t kFormatFlags = asmjit::FormatOptions::kFlagMachineCode | asmjit::FormatOptions::kFlagExplainImms | asmjit::FormatOptions::kFlagRegCasts | asmjit::FormatOptions::kFlagAnnotations | asmjit::FormatOptions::kFlagDebugPasses | asmjit::FormatOptions::kFlagDebugRA | asmjit::FormatOptions::kFlagHexImms | asmjit::FormatOptions::kFlagHexOffsets | asmjit::FormatOptions::kFlagPositions;
    log.addFlags(kFormatFlags);
    code.setLogger(&log);

    // too small to really need it
    func->frame().resetPreservedFP();

    constexpr uint32_t QWORD_SIZE = sizeof(uint64_t);

    // map argument slots to registers, following abi.
    std::vector<asmjit::x86::Reg> argRegisters;
    for (uint8_t argIdx = 0; argIdx < sig.argCount(); argIdx++)
    {
        const uint8_t argType = sig.args()[argIdx];

        asmjit::x86::Reg arg;
        if (isGeneralReg(argType))
        {
            arg = cc.newUIntPtr();
        }
        else if (isXmmReg(argType))
        {
            arg = cc.newXmm();
        }
        else
        {
            Log::log("Parameters wider than 64bits not supported", ErrorLevel::SEV);
            return 0;
        }

        cc.setArg(argIdx, arg);
        argRegisters.push_back(arg);
    }

    // setup the stack structure to hold arguments for user callback
    uint32_t stackSize = (uint32_t)(QWORD_SIZE * sig.argCount());
    asmjit::x86::Mem argsStack = cc.newStack(stackSize, 16);

    //// mov from arguments registers into the stack structure
    for (uint8_t argIdx = 0; argIdx < sig.argCount(); argIdx++)
    {
        const uint8_t argType = sig.args()[argIdx];

        // have to cast back to explicit register types to gen right mov type
        if (isGeneralReg(argType))
        {
            cc.mov(argsStack, argRegisters[argIdx].as<asmjit::x86::Gp>());
        }
        else if (isXmmReg(argType))
        {
            cc.movq(argsStack, argRegisters[argIdx].as<asmjit::x86::Xmm>());
        }
        else
        {
            Log::log("Parameters wider than 64bits not supported", ErrorLevel::SEV);
            return 0;
        }

        argsStack.addOffset(QWORD_SIZE);
    }

    // get pointer to stack structure and pass it to the user callback
    asmjit::x86::Gp argStruct = cc.newUIntPtr("argStruct");
    argsStack.resetOffset();
    cc.lea(argStruct, argsStack);

    // fill reg to pass struct arg count to callback
    asmjit::x86::Gp argCountParam = cc.newU8();
    cc.mov(argCountParam, (uint8_t)sig.argCount());

    // create buffer for ret val
    asmjit::x86::Mem retStack = cc.newStack(1 * QWORD_SIZE, 16);
    asmjit::x86::Gp retStruct = cc.newUIntPtr("retStruct");
    cc.lea(retStruct, retStack);

    asmjit::x86::Gp param0 = cc.newInt32("tmp");
    cc.mov(param0, unique_id);

    asmjit::InvokeNode *invokeNode;
    cc.invoke(&invokeNode,
              reinterpret_cast<uint32_t>(callback),
              asmjit::FuncSignatureT<void, uint32_t, Parameters *, uint8_t, ReturnValue *>());

    // call to user provided function (use ABI of host compiler)
    invokeNode->setArg(0, param0);
    invokeNode->setArg(1, argStruct);
    invokeNode->setArg(2, argCountParam);
    invokeNode->setArg(3, retStruct);

    argsStack.resetOffset();
    for (uint8_t arg_idx = 0; arg_idx < sig.argCount(); arg_idx++)
    {
        const uint8_t argType = sig.args()[arg_idx];

        if (isGeneralReg(argType))
        {
            cc.mov(argRegisters[arg_idx].as<asmjit::x86::Gp>(), argsStack);
        }
        else if (isXmmReg(argType))
        {
            cc.movq(argRegisters[arg_idx].as<asmjit::x86::Xmm>(), argsStack);
        }
        else
        {
            Log::log("Parameters wider than 64bits not supported", ErrorLevel::SEV);
            return 0;
        }

        // next structure slot (+= sizeof(uint64_t))
        argsStack.addOffset(QWORD_SIZE);
    }

    if (use_trampoline)
    {
        asmjit::x86::Reg trampoline_ret;
        if (sig.hasRet() && asmjit::Type::isFloat(sig.ret()))
        {
            trampoline_ret = cc.newXmm();
        }
        else
        {
            trampoline_ret = cc.newUInt32();
        }

        asmjit::x86::Gp origPtr = cc.newUIntPtr("trampoline_holder");

        cc.mov(origPtr, (uintptr_t)getTrampolineHolder());
        cc.mov(origPtr, asmjit::x86::ptr(origPtr));

        asmjit::InvokeNode *origInvokeNode;
        cc.invoke(&origInvokeNode, origPtr, sig);
        for (uint8_t argIdx = 0; argIdx < sig.argCount(); argIdx++)
        {
            origInvokeNode->setArg(argIdx, argRegisters[argIdx]);
        }

        if (sig.hasRet() && !asmjit::Type::isFloat(sig.ret()))
        {
            // float and double do not need extra code because return value is in FPU's ST0
            origInvokeNode->setRet(0, trampoline_ret);
            cc.ret(trampoline_ret);
        }
    }
    else if (sig.hasRet())
    {
        asmjit::x86::Mem retStackIdx(retStack);
        retStackIdx.setSize(QWORD_SIZE);
        if (isGeneralReg((uint8_t)sig.ret()))
        {
            asmjit::x86::Gp tmp2 = cc.newUIntPtr();
            cc.mov(tmp2, retStackIdx);
            cc.ret(tmp2);
        }
        else
        {
            asmjit::x86::Xmm tmp2 = cc.newXmm();
            cc.movq(tmp2, retStackIdx);
            cc.ret(tmp2);
        }
    }

    cc.endFunc();
    // write to buffer
    cc.finalize();

    // worst case, overestimates for case trampolines needed
    code.flatten();
    size_t size = code.codeSize();

    // Allocate a virtual memory (executable).
    m_callbackBuf = (uint64_t)m_mem.getBlock(size);
    if (!m_callbackBuf)
    {
        __debugbreak();
        return 0;
    }

    // if multiple sections, resolve linkage (1 atm)
    if (code.hasUnresolvedLinks())
    {
        code.resolveUnresolvedLinks();
    }

    // Relocate to the base-address of the allocated memory.
    code.relocateToBase(m_callbackBuf);
    code.copyFlattenedData((unsigned char *)m_callbackBuf, size);

    Log::log("JIT Stub:\n" + std::string(log.data()), ErrorLevel::INFO);
    return m_callbackBuf;
}

uint64_t PLH::PyCallback::getJitFunc(const uint32_t unique_id, const std::string &retType, const std::vector<std::string> &paramTypes, const tUserCallback callback, std::string callConv)
{
    asmjit::FuncSignature sig = {};
    std::vector<uint8_t> args;
    for (const std::string &s : paramTypes)
    {
        args.push_back(getTypeId(s));
    }
    sig.init(getCallConv(callConv), asmjit::FuncSignature::kNoVarArgs, getTypeId(retType), args.data(), (uint32_t)args.size());
    return getJitFunc(unique_id, sig, asmjit::Environment::kArchHost, callback);
}
