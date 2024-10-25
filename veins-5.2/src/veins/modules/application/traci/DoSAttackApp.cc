#include "veins/modules/application/traci/DoSAttackApp.h"
#include <algorithm>
#include <sstream>

using namespace veins;

Define_Module(DoSAttackApp);

void DoSAttackApp::initialize(int stage)
{
    // Initialize parent class first
    DemoBaseApplLayer::initialize(stage);
    
    // Stage 0 is used for basic initialization
    if (stage == 0) {
        // Get node's unique identifier from its parent module
        nodeId = getParentModule()->getIndex();
        
        // Read attack configuration from omnetpp.ini
        attackerId = par("attackerId").intValue();
        isAttacker = (nodeId == attackerId);
        attackInterval = par("attackInterval").doubleValue();
        
        // Get attack start and stop times from parameters
        attackStartTime = par("attackStartTime").doubleValue();
        attackStopTime = par("attackStopTime").doubleValue();
        
        // Parse the space-separated list of target node IDs
        std::string targetNodesStr = par("targetNodes").stdstringValue();
        std::istringstream iss(targetNodesStr);
        int target;
        while (iss >> target) {
            targetNodes.insert(target);
        }
        
        // Initialize variables for tracking attack impact
        lastAttackTime = simTime();
        attackImpact = 0;
        isAttackActive = false;
        
        if (isAttacker) {
            // Setup attacker node visual appearance
            findHost()->getDisplayString().setTagArg("i", 1, "red");
            findHost()->getDisplayString().setTagArg("is", 0, "2");  // Double size
            
            // Create timers for attack control
            attackTimer = new cMessage("attackTimer");
            stopTimer = new cMessage("stopTimer");
            
            // Schedule attack start and stop times
            scheduleAt(attackStartTime, attackTimer);
            scheduleAt(attackStopTime, stopTimer);
            
            EV_INFO << "DoSAttackApp initialized for Node " << nodeId 
                    << " (Attacker), attack scheduled at " << attackStartTime
                    << ", will stop at " << attackStopTime
                    << ", targeting nodes: " << targetNodesStr << endl;
        } else {
            // Setup regular node appearance
            findHost()->getDisplayString().setTagArg("i", 1, "green");
            
            // Create timer for recovery process
            recoveryTimer = new cMessage("recoveryTimer");
            
            EV_INFO << "DoSAttackApp initialized for Node " << nodeId << " (Regular node)" << endl;
        }
    }
}

