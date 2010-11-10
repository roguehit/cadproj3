/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 IITP RAS
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * This is an example script for AODV manet routing protocol. 
 *
 * Authors: Pavel Boyko <boyko@iitp.ru>
 */

#include "ns3/aodv-module.h"
#include "ns3/core-module.h"
#include "ns3/common-module.h"
#include "ns3/node-module.h"
#include "ns3/helper-module.h"
#include "ns3/mobility-module.h"
#include "ns3/contrib-module.h"
#include "ns3/wifi-module.h" 
#include "ns3/v4ping-helper.h"
#include <iostream>
#include <cmath>
#include "ns3/visualizer.h"
#include <sys/time.h>
#include <assert.h>
#include <math.h>
#include "ns3/flow-monitor-module.h"
#include <assert.h>

#define MAX_NODE 1000
#define RAND ( rand() % (MAX_NODE) + (1) )
#define TRUE 0x1
#define FALSE 0x0

using namespace ns3;
using namespace std;

class AodvExample 
{
public:
  AodvExample ();
  /// Configure script parameters, \return true on successful configuration
  bool Configure (int argc, char **argv);
  /// Run simulation
  void Run ();
  
private:
  ///\name parameters
  //{
  /// Number of nodes
  uint32_t size;
  /// Distance between nodes, meters
  double step;
  /// Transmit Power
  double power;
  /// Simulation time, seconds
  double totalTime;
  /// Write per-device PCAP traces if true
  bool pcap;
  //\}
  double dutycycle;
  double dutyinterval;
 
  int track_position[2][MAX_NODE]; 
  ///\name network
  //\{
  NodeContainer nodes;
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;
  //\}
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor;
private:
  void CreateNodes ();
  void CreateDevices ();
  void InstallInternetStack ();
  void InstallApplications ();
  void InstallFlowMonitorModules ();
  void PrintStats ();
  void init_position();
  int  set_position (int dir);
  bool CheckValidPair(Ipv4Address node1, Ipv4Address node2 );
};

int main (int argc, char **argv)
{
  AodvExample test;
  if (! test.Configure(argc, argv)) 
    NS_FATAL_ERROR ("Configuration failed. Aborted.");
  
  test.Run();
  return 0;
}

//-----------------------------------------------------------------------------
AodvExample::AodvExample () :
  size (10),
  step (120),
  totalTime (10),
  pcap (false)
{
}

bool
AodvExample::Configure (int argc, char **argv)
{
  // Enable AODV logs by default. Comment this if too noisy
  // LogComponentEnable("AodvRoutingProtocol", LOG_LEVEL_ALL);
  
  SeedManager::SetSeed(12345);
  CommandLine cmd;
  
  power = 1000; 
  dutycycle = 0.5;
  dutyinterval = 1;

  cmd.AddValue ("pcap", "Write PCAP traces.", pcap);
  cmd.AddValue ("size", "Number of nodes.", size);
  cmd.AddValue ("time", "Simulation time, s.", totalTime);
  cmd.AddValue ("step", "Grid step, m", step);
  cmd.AddValue ("power","Transmit Power",power);
  cmd.AddValue ("duty","DutyCycle",dutycycle);
  cmd.AddValue ("interval","DutyInterval",dutyinterval);
  cmd.Parse (argc, argv);

  assert ((dutycycle <=1 ) && (dutycycle >= 0));

  /// Converting to dBm
  // The input is in milli watts
  power = 10 * log10 (power/1000) + 30;

 // printf("Power Set is %f\n",(float)power); 
  return true;
}
void AodvExample::init_position()
{
int i,j;
for(i = 0; i < 2;i++)
  for(j = 0; j < MAX_NODE ; j++ )
    track_position[i][j]=0;     

return;
}
/**
** This function returns a unique number
** between 1 to MAX_NODE which is randomly chosen
*/
int AodvExample::set_position(int dir)
{

        struct timeval now;
        gettimeofday(&now,NULL);
        //Comment this to get random placement for each run
        //srand(now.tv_usec);
	unsigned int i;
	int index = RAND;
	int SUCCESS = FALSE;

	for(i = 0; i < size; i++){
		if(!track_position[dir][index]){
			track_position[dir][ index ] = 1;
			SUCCESS = TRUE; 				
			break;
		}else index = RAND;
	}
	return SUCCESS ? index : -1;
}


void
AodvExample::Run ()
{
//  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue (1)); // enable rts cts all the time.
  //fclose(stdout);
  init_position();
  CreateNodes ();
  CreateDevices ();
  InstallInternetStack ();
  InstallApplications ();
  //printf("I am here\n");
  InstallFlowMonitorModules ();
  
  //std::cout << "Starting simulation for " << totalTime << " s ...\n";
 
  Simulator::Stop (Seconds (totalTime));
  Simulator::Run ();
  Visualizer::Run ();

  //printf("Printing Stats\n");
  PrintStats ();
  Simulator::Destroy ();
}

  // 10. Print per flow statistics
