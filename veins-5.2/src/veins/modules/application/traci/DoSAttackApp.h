#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"
#include <set>

namespace veins {

class DoSAttackApp : public DemoBaseApplLayer {
public:
    DoSAttackApp() : nodeId(-1), 
                     attackTimer(nullptr), 
                     stopTimer(nullptr),
                     recoveryTimer(nullptr), 
                     isAttacker(false), 
                     attackerId(-1), 
                     attackInterval(0.1),
                     recoveryInterval(0.5),
                     attackStartTime(0),
                     attackStopTime(0),
                     isAttackActive(false),
                     attackImpact(0.0) {}
    
    virtual ~DoSAttackApp() {
        cancelAndDelete(attackTimer);
        cancelAndDelete(stopTimer);
        cancelAndDelete(recoveryTimer);
    }

protected:
    virtual void initialize(int stage) override;
    virtual void handleSelfMsg(cMessage* msg) override;
    virtual void onWSM(BaseFrame1609_4* wsm) override;

private:
    void sendDoSMessage();
    void updateTargetVisuals();
    bool isTargetNode(int nodeId) const;

    int nodeId;
    cMessage* attackTimer;
    cMessage* stopTimer;
    cMessage* recoveryTimer;
    bool isAttacker;
    int attackerId;
    double attackInterval;
    double recoveryInterval;
    
    simtime_t attackStartTime;
    simtime_t attackStopTime;
    
    bool isAttackActive;
    std::set<int> targetNodes;
    
    simtime_t lastAttackTime;
    double attackImpact;
};

}