void DoSAttackApp::handleSelfMsg(cMessage* msg)
{
    if (msg == attackTimer) {
        // Check if we've reached the stop time
        if (simTime() < attackStopTime) {
            // Start or continue attack
            if (!isAttackActive) {
                isAttackActive = true;
                EV_INFO << "Node " << nodeId << " started DoS attack at " << simTime() 
                       << " (scheduled start: " << attackStartTime << ")" << endl;
            }
            
            sendDoSMessage();
            // Schedule next attack message
            scheduleAt(simTime() + attackInterval, attackTimer);
        }
    }
    else if (msg == stopTimer) {
        // Handle attack stop
        isAttackActive = false;
        if (attackTimer->isScheduled()) {
            cancelEvent(attackTimer);
        }
        
        // Reset attacker appearance
        findHost()->getDisplayString().setTagArg("is", 0, "1");
        findHost()->getDisplayString().setTagArg("i", 1, "green");
        
        EV_INFO << "Node " << nodeId << " stopped DoS attack at " << simTime() 
               << " (scheduled stop: " << attackStopTime << ")" << endl;
    }
    else if (msg == recoveryTimer) {
        if (attackImpact > 0) {
            // Calculate recovery rate based on current impact level
            double recoveryRate;
            if (attackImpact > 0.8) {
                recoveryRate = 0.01; 
            } else if (attackImpact > 0.5) {
                recoveryRate = 0.02; 
            } else if (attackImpact > 0.2) {
                recoveryRate = 0.03; 
            } else {
                recoveryRate = 0.08;
            }
            
            // Apply recovery and update visuals
            attackImpact = std::max(0.0, attackImpact - recoveryRate);
            updateTargetVisuals();
            
            // Schedule next recovery step
            scheduleAt(simTime() + recoveryInterval, recoveryTimer);
            
            // Log recovery milestones
            if (static_cast<int>(attackImpact * 100) % 20 == 0) {
                EV_INFO << "Node " << nodeId << " recovering from attack (impact: " << attackImpact 
                       << ") at " << simTime() << endl;
            }
        }
    }
    else {
        // Let parent class handle other messages
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

void DoSAttackApp::sendDoSMessage()
{
    // Create new WAVE Short Message for the attack
    TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
    populateWSM(wsm);
    
    // Prepare target information
    std::string targetList;
    for (const auto& target : targetNodes) {
        targetList += std::to_string(target) + " ";
    }
    // Combine attack identifier with target list
    std::string messageData = "DoS Attack Message|" + targetList;
    wsm->setDemoData(messageData.c_str());
    
    sendDown(wsm);
    
    // Log message sending (reduced frequency to prevent spam)
    if (static_cast<int>(simTime().dbl() * 10) % 10 == 0) {
        EV_INFO << "Node " << nodeId << " sent a DoS attack message targeting nodes: " 
               << targetList << " at " << simTime() << endl;
    }
}

void DoSAttackApp::updateTargetVisuals()
{
    // Update node visualization based on attack impact level
    if (attackImpact > 0.8) {
        // Severe impact - Red
        findHost()->getDisplayString().setTagArg("i", 1, "red");
        findHost()->getDisplayString().setTagArg("is", 0, "1.5");
    } 
    else if (attackImpact > 0.5) {
        // High impact - Orange
        findHost()->getDisplayString().setTagArg("i", 1, "orange");
        findHost()->getDisplayString().setTagArg("is", 0, "1.3");
    } 
    else if (attackImpact > 0.2) {
        // Medium impact - Yellow
        findHost()->getDisplayString().setTagArg("i", 1, "yellow");
        findHost()->getDisplayString().setTagArg("is", 0, "1.2");
    }
    else {
        // No impact - Normal (Green)
        findHost()->getDisplayString().setTagArg("i", 1, "green");
        findHost()->getDisplayString().setTagArg("is", 0, "1");
    }
    
    if (traciVehicle) {
        // Calculate appropriate speed based on impact level
        double recoverySpeed;
        if (attackImpact > 0.8) {
            recoverySpeed = traciVehicle->getMaxSpeed() * 0.15; 
        } else if (attackImpact > 0.5) {
            recoverySpeed = traciVehicle->getMaxSpeed() * 0.3;  
        } else if (attackImpact > 0.2) {
            recoverySpeed = traciVehicle->getMaxSpeed() * 0.5; 
        } else {
            recoverySpeed = traciVehicle->getMaxSpeed() * 1; 
        }
        
        // Gradually adjust vehicle speed for smooth transitions
        double currentSpeed = traciVehicle->getSpeed();
        double targetSpeed = recoverySpeed * (1.0 + attackImpact * 0.2); 
        double newSpeed = currentSpeed + (targetSpeed - currentSpeed) * 0.1;
        traciVehicle->setSpeed(newSpeed);
        
        // Update vehicle color to match impact level
        if (attackImpact > 0.8) {
            traciVehicle->setColor(TraCIColor(255, 0, 0, 255));      // Red
        } else if (attackImpact > 0.5) {
            traciVehicle->setColor(TraCIColor(255, 165, 0, 255));    // Orange
        } else if (attackImpact > 0.2) {
            traciVehicle->setColor(TraCIColor(255, 255, 0, 255));    // Yellow
        } else {
            traciVehicle->setColor(TraCIColor(255, 255, 255, 255));  // White
        }
    }
}

void DoSAttackApp::onWSM(BaseFrame1609_4* frame)
{
    // Cast received frame to correct message type
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);
    std::string message = wsm->getDemoData();
    
    // Process DoS attack messages
    if (message.find("DoS Attack Message") != std::string::npos) {
        if (!isAttacker && isTargetNode(nodeId)) {
            // Calculate time since last attack
            simtime_t currentTime = simTime();
            simtime_t timeSinceLastAttack = currentTime - lastAttackTime;
            lastAttackTime = currentTime;
            
            // Increase impact if attacks are frequent enough
            if (timeSinceLastAttack < simtime_t(attackInterval * 2)) {
                attackImpact = std::min(1.0, attackImpact + 0.15);  // Increase by 15%
            }
            
            // Update visual representation
            updateTargetVisuals();
            
            // Start or reset recovery process
            if (!recoveryTimer->isScheduled()) {
                scheduleAt(simTime() + recoveryInterval, recoveryTimer);
            }
            
            // Log significant impact changes
            if (static_cast<int>(attackImpact * 10) % 2 == 0) {
                EV_INFO << "Node " << nodeId << " (TARGET) under attack (impact: " << attackImpact 
                       << ") at " << simTime() << endl;
            }
        } else {
            if (static_cast<int>(simTime().dbl() * 10) % 50 == 0) {
                EV_INFO << "Node " << nodeId << " (non-target) ignored attack message at " 
                       << simTime() << endl;
            }
        }
    }
}

bool DoSAttackApp::isTargetNode(int nodeId) const
{
    // Return true if either:
    // 1. targetNodes is empty (all nodes are targets)
    // 2. The specific nodeId is in the targetNodes set
    return targetNodes.empty() || targetNodes.find(nodeId) != targetNodes.end();
}