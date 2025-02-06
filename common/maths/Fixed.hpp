#pragma once

#include <common/maths.hpp>

namespace maths {

	template <typename Type, unsigned divisor>
	struct Fixed {
	protected:
		constexpr /**/ explicit Fixed(Type value): value(value) {};

	public:
		constexpr /**/ Fixed(const Fixed<Type, divisor> &copy): value(copy.value) {};
		
		static constexpr auto whole(Type a) { return Fixed(a*divisor); }
		static constexpr auto fraction(Type a) { return Fixed(a); }

		Type value;

		template <typename CastType> auto cast() { return Fixed<CastType, divisor>::fraction(value); }

		auto round() const -> Type { return (value+((Type)divisor-1)*maths::sign(value))/(Type)divisor; }
		auto round_zero() const -> Type { return value/(Type)divisor; }
		auto round_down() const -> Type { return (value-(value>=0?0:(Type)divisor-1))/(Type)divisor; }
		auto round_up() const -> Type { return (value+(value>=0?(Type)divisor-1:0))/(Type)divisor; }

		auto negate() const { return Fixed(-value); }

		auto fract() const { return Fixed(value-round_zero()*(Type)divisor); }

		template <typename OpType> auto multiply(OpType op) const { return Fixed<decltype(value*op), divisor>::fraction(value*op); }
		template <typename OpType> auto divide  (OpType op) const { return Fixed<decltype(value/op), divisor>::fraction(value/op); }
		template <typename OpType> auto add     (OpType op) const { return Fixed<decltype(value+op), divisor>::fraction(value+op*(Type)divisor); }
		template <typename OpType> auto subtract(OpType op) const { return Fixed<decltype(value-op), divisor>::fraction(value-op*(Type)divisor); }

		template <typename OpType> auto multiply(Fixed<OpType, divisor> op) const { return Fixed<decltype(value*op.value), divisor>::fraction(value*op.value/(Type)divisor); }
		template <typename OpType> auto divide  (Fixed<OpType, divisor> op) const { return Fixed<decltype(value/op.value), divisor>::fraction(value*(Type)divisor/op.value); }
		template <typename OpType> auto add     (Fixed<OpType, divisor> op) const { return Fixed<decltype(value+op.value), divisor>::fraction(value+op.value); }
		template <typename OpType> auto subtract(Fixed<OpType, divisor> op) const { return Fixed<decltype(value-op.value), divisor>::fraction(value-op.value); }

		static auto divide(Type a, Type b) { return Fixed(a*(Type)divisor/b); }

		// operator Type() const { return round(); }
		auto operator-() const { return negate(); }

		auto operator+=(Fixed op) -> Fixed& { return *this = add(op); }
		auto operator-=(Fixed op) -> Fixed& { return *this = subtract(op); }
		auto operator*=(Fixed op) -> Fixed& { return *this = multiply(op); }
		auto operator/=(Fixed op) -> Fixed& { return *this = divide(op); }

		auto operator+=(Type op) -> Fixed& { return *this = add(op); }
		auto operator-=(Type op) -> Fixed& { return *this = subtract(op); }
		auto operator*=(Type op) -> Fixed& { return *this = multiply(op); }
		auto operator/=(Type op) -> Fixed& { return *this = divide(op); }

		bool operator==(Fixed op) const { return value==op.value; }
		bool operator!=(Fixed op) const { return value!=op.value; }
		bool operator> (Fixed op) const { return value> op.value; }
		bool operator>=(Fixed op) const { return value>=op.value; }
		bool operator< (Fixed op) const { return value< op.value; }
		bool operator<=(Fixed op) const { return value<=op.value; }
	};

	template <typename Type1, typename Type2, unsigned divisor> auto operator+(Fixed<Type1, divisor> op1, Fixed<Type2, divisor> op2){ return op1.add(op2); }
	template <typename Type1, typename Type2, unsigned divisor> auto operator-(Fixed<Type1, divisor> op1, Fixed<Type2, divisor> op2){ return op1.subtract(op2); }
	template <typename Type1, typename Type2, unsigned divisor> auto operator*(Fixed<Type1, divisor> op1, Fixed<Type2, divisor> op2){ return op1.multiply(op2); }
	template <typename Type1, typename Type2, unsigned divisor> auto operator/(Fixed<Type1, divisor> op1, Fixed<Type2, divisor> op2){ return op1.divide(op2); }

	template <typename Type1, typename Type2, unsigned divisor> auto operator+(Fixed<Type1, divisor> op1, Type2 op2){ return op1.add(op2); }
	template <typename Type1, typename Type2, unsigned divisor> auto operator-(Fixed<Type1, divisor> op1, Type2 op2){ return op1.subtract(op2); }
	template <typename Type1, typename Type2, unsigned divisor> auto operator*(Fixed<Type1, divisor> op1, Type2 op2){ return op1.multiply(op2); }
	template <typename Type1, typename Type2, unsigned divisor> auto operator/(Fixed<Type1, divisor> op1, Type2 op2){ return op1.divide(op2); }

	template <typename Type, unsigned divisor> auto operator+(Type op1, Fixed<Type, divisor> op2){ return Fixed<Type, divisor>::whole(op1).add(op2); }
	template <typename Type, unsigned divisor> auto operator-(Type op1, Fixed<Type, divisor> op2){ return Fixed<Type, divisor>::whole(op1).subtract(op2); }
	template <typename Type, unsigned divisor> auto operator*(Type op1, Fixed<Type, divisor> op2){ return Fixed<Type, divisor>::whole(op1).multiply(op2); }
	template <typename Type, unsigned divisor> auto operator/(Type op1, Fixed<Type, divisor> op2){ return Fixed<Type, divisor>::whole(op1).divide(op2); }
}

typedef maths::Fixed<I16, 256> FixedI16;
typedef maths::Fixed<I32, 256> FixedI32;
typedef maths::Fixed<I64, 256> FixedI64;

typedef maths::Fixed<U16, 256> FixedU16;
typedef maths::Fixed<U32, 256> FixedU32;
typedef maths::Fixed<U64, 256> FixedU64;

typedef maths::Fixed<I16Fast, 256> FastFixedI16;
typedef maths::Fixed<I32Fast, 256> FastFixedI32;
typedef maths::Fixed<I64Fast, 256> FastFixedI64;

typedef maths::Fixed<U16Fast, 256> FastFixedU16;
typedef maths::Fixed<U32Fast, 256> FastFixedU32;
typedef maths::Fixed<U64Fast, 256> FastFixedU64;
