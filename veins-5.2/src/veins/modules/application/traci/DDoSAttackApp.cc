#include "veins/modules/application/traci/DDoSAttackApp.h"
#include <algorithm>
#include <sstream>

using namespace veins;

Define_Module(DDoSAttackApp);

void DDoSAttackApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    
    if (stage == 0) {
        nodeId = getParentModule()->getIndex();
        isRSU = par("isRSU").boolValue();
        isAttacker = isRSU && par("isAttacker").boolValue();
        targetNode = par("targetNode").intValue();
        attackStartTime = par("attackStartTime").doubleValue();
        attackStopTime = par("attackStopTime").doubleValue();
        attackInterval = par("attackInterval").doubleValue();
        
        // Initialize attack variables
        messageCount = 0;
        attackImpact = 0;
        isAttackActive = false;
        
        if (isRSU && isAttacker) {
            // Configure attacker RSU appearance
            findHost()->getDisplayString().setTagArg("i", 1, "red");
            findHost()->getDisplayString().setTagArg("is", 0, "2");
            
            // Create attack timers
            attackTimer = new cMessage("attackTimer");
            stopTimer = new cMessage("stopTimer");
            
            // Schedule attack
            scheduleAt(simTime() + attackStartTime, attackTimer);
            scheduleAt(simTime() + attackStopTime, stopTimer);
            
            EV_INFO << "DDoS Attacker RSU " << nodeId << " initialized, targeting vehicle " 
                    << targetNode << endl;
        }
        else if (!isRSU) {
            // Vehicle specific initialization
            recoveryTimer = new cMessage("recoveryTimer");
            
            if (nodeId == targetNode) {
                EV_INFO << "Target vehicle " << nodeId << " initialized" << endl;
                // Make target vehicle visually distinct
                findHost()->getDisplayString().setTagArg("i", 1, "blue");
            }
        }
    }
}

void DDoSAttackApp::handleSelfMsg(cMessage* msg)
{
    if (msg == attackTimer) {
        isAttackActive = true;
        // Send attack message
        sendAttackMessage();
        // Schedule next attack message
        scheduleAt(simTime() + attackInterval, attackTimer);
    }
    else if (msg == stopTimer) {
        isAttackActive = false;
        // Cancel any pending attack messages
        if (attackTimer->isScheduled()) {
            cancelEvent(attackTimer);
        }
        EV_INFO << "RSU " << nodeId << " stopped DDoS attack. Messages sent: " 
                << messageCount << endl;
    }
    else if (msg == recoveryTimer && nodeId == targetNode) {
        // Gradual recovery for target vehicle
        if (attackImpact > 0) {
            attackImpact = std::max(0.0, attackImpact - 0.05);
            updateTargetVisuals();
            scheduleAt(simTime() + 1, recoveryTimer);
        }
    }
}

void DDoSAttackApp::sendAttackMessage()
{
    TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
    populateWSM(wsm);
    wsm->setDemoData("DDoS_ATTACK");
    sendDown(wsm);
    messageCount++;
    
    // Log every 10th message to reduce spam
    if (messageCount % 10 == 0) {
        EV_INFO << "RSU " << nodeId << " sent attack message " << messageCount 
                << " at " << simTime() << endl;
    }
}

void DDoSAttackApp::updateTargetVisuals()
{
    // Update visual representation based on attack impact
    if (attackImpact > 0.8) {
        findHost()->getDisplayString().setTagArg("i", 1, "red");
        findHost()->getDisplayString().setTagArg("is", 0, "2.0");
    } 
    else if (attackImpact > 0.5) {
        findHost()->getDisplayString().setTagArg("i", 1, "orange");
        findHost()->getDisplayString().setTagArg("is", 0, "1.7");
    } 
    else if (attackImpact > 0.2) {
        findHost()->getDisplayString().setTagArg("i", 1, "yellow");
        findHost()->getDisplayString().setTagArg("is", 0, "1.4");
    }
    else {
        findHost()->getDisplayString().setTagArg("i", 1, "blue");
        findHost()->getDisplayString().setTagArg("is", 0, "1");
    }
    
    if (traciVehicle) {
        // Reduce speed based on impact
        double maxSpeed = traciVehicle->getMaxSpeed();
        double targetSpeed = maxSpeed * (1.0 - attackImpact);
        traciVehicle->setSpeed(targetSpeed);
        
        // Change vehicle color
        if (attackImpact > 0.5) {
            traciVehicle->setColor(TraCIColor(255, 0, 0, 255));  // Red
        } else if (attackImpact > 0.2) {
            traciVehicle->setColor(TraCIColor(255, 165, 0, 255));  // Orange
        } else {
            traciVehicle->setColor(TraCIColor(0, 0, 255, 255));  // Blue
        }
    }
}

void DDoSAttackApp::onWSM(BaseFrame1609_4* frame)
{
    if (!isRSU && nodeId == targetNode) {
        TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);
        
        if (std::string(wsm->getDemoData()) == "DDoS_ATTACK") {
            // Increase attack impact based on message frequency
            attackImpact = std::min(1.0, attackImpact + 0.1);
            updateTargetVisuals();
            
            // Start/reset recovery timer
            if (!recoveryTimer->isScheduled()) {
                scheduleAt(simTime() + 1, recoveryTimer);
            }
            
            // Log every 5th received message
            if (static_cast<int>(attackImpact * 10) % 5 == 0) {
                EV_INFO << "Target vehicle " << nodeId << " impact level: " 
                        << attackImpact << " at " << simTime() << endl;
            }
        }
    }
}