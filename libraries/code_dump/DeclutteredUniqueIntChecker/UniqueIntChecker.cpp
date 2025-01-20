#include "UniqueIntChecker.h"
namespace com{
    namespace elaeocarpus{
        namespace decluttered {

            int64_t UniqueIdChecker::getPower(int base, int exponent) {
                /* Recursion termination condition,
                 * Anything^0 = 1
                 */
                if (exponent == 0) {

                    return 1;
                }
                return base * getPower(base, exponent - 1);
            }            
        }
    }
}
