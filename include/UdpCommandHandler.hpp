// #############################################################################
// ##                                                                         ##
// ## UdpCommandHandler.hpp                    (c) Wolfram Plettscher 04/2026 ##
// ##                                                                         ##
// #############################################################################
#pragma once
#include "MSFSController.hpp"
#include <string>

class UdpCommandHandler {
public:
    UdpCommandHandler(MSFSController& controller);
    void handle(const std::string& packet);
private:
    MSFSController& controller;
};
