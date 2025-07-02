#include <iostream>
#include "core/application.h"

int main(){

    std::shared_ptr<lzvk::core::Application> app = std::make_shared<lzvk::core::Application>();

    try {

        app->run();
    }
    catch(const std::exception& e) {

        std::cout << e.what() << std::endl;
    }

    return 0;
}