#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

namespace veins {

class DDoSAttackApp : public DemoBaseApplLayer {
public:
    DDoSAttackApp() : nodeId(-1), 
                      attackTimer(nullptr),
                      stopTimer(nullptr),
                      recoveryTimer(nullptr),
                      isRSU(false),
                      isAttacker(false),
                      isAttackActive(false),
                      messageCount(0),
                      attackImpact(0.0) {}
    
    virtual ~DDoSAttackApp() {
        cancelAndDelete(attackTimer);
        cancelAndDelete(stopTimer);
        cancelAndDelete(recoveryTimer);
    }

protected:
    virtual void initialize(int stage) override;
    virtual void handleSelfMsg(cMessage* msg) override;
    virtual void onWSM(BaseFrame1609_4* wsm) override;

private:
    void sendAttackMessage();
    void updateTargetVisuals();

    int nodeId;
    cMessage* attackTimer;
    cMessage* stopTimer;
    cMessage* recoveryTimer;
    
    bool isRSU;
    bool isAttacker;
    bool isAttackActive;
    
    int targetNode;
    int messageCount;
    double attackImpact;
    double attackStartTime;
    double attackStopTime;
    double attackInterval;
};

}