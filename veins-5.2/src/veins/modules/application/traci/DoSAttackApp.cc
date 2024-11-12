#include "veins/modules/application/traci/DoSAttackApp.h"
#include "veins/modules/mac/ieee80211p/Mac1609_4.h"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "veins/base/modules/BaseMobility.h"
#include "veins/modules/mobility/traci/TraCIMobility.h"
#include "veins/base/utils/Coord.h"

using namespace veins;

Define_Module(DoSAttackApp);

void DoSAttackApp::initialize(int stage)
{
    DemoBaseApplLayer::initialize(stage);
    
    if (stage == 0) {
        // Initialize parameters
        nodeId = getParentModule()->getIndex();
        attackerId = par("attackerId");
        isAttacker = (nodeId == attackerId);
        pingInterval = par("attackInterval").doubleValue();
        attackStartTime = par("attackStartTime").doubleValue();
        attackStopTime = par("attackStopTime").doubleValue();
        
        // Initialize counters
        pingsSent = 0;
        pingsReceived = 0;
        responsesSent = 0;
        responsesDropped = 0;
        
        // Parse target nodes
        std::string targetNodesStr = par("targetNodes").stdstringValue();
        std::istringstream iss(targetNodesStr);
        int target;
        while (iss >> target) {
            targetNodes.insert(target);
            EV_INFO << "Added target node: " << target << endl;
        }
        
        // Register signals
        packetSentSignal = registerSignal("packetSent");
        packetReceivedSignal = registerSignal("packetReceived");
        
        if (isAttacker) {
            // Set attacker visual properties
            findHost()->getDisplayString().setTagArg("i", 1, "red");
            
            pingTimer = new cMessage("pingTimer");
            scheduleAt(attackStartTime, pingTimer);
            
            EV_INFO << "DoS Attacker initialized - Node " << nodeId 
                   << " will start attack at " << attackStartTime 
                   << " targeting nodes: " << targetNodesStr << endl;
        } else {
            if (isTargetNode(nodeId)) {
                findHost()->getDisplayString().setTagArg("i", 1, "green");
                monitorTimer = new cMessage("monitorTimer");
                scheduleAt(simTime() + 1, monitorTimer);
                EV_INFO << "Target node " << nodeId << " initialized" << endl;
            }
        }
    }
}

void DoSAttackApp::handleSelfMsg(cMessage* msg)
{
    if (msg == pingTimer && isAttacker) {
        if (simTime() >= attackStartTime && simTime() <= attackStopTime) {
            // Send attack messages
            sendPingBurst();
            // Schedule next burst
            scheduleAt(simTime() + pingInterval, pingTimer);
        } 
        else if (simTime() > attackStopTime) {
            // Stop the attack
            EV_INFO << "Attack stopped at " << simTime() 
                   << ". Total messages sent: " << pingsSent << endl;
            
            // Change color back to indicate attack has stopped
            findHost()->getDisplayString().setTagArg("i", 1, "blue");
            
            // Don't reschedule the timer
            cancelAndDelete(pingTimer);
            pingTimer = nullptr;
        }
    }
    else if (msg == monitorTimer && isTargetNode(nodeId)) {
        monitorQueueStatus();
        if (simTime() <= attackStopTime) {
            scheduleAt(simTime() + 0.1, monitorTimer);
        } else {
            cancelAndDelete(monitorTimer);
            monitorTimer = nullptr;
        }
    }
    else if (strcmp(msg->getName(), "processingDelay") == 0) {
        // Handle the delayed response after processing time
        int* targetId = static_cast<int*>(msg->getContextPointer());
        
        cModule* nicModule = findHost()->getSubmodule("nic");
        Mac1609_4* mac = check_and_cast<Mac1609_4*>(nicModule->getSubmodule("mac1609_4"));
        
        TraCIDemo11pMessage* response = new TraCIDemo11pMessage();
        populateWSM(response);
        
        response->setSenderAddress(nodeId);
        response->setDemoData("PONG");
        response->setByteLength(1000);
        
        // Send the response after the delay
        sendDown(response);
        responsesSent++;
        emit(packetSentSignal, 1L);
        
        // Clean up
        delete targetId;
        delete msg;
    }
}

void DoSAttackApp::sendPingBurst()
{
    // Get MAC layer for queue monitoring
    cModule* nicModule = findHost()->getSubmodule("nic");
    Mac1609_4* mac = check_and_cast<Mac1609_4*>(nicModule->getSubmodule("mac1609_4"));
    
    // Send multiple messages in a burst
    for (int i = 0; i < burstSize; i++) {
        TraCIDemo11pMessage* ping = new TraCIDemo11pMessage();
        populateWSM(ping);
        
        ping->setSenderAddress(nodeId);
        ping->setDemoData("PING");
        ping->setByteLength(1000);
        
        sendDown(ping);
        pingsSent++;
        emit(packetSentSignal, 1L);
        
        // Log every 100th message
        if (pingsSent % 100 == 0) {
            Coord myPos = mobility->getPositionAt(simTime());
            for (int targetId : targetNodes) {
                cModule* targetModule = getSimulation()->getSystemModule()
                    ->getModuleByPath(("RSUExampleScenario.node[" + std::to_string(targetId) + "]").c_str());
                if (targetModule) {
                    cModule* mobilityModule = targetModule->getSubmodule("veinsmobility");
                    if (mobilityModule) {
                        TraCIMobility* targetMobility = check_and_cast<TraCIMobility*>(mobilityModule);
                        Coord targetPos = targetMobility->getPositionAt(simTime());
                        double distance = myPos.distance(targetPos);
                        
                        EV_INFO << "Attack progress at " << simTime() 
                               << "\n  Target " << targetId 
                               << "\n  Distance: " << distance << "m"
                               << "\n  Pings sent: " << pingsSent
                               << endl;
                    }
                }
            }
        }
    }
}

