#pragma once
#include <assert.h>
namespace fb {
	struct CounterFromZero {
		CounterFromZero()
			: c(0) {}

		CounterFromZero(unsigned sc)
			:c(sc) {}

		CounterFromZero& operator++() {
			++c;
			return *this;
		}

		CounterFromZero operator++(int unused) {
			return c++;
		}

		CounterFromZero& operator += (unsigned num) {
			c += num;
			return *this;
		}

		CounterFromZero& operator -= (unsigned num) {
			c -= num;
			if (c == (unsigned)-1) {
				assert(0 && "Invalid count");
			}
			return *this;
		}

		operator unsigned() const {
			return c;
		}

		unsigned c;
	};
}
