#include "../include/managers/CommunicationManager.h"
#include "../include/channels/EmailChannel.h"
#include "../../common/include/exceptions/ValidationException.h"

#include <memory>
#include <iostream>

using namespace sdrs::communication;
using namespace sdrs::exceptions;

void printResult(const std::string& testName, bool passed)
{
    std::cout << "[TEST] " << testName<< " : " << (passed ? "PASSED" : "FAILED")<< std::endl;
}

int main()
{
    try 
    {
        CommunicationManager manager;
        manager.registerChannel(std::make_shared<EmailChannel>());
        bool result = manager.sendMessage(1001, "Test communication service");
        printResult("Happy path", result);
    }
    catch (...) 
    {
        printResult("Happy path", false);
    }

    try 
    {
        CommunicationManager manager;
        manager.registerChannel(std::make_shared<EmailChannel>());
        manager.sendMessage(1002, "");
        printResult("Empty message validation", false);
    }
    catch (const ValidationException& ex) 
    {
        printResult("Empty message validation", true);
        std::cout << "  -> " << ex.what()<< " | field=" << ex.getField()<< std::endl;
    }

    try 
    {
        CommunicationManager manager;
        bool result = manager.sendMessage(1003, "No channel case");
        printResult("No channel registered", result == false);
    }
    catch (...) 
    {
        printResult("No channel registered", false);
    }

    try 
    {
        CommunicationManager manager;
        manager.registerChannel(nullptr);
        printResult("Register null channel", false);
    }
    catch (const ValidationException& ex) 
    {
        printResult("Register null channel", true);
        std::cout << "  -> " << ex.what() << " | field=" << ex.getField()  << std::endl;
    }

    try 
    {
        CommunicationManager manager;
        manager.registerChannel(std::make_shared<EmailChannel>());
        bool r1 = manager.sendMessage(2001, "Message 1");
        bool r2 = manager.sendMessage(2002, "Message 2");
        printResult("Multiple sends", r1 && r2);
    }
    catch (...) 
    {
        printResult("Multiple sends", false);
    }

    return 0;
}
