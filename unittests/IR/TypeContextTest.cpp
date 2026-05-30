/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/IR/TypeContext.h"

#include "hermes/IR/IR.h"

#include "llvh/Support/raw_ostream.h"

#include "gtest/gtest.h"

namespace hermes {

/// Test fixture for TypeContext unit tests. Provides an asType()
/// helper that constructs a Type from a raw well-known ID. This is
/// needed for tests of refined kinds (UInt31, Int32, Uint32, Bits32)
/// that have no public Type::createXxx() factory in Phase 1. The
/// fixture is declared friend of Type in IR.h, so the call to the
/// private Type(uint32_t) constructor is legal here.
class TypeContextTest : public ::testing::Test {
 protected:
  static Type asType(uint32_t id) {
    return Type(id);
  }
};

} // namespace hermes

using namespace hermes;

namespace {

TEST_F(TypeContextTest, LeafKinds) {
  TypeContext ctx;

  EXPECT_EQ(ctx.getKind(Type::createNoType()), TypeKind::NoType);
  EXPECT_EQ(ctx.getKind(Type::createEmpty()), TypeKind::Empty);
  EXPECT_EQ(ctx.getKind(Type::createUninit()), TypeKind::Uninit);
  EXPECT_EQ(ctx.getKind(Type::createUndefined()), TypeKind::Undefined);
  EXPECT_EQ(ctx.getKind(Type::createNull()), TypeKind::Null);
  EXPECT_EQ(ctx.getKind(Type::createBoolean()), TypeKind::Boolean);
  EXPECT_EQ(ctx.getKind(Type::createString()), TypeKind::String);
  EXPECT_EQ(ctx.getKind(Type::createNumber()), TypeKind::Number);
  EXPECT_EQ(ctx.getKind(Type::createBigInt()), TypeKind::BigInt);
  EXPECT_EQ(ctx.getKind(Type::createSymbol()), TypeKind::Symbol);
  EXPECT_EQ(ctx.getKind(Type::createEnvironment()), TypeKind::Environment);
  EXPECT_EQ(ctx.getKind(Type::createPrivateName()), TypeKind::PrivateName);
  EXPECT_EQ(ctx.getKind(Type::createFunctionCode()), TypeKind::FunctionCode);
  EXPECT_EQ(ctx.getKind(Type::createObject()), TypeKind::Object);
  EXPECT_EQ(ctx.getKind(asType(kBits32Id)), TypeKind::Bits32);
}

TEST_F(TypeContextTest, AnyTypeIsUnion) {
  TypeContext ctx;

  EXPECT_EQ(ctx.getKind(Type::createAnyType()), TypeKind::Union);
  auto arms = ctx.getUnionArms(Type::createAnyType());
  // AnyType = Undefined | Null | Boolean | String | Number | BigInt | Symbol |
  // Object.
  EXPECT_EQ(arms.size(), 8u);

  // Verify all expected arms are present.
  auto contains = [&](Type t) {
    for (auto arm : arms)
      if (arm == t)
        return true;
    return false;
  };
  EXPECT_TRUE(contains(Type::createUndefined()));
  EXPECT_TRUE(contains(Type::createNull()));
  EXPECT_TRUE(contains(Type::createBoolean()));
  EXPECT_TRUE(contains(Type::createString()));
  EXPECT_TRUE(contains(Type::createNumber()));
  EXPECT_TRUE(contains(Type::createBigInt()));
  EXPECT_TRUE(contains(Type::createSymbol()));
  EXPECT_TRUE(contains(Type::createObject()));

  // Should NOT contain internal types.
  EXPECT_FALSE(contains(Type::createEmpty()));
  EXPECT_FALSE(contains(Type::createUninit()));
  EXPECT_FALSE(contains(Type::createEnvironment()));
  EXPECT_FALSE(contains(Type::createPrivateName()));
  EXPECT_FALSE(contains(Type::createFunctionCode()));
}

TEST_F(TypeContextTest, NumericIsUnion) {
  TypeContext ctx;

  EXPECT_EQ(ctx.getKind(Type::createNumeric()), TypeKind::Union);
  auto arms = ctx.getUnionArms(Type::createNumeric());
  EXPECT_EQ(arms.size(), 2u);

  bool hasNumber = false, hasBigInt = false;
  for (auto arm : arms) {
    if (arm == Type::createNumber())
      hasNumber = true;
    if (arm == Type::createBigInt())
      hasBigInt = true;
  }
  EXPECT_TRUE(hasNumber);
  EXPECT_TRUE(hasBigInt);
}

TEST_F(TypeContextTest, AnyEmptyUninitIsUnion) {
  TypeContext ctx;

  EXPECT_EQ(ctx.getKind(Type::createAnyEmptyUninit()), TypeKind::Union);
  auto arms = ctx.getUnionArms(Type::createAnyEmptyUninit());
  // All AnyType arms + Empty + Uninit = 10.
  EXPECT_EQ(arms.size(), 10u);

  auto contains = [&](Type t) {
    for (auto arm : arms)
      if (arm == t)
        return true;
    return false;
  };
  EXPECT_TRUE(contains(Type::createEmpty()));
  EXPECT_TRUE(contains(Type::createUninit()));
  EXPECT_TRUE(contains(Type::createUndefined()));
  EXPECT_TRUE(contains(Type::createNull()));
  EXPECT_TRUE(contains(Type::createBoolean()));
  EXPECT_TRUE(contains(Type::createString()));
  EXPECT_TRUE(contains(Type::createNumber()));
  EXPECT_TRUE(contains(Type::createBigInt()));
  EXPECT_TRUE(contains(Type::createSymbol()));
  EXPECT_TRUE(contains(Type::createObject()));
}

TEST_F(TypeContextTest, NullOrUndefIsUnion) {
  TypeContext ctx;

  EXPECT_EQ(ctx.getKind(Type::createNullOrUndef()), TypeKind::Union);
  auto arms = ctx.getUnionArms(Type::createNullOrUndef());
  EXPECT_EQ(arms.size(), 2u);

  bool hasUndefined = false, hasNull = false;
  for (auto arm : arms) {
    if (arm == Type::createUndefined())
      hasUndefined = true;
    if (arm == Type::createNull())
      hasNull = true;
  }
  EXPECT_TRUE(hasUndefined);
  EXPECT_TRUE(hasNull);
}

TEST_F(TypeContextTest, ReservedSlots) {
  TypeContext ctx;
  // Padding slots between the last well-known union and kFirstDynamicId
  // should be NoType placeholders.
  for (uint32_t i = _kLastUnionId + 1; i < kFirstDynamicId; ++i) {
    EXPECT_EQ(ctx.getKind(asType(i)), TypeKind::NoType)
        << "Reserved slot " << i << " should be NoType";
  }
}

TEST_F(TypeContextTest, CanBeNumberLeaf) {
  TypeContext ctx;

  EXPECT_TRUE(ctx.canBeNumber(Type::createNumber()));
  EXPECT_FALSE(ctx.canBeNumber(Type::createString()));
  EXPECT_FALSE(ctx.canBeNumber(Type::createObject()));
  EXPECT_FALSE(ctx.canBeNumber(Type::createNoType()));
  EXPECT_FALSE(ctx.canBeNumber(Type::createBoolean()));
}

TEST_F(TypeContextTest, CanBeNumberUnion) {
  TypeContext ctx;

  // AnyType is a union containing Number.
  EXPECT_TRUE(ctx.canBeNumber(Type::createAnyType()));
  // Numeric = Number | BigInt.
  EXPECT_TRUE(ctx.canBeNumber(Type::createNumeric()));
  // AnyEmptyUninit contains Number.
  EXPECT_TRUE(ctx.canBeNumber(Type::createAnyEmptyUninit()));
  // NullOrUndef does not contain Number.
  EXPECT_FALSE(ctx.canBeNumber(Type::createNullOrUndef()));
}

TEST_F(TypeContextTest, CanBeOtherKinds) {
  TypeContext ctx;

  EXPECT_TRUE(ctx.canBeString(Type::createString()));
  EXPECT_FALSE(ctx.canBeString(Type::createNumber()));
  EXPECT_TRUE(ctx.canBeString(Type::createAnyType()));

  EXPECT_TRUE(ctx.canBeObject(Type::createObject()));
  EXPECT_FALSE(ctx.canBeObject(Type::createNumber()));
  EXPECT_TRUE(ctx.canBeObject(Type::createAnyType()));

  EXPECT_TRUE(ctx.canBeNull(Type::createNull()));
  EXPECT_FALSE(ctx.canBeNull(Type::createNumber()));
  EXPECT_TRUE(ctx.canBeNull(Type::createAnyType()));
  EXPECT_TRUE(ctx.canBeNull(Type::createNullOrUndef()));

  EXPECT_TRUE(ctx.canBeUndefined(Type::createUndefined()));
  EXPECT_FALSE(ctx.canBeUndefined(Type::createNumber()));
  EXPECT_TRUE(ctx.canBeUndefined(Type::createAnyType()));
  EXPECT_TRUE(ctx.canBeUndefined(Type::createNullOrUndef()));

  EXPECT_TRUE(ctx.canBeEmpty(Type::createEmpty()));
  EXPECT_FALSE(ctx.canBeEmpty(Type::createNumber()));
  EXPECT_FALSE(ctx.canBeEmpty(Type::createAnyType()));
  EXPECT_TRUE(ctx.canBeEmpty(Type::createAnyEmptyUninit()));

  EXPECT_TRUE(ctx.canBeUninit(Type::createUninit()));
  EXPECT_FALSE(ctx.canBeUninit(Type::createNumber()));
  EXPECT_FALSE(ctx.canBeUninit(Type::createAnyType()));
  EXPECT_TRUE(ctx.canBeUninit(Type::createAnyEmptyUninit()));

  EXPECT_TRUE(ctx.canBeBigInt(Type::createBigInt()));
  EXPECT_FALSE(ctx.canBeBigInt(Type::createString()));
  EXPECT_TRUE(ctx.canBeBigInt(Type::createNumeric()));
  EXPECT_TRUE(ctx.canBeBigInt(Type::createAnyType()));

  EXPECT_TRUE(ctx.canBeBoolean(Type::createBoolean()));
  EXPECT_FALSE(ctx.canBeBoolean(Type::createString()));
  EXPECT_TRUE(ctx.canBeBoolean(Type::createAnyType()));

  EXPECT_TRUE(ctx.canBeSymbol(Type::createSymbol()));
  EXPECT_FALSE(ctx.canBeSymbol(Type::createString()));
  EXPECT_TRUE(ctx.canBeSymbol(Type::createAnyType()));
}

TEST_F(TypeContextTest, IsNoType) {
  TypeContext ctx;

  EXPECT_TRUE(ctx.isNoType(Type::createNoType()));
  EXPECT_FALSE(ctx.isNoType(Type::createNumber()));
  EXPECT_FALSE(ctx.isNoType(Type::createAnyType()));
}

TEST_F(TypeContextTest, IsPrimitive) {
  TypeContext ctx;

  // Primitive kinds: Number, String, BigInt, Null, Undefined, Boolean, Symbol.
  EXPECT_TRUE(ctx.isPrimitive(Type::createNumber()));
  EXPECT_TRUE(ctx.isPrimitive(Type::createString()));
  EXPECT_TRUE(ctx.isPrimitive(Type::createBigInt()));
  EXPECT_TRUE(ctx.isPrimitive(Type::createNull()));
  EXPECT_TRUE(ctx.isPrimitive(Type::createUndefined()));
  EXPECT_TRUE(ctx.isPrimitive(Type::createBoolean()));
  EXPECT_TRUE(ctx.isPrimitive(Type::createSymbol()));

  // Object is NOT primitive.
  EXPECT_FALSE(ctx.isPrimitive(Type::createObject()));
  // Internal types are not primitive.
  EXPECT_FALSE(ctx.isPrimitive(Type::createEmpty()));
  EXPECT_FALSE(ctx.isPrimitive(Type::createUninit()));
  EXPECT_FALSE(ctx.isPrimitive(Type::createEnvironment()));
  EXPECT_FALSE(ctx.isPrimitive(asType(kBits32Id)));
  // NoType is not primitive.
  EXPECT_FALSE(ctx.isPrimitive(Type::createNoType()));

  // AnyType contains Object, so it's not all-primitive.
  EXPECT_FALSE(ctx.isPrimitive(Type::createAnyType()));
  // Numeric = Number | BigInt — both primitive.
  EXPECT_TRUE(ctx.isPrimitive(Type::createNumeric()));
  // NullOrUndef = Null | Undefined — both primitive.
  EXPECT_TRUE(ctx.isPrimitive(Type::createNullOrUndef()));
}

TEST_F(TypeContextTest, CanBePrimitive) {
  TypeContext ctx;

  EXPECT_TRUE(ctx.canBePrimitive(Type::createNumber()));
  EXPECT_FALSE(ctx.canBePrimitive(Type::createObject()));
  EXPECT_FALSE(ctx.canBePrimitive(Type::createNoType()));
  // AnyType contains primitives (and Object).
  EXPECT_TRUE(ctx.canBePrimitive(Type::createAnyType()));
  // AnyEmptyUninit contains primitives.
  EXPECT_TRUE(ctx.canBePrimitive(Type::createAnyEmptyUninit()));
}

TEST_F(TypeContextTest, IsNonPtr) {
  TypeContext ctx;

  // NonPtr kinds: Number, Boolean, Null, Undefined.
  EXPECT_TRUE(ctx.isNonPtr(Type::createNumber()));
  EXPECT_TRUE(ctx.isNonPtr(Type::createBoolean()));
  EXPECT_TRUE(ctx.isNonPtr(Type::createNull()));
  EXPECT_TRUE(ctx.isNonPtr(Type::createUndefined()));

  // String is a pointer type.
  EXPECT_FALSE(ctx.isNonPtr(Type::createString()));
  // Object is a pointer type.
  EXPECT_FALSE(ctx.isNonPtr(Type::createObject()));
  // NoType is not nonPtr.
  EXPECT_FALSE(ctx.isNonPtr(Type::createNoType()));

  // NullOrUndef = Null | Undefined — both non-ptr.
  EXPECT_TRUE(ctx.isNonPtr(Type::createNullOrUndef()));
  // AnyType contains String/Object, so not all non-ptr.
  EXPECT_FALSE(ctx.isNonPtr(Type::createAnyType()));
  // Numeric contains BigInt, which is a pointer type.
  EXPECT_FALSE(ctx.isNonPtr(Type::createNumeric()));
}

TEST_F(TypeContextTest, IsSubsetOf) {
  TypeContext ctx;

  // Reflexive.
  EXPECT_TRUE(ctx.isSubsetOf(Type::createNumber(), Type::createNumber()));
  // NoType is subset of everything.
  EXPECT_TRUE(ctx.isSubsetOf(Type::createNoType(), Type::createNumber()));
  EXPECT_TRUE(ctx.isSubsetOf(Type::createNoType(), Type::createAnyType()));
  EXPECT_TRUE(ctx.isSubsetOf(Type::createNoType(), Type::createNoType()));
  // Nothing (except NoType) is subset of NoType.
  EXPECT_FALSE(ctx.isSubsetOf(Type::createNumber(), Type::createNoType()));
  // Leaf is subset of union containing it.
  EXPECT_TRUE(ctx.isSubsetOf(Type::createNumber(), Type::createAnyType()));
  EXPECT_TRUE(ctx.isSubsetOf(Type::createNumber(), Type::createNumeric()));
  EXPECT_TRUE(ctx.isSubsetOf(Type::createBigInt(), Type::createNumeric()));
  // Union is not subset of its member.
  EXPECT_FALSE(ctx.isSubsetOf(Type::createAnyType(), Type::createNumber()));
  EXPECT_FALSE(ctx.isSubsetOf(Type::createNumeric(), Type::createNumber()));
  // Sub-union is subset of super-union.
  EXPECT_TRUE(ctx.isSubsetOf(Type::createNumeric(), Type::createAnyType()));
  EXPECT_TRUE(ctx.isSubsetOf(Type::createNullOrUndef(), Type::createAnyType()));
  // Disjoint types.
  EXPECT_FALSE(ctx.isSubsetOf(Type::createNumber(), Type::createString()));
  EXPECT_FALSE(ctx.isSubsetOf(Type::createString(), Type::createNumber()));
}

TEST_F(TypeContextTest, AreDisjoint) {
  TypeContext ctx;

  // Same type is not disjoint with itself.
  EXPECT_FALSE(ctx.areDisjoint(Type::createNumber(), Type::createNumber()));
  // NoType is disjoint from everything (including itself).
  EXPECT_TRUE(ctx.areDisjoint(Type::createNoType(), Type::createNumber()));
  EXPECT_TRUE(ctx.areDisjoint(Type::createNumber(), Type::createNoType()));
  EXPECT_TRUE(ctx.areDisjoint(Type::createNoType(), Type::createNoType()));
  // Different leaf types are disjoint.
  EXPECT_TRUE(ctx.areDisjoint(Type::createNumber(), Type::createString()));
  EXPECT_TRUE(ctx.areDisjoint(Type::createBoolean(), Type::createObject()));
  // Union overlaps with its members.
  EXPECT_FALSE(ctx.areDisjoint(Type::createNumber(), Type::createNumeric()));
  EXPECT_FALSE(ctx.areDisjoint(Type::createNumber(), Type::createAnyType()));
  // Disjoint unions.
  EXPECT_TRUE(
      ctx.areDisjoint(Type::createNullOrUndef(), Type::createNumeric()));
  // Overlapping unions.
  EXPECT_FALSE(ctx.areDisjoint(Type::createAnyType(), Type::createNumeric()));
}

TEST_F(TypeContextTest, UnionTyIdentity) {
  TypeContext ctx;

  EXPECT_EQ(
      ctx.unionTy(Type::createNumber(), Type::createNumber()),
      Type::createNumber());
  EXPECT_EQ(
      ctx.unionTy(Type::createAnyType(), Type::createAnyType()),
      Type::createAnyType());
}

TEST_F(TypeContextTest, UnionTyNoType) {
  TypeContext ctx;

  EXPECT_EQ(
      ctx.unionTy(Type::createNoType(), Type::createString()),
      Type::createString());
  EXPECT_EQ(
      ctx.unionTy(Type::createString(), Type::createNoType()),
      Type::createString());
  EXPECT_EQ(
      ctx.unionTy(Type::createNoType(), Type::createNoType()),
      Type::createNoType());
}

TEST_F(TypeContextTest, UnionTySubset) {
  TypeContext ctx;

  // Number is subset of AnyType.
  EXPECT_EQ(
      ctx.unionTy(Type::createNumber(), Type::createAnyType()),
      Type::createAnyType());
  EXPECT_EQ(
      ctx.unionTy(Type::createAnyType(), Type::createNumber()),
      Type::createAnyType());
  // Numeric is subset of AnyType.
  EXPECT_EQ(
      ctx.unionTy(Type::createNumeric(), Type::createAnyType()),
      Type::createAnyType());
}

TEST_F(TypeContextTest, UnionTyCreatesDynamic) {
  TypeContext ctx;

  Type numStr = ctx.unionTy(Type::createNumber(), Type::createString());
  EXPECT_EQ(ctx.getKind(numStr), TypeKind::Union);
  auto arms = ctx.getUnionArms(numStr);
  EXPECT_EQ(arms.size(), 2u);
  // Arms sorted by ID: String(6), Number(7).
  EXPECT_EQ(arms[0], Type::createString());
  EXPECT_EQ(arms[1], Type::createNumber());
}

TEST_F(TypeContextTest, UnionTyInterning) {
  TypeContext ctx;

  Type a = ctx.unionTy(Type::createNumber(), Type::createString());
  Type b = ctx.unionTy(Type::createNumber(), Type::createString());
  EXPECT_EQ(a, b);

  // Reverse order produces same result.
  Type c = ctx.unionTy(Type::createString(), Type::createNumber());
  EXPECT_EQ(a, c);
}

TEST_F(TypeContextTest, UnionTyReturnsWellKnown) {
  TypeContext ctx;

  // unionTy(Number, BigInt) should return the well-known Numeric.
  EXPECT_EQ(
      ctx.unionTy(Type::createNumber(), Type::createBigInt()),
      Type::createNumeric());
  // unionTy(Null, Undefined) should return well-known NullOrUndef.
  EXPECT_EQ(
      ctx.unionTy(Type::createNull(), Type::createUndefined()),
      Type::createNullOrUndef());
}

TEST_F(TypeContextTest, IntersectTy) {
  TypeContext ctx;

  // Disjoint types.
  EXPECT_EQ(
      ctx.intersectTy(Type::createNumber(), Type::createString()),
      Type::createNoType());
  // Subset: Number intersect AnyType = Number.
  EXPECT_EQ(
      ctx.intersectTy(Type::createNumber(), Type::createAnyType()),
      Type::createNumber());
  EXPECT_EQ(
      ctx.intersectTy(Type::createAnyType(), Type::createNumber()),
      Type::createNumber());
  // Same type.
  EXPECT_EQ(
      ctx.intersectTy(Type::createNumber(), Type::createNumber()),
      Type::createNumber());
  // NoType.
  EXPECT_EQ(
      ctx.intersectTy(Type::createNoType(), Type::createNumber()),
      Type::createNoType());
  EXPECT_EQ(
      ctx.intersectTy(Type::createNumber(), Type::createNoType()),
      Type::createNoType());
  // Union intersect Union: Numeric subset of AnyType.
  EXPECT_EQ(
      ctx.intersectTy(Type::createNumeric(), Type::createAnyType()),
      Type::createNumeric());
  // NullOrUndef intersect Numeric: disjoint → NoType.
  EXPECT_EQ(
      ctx.intersectTy(Type::createNullOrUndef(), Type::createNumeric()),
      Type::createNoType());
}

TEST_F(TypeContextTest, SubtractTy) {
  TypeContext ctx;

  // Subtract member from union: AnyType - Number.
  Type result = ctx.subtractTy(Type::createAnyType(), Type::createNumber());
  EXPECT_EQ(ctx.getKind(result), TypeKind::Union);
  auto arms = ctx.getUnionArms(result);
  EXPECT_EQ(arms.size(), 7u);
  EXPECT_FALSE(ctx.canBeNumber(result));
  EXPECT_TRUE(ctx.canBeString(result));
  EXPECT_TRUE(ctx.canBeObject(result));
  EXPECT_TRUE(ctx.canBeBigInt(result));

  // Subset subtraction → NoType.
  EXPECT_EQ(
      ctx.subtractTy(Type::createNumber(), Type::createAnyType()),
      Type::createNoType());
  // Disjoint subtraction → unchanged.
  EXPECT_EQ(
      ctx.subtractTy(Type::createNumber(), Type::createString()),
      Type::createNumber());
  // NoType cases.
  EXPECT_EQ(
      ctx.subtractTy(Type::createNoType(), Type::createNumber()),
      Type::createNoType());
  EXPECT_EQ(
      ctx.subtractTy(Type::createNumber(), Type::createNoType()),
      Type::createNumber());
  // Subtract union from union: AnyType - NullOrUndef.
  Type r2 = ctx.subtractTy(Type::createAnyType(), Type::createNullOrUndef());
  EXPECT_FALSE(ctx.canBeNull(r2));
  EXPECT_FALSE(ctx.canBeUndefined(r2));
  EXPECT_TRUE(ctx.canBeNumber(r2));
  EXPECT_TRUE(ctx.canBeString(r2));
}

TEST_F(TypeContextTest, UInt31WellKnownId) {
  TypeContext ctx;

  // UInt31 is pre-allocated with its own well-known ID.
  EXPECT_EQ(ctx.getKind(asType(kUInt31Id)), TypeKind::UInt31);
  EXPECT_EQ(ctx.getKind(asType(kInt32Id)), TypeKind::Int32);
  EXPECT_EQ(ctx.getKind(asType(kUint32Id)), TypeKind::Uint32);
}

TEST_F(TypeContextTest, UInt31SubtypeRelationships) {
  TypeContext ctx;

  // UInt31 <: Int32, Uint32, Number.
  EXPECT_TRUE(ctx.isSubsetOf(asType(kUInt31Id), asType(kInt32Id)));
  EXPECT_TRUE(ctx.isSubsetOf(asType(kUInt31Id), asType(kUint32Id)));
  EXPECT_TRUE(ctx.isSubsetOf(asType(kUInt31Id), Type::createNumber()));
  // Not the reverse.
  EXPECT_FALSE(ctx.isSubsetOf(asType(kInt32Id), asType(kUInt31Id)));
  EXPECT_FALSE(ctx.isSubsetOf(asType(kUint32Id), asType(kUInt31Id)));
  EXPECT_FALSE(ctx.isSubsetOf(Type::createNumber(), asType(kUInt31Id)));
  // Int32/Uint32 are not subsets of each other.
  EXPECT_FALSE(ctx.isSubsetOf(asType(kInt32Id), asType(kUint32Id)));
  EXPECT_FALSE(ctx.isSubsetOf(asType(kUint32Id), asType(kInt32Id)));
  // But both are subsets of Number.
  EXPECT_TRUE(ctx.isSubsetOf(asType(kInt32Id), Type::createNumber()));
  EXPECT_TRUE(ctx.isSubsetOf(asType(kUint32Id), Type::createNumber()));
}

TEST_F(TypeContextTest, UInt31Disjointness) {
  TypeContext ctx;

  // UInt31 is not disjoint from its supertypes.
  EXPECT_FALSE(ctx.areDisjoint(asType(kUInt31Id), asType(kInt32Id)));
  EXPECT_FALSE(ctx.areDisjoint(asType(kUInt31Id), asType(kUint32Id)));
  EXPECT_FALSE(ctx.areDisjoint(asType(kUInt31Id), Type::createNumber()));
  // Int32 and Uint32 overlap (via UInt31).
  EXPECT_FALSE(ctx.areDisjoint(asType(kInt32Id), asType(kUint32Id)));
  // UInt31 is disjoint from non-number types.
  EXPECT_TRUE(ctx.areDisjoint(asType(kUInt31Id), Type::createString()));
  EXPECT_TRUE(ctx.areDisjoint(asType(kUInt31Id), Type::createObject()));
  EXPECT_TRUE(ctx.areDisjoint(asType(kUInt31Id), Type::createBoolean()));
}

TEST_F(TypeContextTest, IntersectInt32Uint32) {
  TypeContext ctx;

  // Int32 ∩ Uint32 = UInt31.
  EXPECT_EQ(
      ctx.intersectTy(asType(kInt32Id), asType(kUint32Id)), asType(kUInt31Id));
  EXPECT_EQ(
      ctx.intersectTy(asType(kUint32Id), asType(kInt32Id)), asType(kUInt31Id));
  // Number ∩ Int32 = Int32 (subset).
  EXPECT_EQ(
      ctx.intersectTy(Type::createNumber(), asType(kInt32Id)),
      asType(kInt32Id));
  // Number ∩ Uint32 = Uint32 (subset).
  EXPECT_EQ(
      ctx.intersectTy(Type::createNumber(), asType(kUint32Id)),
      asType(kUint32Id));
  // UInt31 ∩ Int32 = UInt31 (subset).
  EXPECT_EQ(
      ctx.intersectTy(asType(kUInt31Id), asType(kInt32Id)), asType(kUInt31Id));
  // UInt31 ∩ String = NoType (disjoint).
  EXPECT_EQ(
      ctx.intersectTy(asType(kUInt31Id), Type::createString()),
      Type::createNoType());
}

TEST_F(TypeContextTest, IntersectUnionWithLeaf) {
  TypeContext ctx;

  Type numberOrString = ctx.unionTy(Type::createNumber(), Type::createString());
  EXPECT_EQ(
      ctx.intersectTy(numberOrString, asType(kInt32Id)), asType(kInt32Id));
  EXPECT_EQ(
      ctx.intersectTy(asType(kInt32Id), numberOrString), asType(kInt32Id));
}

TEST_F(TypeContextTest, IntersectUnionWithUnionDeduplicatesResults) {
  TypeContext ctx;

  Type intOrUint = ctx.unionTy(asType(kInt32Id), asType(kUint32Id));
  Type uint31OrString = ctx.unionTy(asType(kUInt31Id), Type::createString());
  EXPECT_EQ(ctx.intersectTy(intOrUint, uint31OrString), asType(kUInt31Id));
  EXPECT_EQ(ctx.intersectTy(uint31OrString, intOrUint), asType(kUInt31Id));
}

TEST_F(TypeContextTest, UInt31Predicates) {
  TypeContext ctx;

  EXPECT_TRUE(ctx.canBeNumber(asType(kUInt31Id)));
  EXPECT_TRUE(ctx.isPrimitive(asType(kUInt31Id)));
  EXPECT_TRUE(ctx.isNonPtr(asType(kUInt31Id)));
  EXPECT_FALSE(ctx.canBeString(asType(kUInt31Id)));
  EXPECT_FALSE(ctx.canBeObject(asType(kUInt31Id)));
}

TEST_F(TypeContextTest, CountKinds) {
  TypeContext ctx;

  EXPECT_EQ(ctx.countKinds(Type::createNoType()), 0u);
  EXPECT_EQ(ctx.countKinds(Type::createNumber()), 1u);
  EXPECT_EQ(ctx.countKinds(Type::createString()), 1u);
  EXPECT_EQ(ctx.countKinds(Type::createObject()), 1u);
  // AnyType has 8 arms.
  EXPECT_EQ(ctx.countKinds(Type::createAnyType()), 8u);
  // Numeric = Number | BigInt.
  EXPECT_EQ(ctx.countKinds(Type::createNumeric()), 2u);
  // NullOrUndef = Null | Undefined.
  EXPECT_EQ(ctx.countKinds(Type::createNullOrUndef()), 2u);
  // AnyEmptyUninit = 10 arms.
  EXPECT_EQ(ctx.countKinds(Type::createAnyEmptyUninit()), 10u);
  // Dynamic union.
  Type numStr = ctx.unionTy(Type::createNumber(), Type::createString());
  EXPECT_EQ(ctx.countKinds(numStr), 2u);
}

TEST_F(TypeContextTest, GetFirstKind) {
  TypeContext ctx;

  EXPECT_EQ(ctx.getFirstKind(Type::createNoType()), TypeKind::NoType);
  EXPECT_EQ(ctx.getFirstKind(Type::createNumber()), TypeKind::Number);
  EXPECT_EQ(ctx.getFirstKind(Type::createString()), TypeKind::String);
  EXPECT_EQ(ctx.getFirstKind(Type::createObject()), TypeKind::Object);
  EXPECT_EQ(ctx.getFirstKind(Type::createEmpty()), TypeKind::Empty);
  // AnyType first arm is Undefined (lowest ID arm).
  EXPECT_EQ(ctx.getFirstKind(Type::createAnyType()), TypeKind::Undefined);
  // Numeric first arm is Number (ID 7 < ID 8).
  EXPECT_EQ(ctx.getFirstKind(Type::createNumeric()), TypeKind::Number);
  // NullOrUndef first arm is Undefined (ID 3 < ID 4).
  EXPECT_EQ(ctx.getFirstKind(Type::createNullOrUndef()), TypeKind::Undefined);
}

TEST_F(TypeContextTest, PrintLeafTypes) {
  TypeContext ctx;
  auto fmt = [&](Type t) -> std::string {
    std::string s;
    llvh::raw_string_ostream os(s);
    ctx.print(os, t);
    return s;
  };

  EXPECT_EQ(fmt(Type::createNoType()), "notype");
  EXPECT_EQ(fmt(Type::createNumber()), "number");
  EXPECT_EQ(fmt(Type::createString()), "string");
  EXPECT_EQ(fmt(Type::createObject()), "object");
  EXPECT_EQ(fmt(Type::createBoolean()), "boolean");
  EXPECT_EQ(fmt(Type::createNull()), "null");
  EXPECT_EQ(fmt(Type::createUndefined()), "undefined");
  EXPECT_EQ(fmt(Type::createBigInt()), "bigint");
  EXPECT_EQ(fmt(Type::createSymbol()), "symbol");
  EXPECT_EQ(fmt(Type::createEmpty()), "empty");
  EXPECT_EQ(fmt(Type::createUninit()), "uninit");
  EXPECT_EQ(fmt(Type::createEnvironment()), "environment");
  EXPECT_EQ(fmt(Type::createPrivateName()), "privateName");
  EXPECT_EQ(fmt(Type::createFunctionCode()), "functionCode");
  EXPECT_EQ(fmt(asType(kBits32Id)), "bits32");
  EXPECT_EQ(fmt(asType(kInt32Id)), "int32");
  EXPECT_EQ(fmt(asType(kUint32Id)), "uint32");
  EXPECT_EQ(fmt(asType(kUInt31Id)), "uint31");
}

TEST_F(TypeContextTest, PrintWellKnownUnions) {
  TypeContext ctx;
  auto fmt = [&](Type t) -> std::string {
    std::string s;
    llvh::raw_string_ostream os(s);
    ctx.print(os, t);
    return s;
  };

  // AnyType prints as "any".
  EXPECT_EQ(fmt(Type::createAnyType()), "any");
  // AnyEmptyUninit prints as "any|empty|uninit".
  EXPECT_EQ(fmt(Type::createAnyEmptyUninit()), "any|empty|uninit");
  // Numeric = Number | BigInt.
  EXPECT_EQ(fmt(Type::createNumeric()), "number|bigint");
  // NullOrUndef = Undefined | Null (sorted by ID).
  EXPECT_EQ(fmt(Type::createNullOrUndef()), "undefined|null");
}

TEST_F(TypeContextTest, PrintDynamicUnion) {
  TypeContext ctx;
  auto fmt = [&](Type t) -> std::string {
    std::string s;
    llvh::raw_string_ostream os(s);
    ctx.print(os, t);
    return s;
  };

  Type numStr = ctx.unionTy(Type::createNumber(), Type::createString());
  // Arms sorted by ID: String(6), Number(7).
  EXPECT_EQ(fmt(numStr), "string|number");
}

TEST_F(TypeContextTest, PrintAnySupersetShowsExtraArms) {
  TypeContext ctx;
  TypeContextRAII guard(ctx);
  auto fmt = [&](Type t) -> std::string {
    std::string s;
    llvh::raw_string_ostream os(s);
    ctx.print(os, t);
    return s;
  };

  // any | environment should print "any|environment", not collapse to "any".
  Type anyEnv = ctx.unionTy(Type::createAnyType(), Type::createEnvironment());
  EXPECT_EQ(fmt(anyEnv), "any|environment");

  // AnyEmptyUninit is any | empty | uninit.
  EXPECT_EQ(fmt(asType(kAnyEmptyUninitId)), "any|empty|uninit");
}

TEST_F(TypeContextTest, ArmsLeaf) {
  TypeContext ctx;
  llvh::SmallVector<Type, 4> v;
  for (Type t : ctx.arms(Type::createNumber()))
    v.push_back(t);
  ASSERT_EQ(v.size(), 1u);
  EXPECT_EQ(v[0], Type::createNumber());
}

TEST_F(TypeContextTest, ArmsNoType) {
  TypeContext ctx;
  llvh::SmallVector<Type, 4> v;
  for (Type t : ctx.arms(Type::createNoType()))
    v.push_back(t);
  EXPECT_EQ(v.size(), 0u);
}

TEST_F(TypeContextTest, ArmsUnion) {
  TypeContext ctx;
  Type numStr = ctx.unionTy(Type::createNumber(), Type::createString());
  llvh::SmallVector<Type, 4> v;
  for (Type t : ctx.arms(numStr))
    v.push_back(t);
  ASSERT_EQ(v.size(), 2u);
  // Sorted by ID: String (6), Number (7).
  EXPECT_EQ(v[0], Type::createString());
  EXPECT_EQ(v[1], Type::createNumber());
}

TEST_F(TypeContextTest, CanBeAnyAndConveniences) {
  TypeContext ctx;
  EXPECT_TRUE(ctx.canBeAny(Type::createAnyType()));
  EXPECT_FALSE(ctx.canBeAny(Type::createNumber()));
  EXPECT_TRUE(ctx.canBeType(Type::createAnyType(), Type::createNumber()));
  EXPECT_FALSE(ctx.canBeType(Type::createNumber(), Type::createString()));
  EXPECT_TRUE(
      ctx.isProperSubsetOf(Type::createNumber(), Type::createAnyType()));
  EXPECT_FALSE(
      ctx.isProperSubsetOf(Type::createNumber(), Type::createNumber()));
  EXPECT_TRUE(ctx.isKnownPrimitiveType(Type::createNumber()));
  EXPECT_FALSE(ctx.isKnownPrimitiveType(Type::createAnyType()));
  EXPECT_FALSE(ctx.isKnownPrimitiveType(Type::createObject()));
}

TEST_F(TypeContextTest, CurrentWithGuard) {
  TypeContext ctx;
  TypeContextRAII guard(ctx);
  EXPECT_EQ(&TypeContext::current(), &ctx);
}

TEST_F(TypeContextTest, NestedGuards) {
  TypeContext outer;
  TypeContext inner;

  TypeContextRAII outerGuard(outer);
  EXPECT_EQ(&TypeContext::current(), &outer);

  {
    TypeContextRAII innerGuard(inner);
    EXPECT_EQ(&TypeContext::current(), &inner);
  }

  // Outer context restored after inner guard destruction.
  EXPECT_EQ(&TypeContext::current(), &outer);
}

TEST(TypeContextNoGuardTest, PrintWithoutGuardFallback) {
  // Type::print() should not crash when no TypeContextRAII guard is
  // installed. Instead it prints a fallback "type#<id>" string.
  std::string s;
  llvh::raw_string_ostream os(s);
  Type::createNumber().print(os);
  EXPECT_EQ(os.str(), "type#7");
}

} // anonymous namespace
