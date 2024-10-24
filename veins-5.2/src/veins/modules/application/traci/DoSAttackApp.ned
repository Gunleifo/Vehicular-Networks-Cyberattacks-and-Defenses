package org.car2x.veins.modules.application.traci;

import org.car2x.veins.modules.application.ieee80211p.DemoBaseApplLayer;

// Simple module for the DoSAttackApp
simple DoSAttackApp extends DemoBaseApplLayer
{
    parameters:
        @class(veins::DoSAttackApp); // C++ class implementing this module
        int attackerId = default(-1);     // ID of the attacker node, set in .ini file
        double attackStartTime @unit(s) = default(70s); // Time when the attack starts
        double attackStopTime @unit(s) = default(120s); // Time when attack stops
        double attackInterval @unit(s) = default(0.1s); // Interval between DoS messages
        string targetNodes = default("");  // List of target node IDs, space-separated
}