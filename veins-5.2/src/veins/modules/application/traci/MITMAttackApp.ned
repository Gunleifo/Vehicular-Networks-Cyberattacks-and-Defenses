package org.car2x.veins.modules.application.traci;

import org.car2x.veins.modules.application.ieee80211p.DemoBaseApplLayer;

simple MITMAttackApp extends DemoBaseApplLayer
{
    parameters:
        @class(veins::MITMAttackApp);
        bool isRSU = default(false);          // Is this node an RSU?
        bool isAttacker = default(false);     // Is this RSU an attacker?
        double attackTime @unit(s) = default(80s);  // When to send the fake message
        int senderNode = default(0);          // ID of the vehicle that sends the initial message
}