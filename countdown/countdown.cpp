#include <optional>

#include "countdown.hpp"

namespace countdown {

    namespace detail {

        template <typename Computation>
        std::optional<steps> try_solve_for_indices(std::size_t first_idx, std::size_t second_idx,
                                                   integer_type target, gsl::span<integer_type> working_set);

        std::optional<steps> solve_impl(integer_type target, gsl::span<integer_type> working_set) {
            if (working_set.size() >= 2) {
                // Generate all index pairs (combinations). All nC2 of them
                const auto size = working_set.size();
                for (std::size_t i = 0; i + 1 < size; ++i) {
                    for (std::size_t j = i + 1; j < size; ++j) {
                        if (auto opt = try_solve_for_indices<computation::addition>(i, j, target, working_set)) {
                            return opt;
                        }
                        if (auto opt = try_solve_for_indices<computation::subtraction>(i, j, target, working_set)) {
                            return opt;
                        }
                        if (auto opt = try_solve_for_indices<computation::multiplication>(i, j, target, working_set)) {
                            return opt;
                        }
                        if (auto opt = try_solve_for_indices<computation::division>(i, j, target, working_set)) {
                            return opt;
                        }
                    }
                }
            }

            return std::nullopt;
        }

        template <typename Computation>
        std::optional<steps> try_solve_for_indices(std::size_t first_idx, std::size_t second_idx,
            integer_type target, gsl::span<integer_type> working_set)
        {
            COUNTDOWN_ASSERT(working_set.size() >= 2);
            COUNTDOWN_ASSERT(first_idx < second_idx);
            const auto first = working_set[first_idx];
            const auto second = working_set[second_idx];

            // Switcheroo trick: use first_idx for curent result; second_idx for last
            working_set[second_idx] = working_set.back();
            auto new_working_set = working_set.subspan(0, working_set.size() - 1);

            // first op second
            if (second == 0 && Computation::op == operation::div);
            else {
                const auto result = Computation()(first, second);
                if (result == target) {
                    return steps{ { Computation::op, first, second, target } };
                }
                else {
                    working_set[first_idx] = result;
                    if (auto opt = solve_impl(target, new_working_set)) {
                        opt->emplace_back(Computation::op, first, second, result);
                        return opt;
                    }
                }
            }
            
            // second op first
            // Redundant for certain operations
            if constexpr (Computation::op != operation::add && Computation::op != operation::mul) {
                if (first == 0 && Computation::op == operation::div);
                else {
                    const auto result = Computation()(second, first);
                    if (result == target) {
                        return steps{ { Computation::op, second, first, target } };
                    }
                    else {
                        working_set[first_idx] = result;
                        if (auto opt = solve_impl(target, new_working_set)) {
                            opt->emplace_back(Computation::op, second, first, result);
                            return opt;
                        }
                    }
                }
            }

            // Restore positions
            working_set[first_idx] = first;
            working_set[second_idx] = second;

            return std::nullopt;
        }

    } // namespace detail

    std::optional<steps> solve(integer_type target, gsl::span<integer_type> working_set) {
        if (auto opt = detail::solve_impl(target, working_set)) {
            auto &steps = *opt;

            // Steps are returned in reverse
            std::reverse(steps.begin(), steps.end());
            return opt;
        }

        return std::nullopt;
    }

} // namespace countdown

int main(int argc, const char** argv) {
    COUNTDOWN_USER_ASSERT(argc >= 4);
    auto args = gsl::make_span(argv + 1, argc - 1);
    
    // Parse target
    countdown::integer_type target;
    {
        std::string_view target_str{ args.front() };
        auto[p, e] = std::from_chars(target_str.data(), target_str.data() + target_str.size(), target);
        COUNTDOWN_USER_ASSERT(e == std::errc{});
        COUNTDOWN_USER_ASSERT((p - target_str.data()) == target_str.size());
    }
    
    // Parse numbers
    std::vector<countdown::integer_type> numbers;
    numbers.reserve(args.size() - 1);

    for (auto arg : args.subspan(1)) {
        std::string_view number_str{ arg };
        countdown::integer_type number;

        auto[p, e] = std::from_chars(number_str.data(), number_str.data() + number_str.size(), number);
        COUNTDOWN_USER_ASSERT(e == std::errc{});
        COUNTDOWN_USER_ASSERT((p - number_str.data()) == number_str.size());

        numbers.emplace_back(number);
    }

    // Solve and print solution
    if (auto opt = countdown::solve(target, numbers)) {
        for (const auto &step : *opt) {
            std::cout << step << '\n';
        }
    }
    else {
        std::cout << "No solution found :(\n";
    }
}