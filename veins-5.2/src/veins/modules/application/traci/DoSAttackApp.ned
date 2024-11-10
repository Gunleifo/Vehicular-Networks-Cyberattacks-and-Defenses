package org.car2x.veins.modules.application.traci;

import org.car2x.veins.modules.application.ieee80211p.DemoBaseApplLayer;

simple DoSAttackApp extends DemoBaseApplLayer
{
    parameters:
        @class(veins::DoSAttackApp);
        
        // Attack configuration
        int attackerId = default(-1);  // ID of the attacking node
        double attackStartTime @unit(s) = default(80s);  // When the attack starts
        double attackStopTime @unit(s) = default(85s);   // When the attack ends
        double attackInterval @unit(s) = default(0.001s); // Interval between pings
        string targetNodes = default("");  // Space-separated list of target node IDs
        
        // Statistics signals
        @signal[packetSent](type="long");
        @signal[packetReceived](type="long");
        
        @statistic[packetsReceived](title="Packets received"; source="packetReceived"; record=count,vector; interpolationmode=none);
        @statistic[packetsSent](title="Packets sent"; source="packetSent"; record=count,vector; interpolationmode=none);

        @display("i=block/network2");
}