#pragma once

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/TraCIDemo11pMessage_m.h"

namespace veins {

// SpoofingAttackApp class: Implements a spoofing attack in a VANET scenario
class SpoofingAttackApp : public DemoBaseApplLayer {
public:
    // Constructor: Initialize member variables
    SpoofingAttackApp() : nodeId(-1), attackTimer(nullptr), isAttacker(false), attackerId(-1) {}
    
    // Destructor: Clean up dynamically allocated memory
    virtual ~SpoofingAttackApp() { cancelAndDelete(attackTimer); }

protected:
    // Override the initialize function from DemoBaseApplLayer
    virtual void initialize(int stage) override;
    
    // Override the handleSelfMsg function to handle self-messages (e.g., timers)
    virtual void handleSelfMsg(cMessage* msg) override;
    
    // Override the onWSM function to handle incoming WAVE Short Messages
    virtual void onWSM(BaseFrame1609_4* wsm) override;

private:
    // Function to send a fake accident message (used by the attacker)
    void sendFakeAccidentMessage();

    int nodeId;           // Unique identifier for this node
    cMessage* attackTimer; // Timer to trigger the spoofing attack
    bool isAttacker;      // Flag to indicate if this node is the attacker
    int attackerId;       // ID of the attacker node, set from .ini file
};

} 