package org.car2x.veins.modules.application.traci;

import org.car2x.veins.modules.application.ieee80211p.DemoBaseApplLayer;

simple AmplificationAttack extends DemoBaseApplLayer
{
    parameters:
        @class(veins::AmplificationAttack);
        bool isRSU = default(false);          // Is this node an RSU?
        bool isAttacker = default(false);     // Is this RSU an attacker?
        int targetVehicle = default(-2);    
        double attackTime @unit(s) = default(90s);  // When to initiate the amplification attack
        int senderNode = default(-2);          // ID of the vehicle that sends the initial small request
}
