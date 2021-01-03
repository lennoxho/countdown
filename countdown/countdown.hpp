#pragma once
#include <charconv>
#include <cstdint>
#include <string_view>
#include <iostream>
#include <vector>

#include <gsl/gsl>

#define FORCE_DEBUG 0

#define CONTDOWN_ABORT(cond_str) { countdown::abort("Unexpected", cond_str, __FILE__, __LINE__); }

#define COUNTDOWN_TYPE_ASSERT(type, cond) \
    if (!GSL_UNLIKELY(cond)) { countdown::abort(type, #cond, __FILE__, __LINE__); }

#define COUNTDOWN_USER_ASSERT(cond) COUNTDOWN_TYPE_ASSERT("User input", cond)

#if defined(_DEBUG) || FORCE_DEBUG != 0
#define COUNTDOWN_ASSERT(cond) COUNTDOWN_TYPE_ASSERT("Assertion", cond)

#undef GSL_CONTRACT_CHECK
#define GSL_CONTRACT_CHECK(type, cond) COUNTDOWN_TYPE_ASSERT(type, cond)
#else
#define COUNTDOWN_ASSERT(cond)

#undef GSL_CONTRACT_CHECK
#define GSL_CONTRACT_CHECK(type, cond)
#endif

namespace countdown {

    using integer_type = std::int64_t;

    [[noreturn]] __declspec(noinline) void abort(std::string_view type, std::string_view cond_str, std::string_view file, int line) {
        std::cerr << type << " failure @" << file << ':' << line << '\n';
        if (!cond_str.empty()) {
            std::cerr << '\t' << cond_str << '\n';
        }

        std::abort();
    }

    enum class operation {
        add,
        sub,
        mul,
        div
    };

    namespace computation {

        struct addition { 
            static constexpr auto op = operation::add;
            inline integer_type operator()(integer_type lhs, integer_type rhs) const noexcept { return lhs + rhs; } 
        };
        struct subtraction { 
            static constexpr auto op = operation::sub;
            inline integer_type operator()(integer_type lhs, integer_type rhs) const noexcept { return lhs - rhs; } 
        };
        struct multiplication { 
            static constexpr auto op = operation::mul;
            inline integer_type operator()(integer_type lhs, integer_type rhs) const noexcept { return lhs * rhs; } 
        };
        struct division { 
            static constexpr auto op = operation::div;
            inline integer_type operator()(integer_type lhs, integer_type rhs) const noexcept { return lhs / rhs; } 
        };

    } // namespace computation

    class step {
        operation m_op;
        integer_type m_lhs;
        integer_type m_rhs;
        integer_type m_res;

    public:
        step(operation op, integer_type lhs, integer_type rhs, integer_type res) noexcept
            :m_op{ op },
            m_lhs{ lhs },
            m_rhs{ rhs },
            m_res{ res }
        {}

        friend std::ostream& operator<<(std::ostream &os, const step &s) {
            os << s.m_lhs << ' ';

            switch (s.m_op) {
            case operation::add: os << '+'; break;
            case operation::sub: os << '-'; break;
            case operation::mul: os << '*'; break;
            case operation::div: os << '/'; break;
            default:
                {
                    // 32 bit integer occupies at most 11 characters in base 10.
                    // +1 for sign
                    // +1 for nul character
                    // = 13 bytes.
                    char buf[13];
                    auto[p, e] = std::to_chars(buf, buf + sizeof(buf), static_cast<int>(s.m_op));
                    if (e == std::errc{}) {
                        *p = '\0';
                        CONTDOWN_ABORT(buf);
                    }
                    else {
                        CONTDOWN_ABORT("<invalid op>");
                    }
                }
            }

            os << ' ' << s.m_rhs
               << " = " << s.m_res;

            return os;
        }
    };

    using steps = std::vector<step>;

} // namespace countdown