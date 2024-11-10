#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
#include <set>

namespace veins {

class VEINS_API DoSAttackApp : public DemoBaseApplLayer {
public:
    DoSAttackApp() : 
        nodeId(-1),
        pingTimer(nullptr),
        monitorTimer(nullptr),
        isAttacker(false),
        attackerId(-1),
        pingInterval(0.0001),    
        burstSize(10),          
        attackStartTime(0),
        attackStopTime(0),
        lastStatsTime(0),
        pingsSent(0),
        pingsReceived(0),
        responsesSent(0),
        responsesDropped(0),
        packetSentSignal(-1),
        packetReceivedSignal(-1) {}
    
    virtual ~DoSAttackApp() {
        cancelAndDelete(pingTimer);
        cancelAndDelete(monitorTimer);
    }

protected:
    virtual void initialize(int stage) override;
    virtual void handleSelfMsg(cMessage* msg) override;
    virtual void onWSM(BaseFrame1609_4* wsm) override;
    virtual void finish() override;

private:
    void sendPingBurst();
    void sendPingResponse(int targetId);
    void monitorQueueStatus();
    bool isTargetNode(int nodeId) const;

    // Basic parameters
    int nodeId;
    cMessage* pingTimer;
    cMessage* monitorTimer;
    bool isAttacker;
    int attackerId;
    
    // Attack parameters
    double pingInterval;
    int burstSize;
    simtime_t attackStartTime;
    simtime_t attackStopTime;
    std::set<int> targetNodes;
    
    // Statistics and monitoring
    double lastStatsTime;
    int pingsSent;
    int pingsReceived;
    int responsesSent;
    int responsesDropped;
    
    // Signals for statistics
    simsignal_t packetSentSignal;
    simsignal_t packetReceivedSignal;
};
}