void DoSAttackApp::onWSM(BaseFrame1609_4* frame)
{
    TraCIDemo11pMessage* wsm = dynamic_cast<TraCIDemo11pMessage*>(frame);
    if (!wsm) return;
    
    std::string data = wsm->getDemoData();
    
    if (isTargetNode(nodeId)) {
        if (data == "PING") {
            pingsReceived++;
            emit(packetReceivedSignal, 1L);
            
            // Schedule delayed response after X processing time
            // Also remember to change SIMTIME_US! :)
            cMessage* delayMsg = new cMessage("processingDelay");
            delayMsg->setContextPointer(new int(wsm->getSenderAddress()));
            scheduleAt(simTime() + SimTime(1000, SIMTIME_US), delayMsg);
            
            // Log every 100th message
            if (pingsReceived % 100 == 0) {
                Coord myPos = mobility->getPositionAt(simTime());
                cModule* senderModule = getSimulation()->getSystemModule()
                    ->getModuleByPath(("RSUExampleScenario.node[" + std::to_string(wsm->getSenderAddress()) + "]").c_str());
                
                if (senderModule) {
                    cModule* mobilityModule = senderModule->getSubmodule("veinsmobility");
                    if (mobilityModule) {
                        TraCIMobility* senderMobility = check_and_cast<TraCIMobility*>(mobilityModule);
                        Coord senderPos = senderMobility->getPositionAt(simTime());
                        double distance = myPos.distance(senderPos);
                        
                        EV_INFO << "Target Node " << nodeId << " status:" 
                               << "\n  Time: " << simTime()
                               << "\n  Pings received: " << pingsReceived
                               << "\n  Responses sent: " << responsesSent
                               << "\n  Distance from attacker: " << distance << "m"
                               << endl;
                    }
                }
            }
        }
    }
}

void DoSAttackApp::monitorQueueStatus()
{
    cModule* nicModule = findHost()->getSubmodule("nic");
    Mac1609_4* mac = check_and_cast<Mac1609_4*>(nicModule->getSubmodule("mac1609_4"));
    
    // Update statistics
    if (simTime().dbl() - lastStatsTime > 1.0) {
        // Calculate packet handling rates
        double timeElapsed = simTime().dbl() - lastStatsTime;
        double receiveRate = timeElapsed > 0 ? pingsReceived / timeElapsed : 0;
        double responseRate = timeElapsed > 0 ? responsesSent / timeElapsed : 0;
        
        std::ostringstream label;
        label << "Status:\n"
              << "Pings Rcvd: " << pingsReceived << "\n"
              << "Resp Sent: " << responsesSent << "\n"
              << "Rate: " << static_cast<int>(receiveRate) << "/s";
        
        findHost()->getDisplayString().setTagArg("t", 0, label.str().c_str());
        
        EV_INFO << "=== Node " << nodeId << " Status at " << simTime() << " ===\n"
               << "Pings received: " << pingsReceived << "\n"
               << "Responses sent: " << responsesSent << "\n"
               << "Receive rate: " << receiveRate << " msgs/sec\n"
               << "Response rate: " << responseRate << " msgs/sec\n"
               << "=================================" << endl;
        
        lastStatsTime = simTime().dbl();
    }
}

bool DoSAttackApp::isTargetNode(int nodeId) const
{
    return targetNodes.find(nodeId) != targetNodes.end();
}

void DoSAttackApp::finish()
{
    if (isAttacker) {
        // Log final attack statistics
        EV_INFO << "\n=== Final DoS Attack Statistics for Node " << nodeId << " ===\n"
                << "Total pings sent: " << pingsSent << "\n"
                << "Attack duration: " << (attackStopTime - attackStartTime) << " seconds\n"
                << "Average send rate: " 
                << (pingsSent / (attackStopTime - attackStartTime).dbl()) 
                << " msgs/sec\n"
                << "==========================================\n" << endl;
    }
    else if (isTargetNode(nodeId)) {
        // Log final target statistics
        double dropRate = (responsesSent + responsesDropped > 0) ? 
                         (responsesDropped * 100.0 / (responsesSent + responsesDropped)) : 0;
        
        EV_INFO << "\n=== Final Target Node " << nodeId << " Statistics ===\n"
                << "Total pings received: " << pingsReceived << "\n"
                << "Total responses sent: " << responsesSent << "\n"
                << "Total responses dropped: " << responsesDropped << "\n"
                << "Response drop rate: " << dropRate << "%\n"
                << "Attack duration: " << (attackStopTime - attackStartTime) << " seconds\n"
                << "Average reception rate: " 
                << (pingsReceived / (attackStopTime - attackStartTime).dbl()) 
                << " msgs/sec\n"
                << "==========================================\n" << endl;
    }

    // Cleanup
    if (pingTimer) {
        cancelAndDelete(pingTimer);
        pingTimer = nullptr;
    }
    if (monitorTimer) {
        cancelAndDelete(monitorTimer);
        monitorTimer = nullptr;
    }

    DemoBaseApplLayer::finish();
}