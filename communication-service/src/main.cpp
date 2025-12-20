#include "../include/managers/CommunicationManager.h"
#include "../include/channels/EmailChannel.h"
#include "../../common/include/exceptions/ValidationException.h"

#include <memory>
#include <iostream>

using namespace sdrs::communication;
using namespace sdrs::exceptions;

int main()
{
    try 
    {
        CommunicationManager manager;
        manager.registerChannel(std::make_shared<EmailChannel>());
        manager.sendMessage(42, "Hello from communication-service");
        std::cout << "[MAIN] Message sent successfully" << std::endl;
    }
    catch (const ValidationException& ex) 
    {
        std::cerr << "[MAIN][ValidationException] "<< ex.what()<< " | field=" << ex.getField()<< std::endl;
    }
    catch (const std::exception& ex) 
    {
        std::cerr << "[MAIN][Exception] " << ex.what() << std::endl;
    }
    return 0;
}
