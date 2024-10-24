#include "veins/modules/application/traci/SpoofingAttackApp.h"

using namespace veins;

// Register the module with OMNeT++
Define_Module(SpoofingAttackApp);

void SpoofingAttackApp::initialize(int stage)
{
    // Call the initialize function of the parent class
    DemoBaseApplLayer::initialize(stage);
    
    // The initialization process in OMNeT++ is done in stages
    // We use stage 0 for our initialization
    if (stage == 0) {
        // Get the node's index, which will serve as its ID
        nodeId = getParentModule()->getIndex();
        
        // Get the attacker ID from the configuration file
        attackerId = par("attackerId").intValue();
        
        // Check if this node is the attacker
        isAttacker = (nodeId == attackerId);
        
        if (isAttacker) {
            // Create a timer for the attacker to start the attack
            attackTimer = new cMessage("attackTimer");
            
            // Get the attack start time from the configuration file
            simtime_t attackStartTime = par("attackStartTime").doubleValue();
            
            // Schedule the attack to start at the specified time
            scheduleAt(simTime() + attackStartTime, attackTimer);
            
            EV_INFO << "SpoofingAttackApp initialized for Node " << nodeId 
                    << " (Attacker), attack scheduled at " << simTime() + attackStartTime << endl;
        } else {
            EV_INFO << "SpoofingAttackApp initialized for Node " << nodeId << " (Regular node)" << endl;
        }
    }
}

void SpoofingAttackApp::handleSelfMsg(cMessage* msg)
{
    // Check if the received message is our attack timer
    if (msg == attackTimer) {
        EV_INFO << "Attack timer triggered for Node " << nodeId << " at " << simTime() << endl;
        
        // Initiate the spoofing attack by sending a fake accident message
        sendFakeAccidentMessage();
    } else {
        // If it's not our timer, let the parent class handle it
        DemoBaseApplLayer::handleSelfMsg(msg);
    }
}

void SpoofingAttackApp::sendFakeAccidentMessage()
{
    // Create a new WAVE Short Message
    TraCIDemo11pMessage* wsm = new TraCIDemo11pMessage();
    
    // Populate the WSM with necessary fields (handled by parent class)
    populateWSM(wsm);
    
    // Set the demo data to indicate a fake accident
    wsm->setDemoData("Fake Accident");
    
    // Send the message
    sendDown(wsm);
    
    EV_INFO << "Node " << nodeId << " sent a fake accident message at " << simTime() << endl;
}

void SpoofingAttackApp::onWSM(BaseFrame1609_4* frame)
{
    // Cast the received frame to our specific message type
    TraCIDemo11pMessage* wsm = check_and_cast<TraCIDemo11pMessage*>(frame);
    
    // Check if the received message is our fake accident message
    if (std::string(wsm->getDemoData()) == "Fake Accident") {
        EV_INFO << "Node " << nodeId << " received a fake accident message at " << simTime() << endl;
        
        // React to the fake accident message only if this node is not the attacker
        if (!isAttacker) {
            // Change the node's display color to red in the simulation GUI
            findHost()->getDisplayString().setTagArg("i", 1, "red");
            
            // Stop the vehicle
            if (traciVehicle) {
                traciVehicle->setSpeed(0);
            }
            
            EV_INFO << "Node " << nodeId << " stopped due to fake accident message at " << simTime() << endl;
        }
    }
}