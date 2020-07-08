#include <memory>

#include <Catch.hpp>

#include "polyhook2/Virtuals/VFuncSwapHook.hpp"
#include "polyhook2/Tests/TestEffectTracker.hpp"

EffectTracker vFuncSwapEffects;

class VirtualTest2 {
public:
	virtual ~VirtualTest2() {}

	virtual int NoParamVirt() {
		return 4;
	}

	virtual int NoParamVirt2() {
		return 7;
	}
};

#pragma warning(disable: 4100)

typedef int(__thiscall* tVirtNoParams)(uintptr_t pThis);
PLH::VFuncMap origVFuncs2;

NOINLINE int __fastcall hkVirtNoParams2(uintptr_t pThis) {
	vFuncSwapEffects.PeakEffect().trigger();
	return ((tVirtNoParams)origVFuncs2.at(1))(pThis);
}

NOINLINE int __fastcall hkVirt2NoParams2(uintptr_t pThis) {
	vFuncSwapEffects.PeakEffect().trigger();
	return ((tVirtNoParams)origVFuncs2.at(2))(pThis);
}

TEST_CASE("VFuncSwap tests", "[VFuncSwap]") {
	std::shared_ptr<VirtualTest2> ClassToHook(new VirtualTest2);

	SECTION("Verify vfunc redirected") {
		PLH::VFuncMap redirect = {{(uint16_t)1, (uint64_t)&hkVirtNoParams2}};
		PLH::VFuncSwapHook hook((char*)ClassToHook.get(), redirect, &origVFuncs2);
		REQUIRE(hook.hook());
		REQUIRE(origVFuncs2.size() == 1);

		vFuncSwapEffects.PushEffect();
		ClassToHook->NoParamVirt();
		REQUIRE(vFuncSwapEffects.PopEffect().didExecute());
		REQUIRE(hook.unHook());
	}

	SECTION("Verify multiple vfunc redirected") {
		PLH::VFuncMap redirect = {{(uint16_t)1, (uint64_t)&hkVirtNoParams2},{(uint16_t)2, (uint64_t)&hkVirt2NoParams2}};
		PLH::VFuncSwapHook hook((char*)ClassToHook.get(), redirect, &origVFuncs2);
		REQUIRE(hook.hook());
		REQUIRE(origVFuncs2.size() == 2);

		vFuncSwapEffects.PushEffect();
		ClassToHook->NoParamVirt();
		REQUIRE(vFuncSwapEffects.PopEffect().didExecute());

		vFuncSwapEffects.PushEffect();
		ClassToHook->NoParamVirt2();
		REQUIRE(vFuncSwapEffects.PopEffect().didExecute());
		REQUIRE(hook.unHook());
	}
}