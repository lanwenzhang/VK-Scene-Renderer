#include <iostream>
#include "core/application.h"

int main(){

    std::shared_ptr<LZ::Core::Application> app = std::make_shared<LZ::Core::Application>();

    try {

        app->run();
    }
    catch(const std::exception& e) {

        std::cout << e.what() << std::endl;
    }

    return 0;
}