void
AodvExample::PrintStats ()
{ 
  int rxByte = 0, txByte = 0;
  //monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if(CheckValidPair(t.sourceAddress,t.destinationAddress)){
      rxByte += i->second.rxBytes;
      txByte += i->second.txBytes;	
      //std::cout << "Flow " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
      //std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
      //std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      //std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 10.0 / 1024 / 1024  << " Mbps\n";
      }
    }
printf("Efficiency = %f , Duty %f\n",(float)rxByte/txByte,dutycycle);
}

bool 
AodvExample::CheckValidPair(Ipv4Address source1, Ipv4Address dest1)
{

//printf("Checking %s & %s",node1,node2);
 //cout << source1;
 unsigned int mask = 65535;
 //cout << node1.Get();
 unsigned int source = (((unsigned int)source1.Get()) & (mask)) - 1;
 unsigned int dest = (((unsigned int)dest1.Get()) & (mask)) - 1;
  
 //cout << " Output - " << source <<endl; 
 //printf("%s %s",(char*)node1, (char*)node2);
 if( (source - (size/2)) == dest ){ 
 //cout << "Dest : "<< dest <<" Source : "<< source << endl;
 return TRUE;
 }
 return FALSE;
}



void
AodvExample::CreateNodes ()
{
  //std::cout << "Creating " << (unsigned)size << " nodes " << step << " m apart.\n";
  nodes.Create (size);
  // Name nodes
  for (uint32_t i = 0; i < size; ++i)
     {
       std::ostringstream os;
       os << "node-" << i;
       Names::Add (os.str (), nodes.Get (i));
     }
  // Create static grid
  int x,y;
  MobilityHelper mobility;
#if 0  
 mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                "MinX", DoubleValue (0.0),
                                "MinY", DoubleValue (0.0),
                                "DeltaX", DoubleValue (step),
                                "DeltaY", DoubleValue (4),
                                "GridWidth", UintegerValue (size),
                                "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
#endif

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  
  for(unsigned int i = 0 ; i < size ; i++)
  {
  while( (x = set_position(0)) == -1) {}
  while( (y = set_position(1)) == -1) {}
  //printf("X:%d, Y:%d\n",x,y);
  positionAlloc->Add (Vector (x, y, 0.0));
  //positionAlloc->Add (Vector (5.0, 0.0, 0.0));
  }  

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
}

void
AodvExample::CreateDevices ()
{
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifiMac.SetType ("ns3::AdhocWifiMac");
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();

  wifiPhy.Set("TxPowerStart",DoubleValue(power));
  wifiPhy.Set("TxPowerEnd",DoubleValue(power));
  
  YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());

  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue ("OfdmRate6Mbps"), "RtsCtsThreshold", UintegerValue (0));
  devices = wifi.Install (wifiPhy, wifiMac, nodes); 
  
  if (pcap)
    {
      wifiPhy.EnablePcapAll (std::string ("aodv"));
    }
}

void
AodvExample::InstallInternetStack ()
{
  AodvHelper aodv;
  // you can configure AODV attributes here using aodv.Set(name, value)
  InternetStackHelper stack;
  stack.SetRoutingHelper (aodv);
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.0.0.0");
  interfaces = address.Assign (devices);
}

void
AodvExample::InstallApplications ()
{
// Assigning Port numbers
  int port_ary[100000];
  for(uint32_t k = 0; k < size/2; k++)
  {
        port_ary[k] = 50000 + k;
  }        
 
 unsigned int i,j;
 
// Code for Sink Applications
 ApplicationContainer sinkApp;
 for(i=0;i < size/2;i++)
 {
  Address sinkLocalAddress (InetSocketAddress ( Ipv4Address::GetAny () ,port_ary[i]));
  
  PacketSinkHelper sinkHelper ("ns3::UdpSocketFactory",sinkLocalAddress);
  sinkApp.Add(sinkHelper.Install ( nodes.Get(i) ));
  sinkApp.Get(i)->SetStartTime (Seconds (0));
  sinkApp.Get(i)->SetStopTime (Seconds (30.0));
 }

// Code for Source Applications  
  ApplicationContainer app;  
  for(i = size/2,j=0 ; i < size ; i++,j++)
  { 
  Address remote (InetSocketAddress( interfaces.GetAddress( i-size/2),port_ary[j]));
  OnOffHelper onOffHelper("ns3::UdpSocketFactory",remote);

  onOffHelper.SetAttribute ("DataRate", StringValue ("1Mbps"));
  onOffHelper.SetAttribute ("OnTime",  RandomVariableValue (ConstantVariable (dutycycle*dutyinterval)));
  onOffHelper.SetAttribute ("OffTime", RandomVariableValue (ConstantVariable (dutyinterval-dutycycle*dutyinterval)));

  app.Add( onOffHelper.Install(nodes.Get(i))); 
  app.Get(j)->SetStartTime(Seconds(UniformVariable (0.0, 1.0).GetValue ()));
  app.Get(j)->SetStopTime(Seconds(30.0));
  }
}

// Install FlowMonitor on all nodes
void
AodvExample::InstallFlowMonitorModules ()
{
 monitor = flowmon.InstallAll();
}

