package org.car2x.veins.modules.application.traci;

import org.car2x.veins.modules.application.ieee80211p.DemoBaseApplLayer;

simple DDoSAttackApp extends DemoBaseApplLayer  
{
    parameters:
        @class(veins::DDoSAttackApp);
        bool isRSU = default(false);
        bool isAttacker = default(false);
        int targetNode = default(0);  // ID of the target vehicle
        double attackStartTime @unit(s) = default(80s);
        double attackStopTime @unit(s) = default(85s);
        double attackInterval @unit(s) = default(0.1s);
}