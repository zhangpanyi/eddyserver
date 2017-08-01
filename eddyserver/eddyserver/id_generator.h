#ifndef __ID_GENERATOR_H__
#define __ID_GENERATOR_H__

#include <limits>
#include <cassert>
#include <unordered_set>

namespace eddyserver
{
    template <typename T>
    class IDGenerator final
    {
        static_assert(std::numeric_limits<T>::is_integer, "");

    public:
        IDGenerator(T min_id = (std::numeric_limits<T>::min()), T max_id = (std::numeric_limits<T>::max()), size_t threshold = 4096)
            : min_(min_id)
            , max_(max_id)
            , next_(min_id)
            , threshold_(threshold)
        {
            assert(min_id <= max_id);
        }

        bool get(T &result)
        {
            if (pools_.size() >= threshold_)
            {
                result = *pools_.begin();
                pools_.erase(pools_.begin());
                return true;
            }

            if (next_ <= max_)
            {
                result = next_;
                ++next_;
                return true;
            }
            return false;
        }

        void put(T id)
        {
            auto found = pools_.find(id);
            assert(found == pools_.end());
            if (found == pools_.end())
            {
                pools_.insert(id);
            }
        }

    private:
        const T                 min_;
        const T                 max_;
        T                       next_;
        const size_t            threshold_;
        std::unordered_set<T>   pools_;
    };
}

#endif
