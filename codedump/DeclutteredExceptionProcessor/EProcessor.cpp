#include <EProcessor.h>

void processException()
{
    using namespace std;
    try {
        throw; // rethrow exception to deal with it here
    }
    catch (const ios_base::failure& e) {
        std::cerr << "I/O EXCEPTION: " << e.what() << std::endl;
        processCodeException(e);
    }
    catch (const system_error& e) {
        std::cerr << "SYSTEM EXCEPTION: " << e.what() << std::endl;
        processCodeException(e);
    }
    catch (const future_error& e) {
        std::cerr << "FUTURE EXCEPTION: " << e.what() << std::endl;
        processCodeException(e);
    }
    catch (const bad_alloc& e) {
        std::cerr << "BAD ALLOC EXCEPTION: " << e.what() << std::endl;
    }
    catch (const exception& e) {
        std::cerr << "EXCEPTION: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "EXCEPTION (unknown)" << std::endl;
    }
}

/*sample usage.*/
/*try {
    ...
}
catch (...) {
    processException();
}*/

