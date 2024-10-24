package org.car2x.veins.modules.application.traci;

import org.car2x.veins.modules.application.ieee80211p.DemoBaseApplLayer;

// Simple module for the SpoofingAttackApp
simple SpoofingAttackApp extends DemoBaseApplLayer
{
    parameters:
        @class(veins::SpoofingAttackApp); // C++ class implementing this module
        int attackerId = default(-1);     // ID of the attacker node, set in .ini file
        double attackStartTime @unit(s) = default(70s); // Time when the attack starts
}