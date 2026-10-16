#include "ns3/vector.h"
#include "ns3/string.h"
#include "ns3/socket.h"
#include "ns3/double.h"
#include "ns3/config.h"
#include "ns3/log.h"
#include "ns3/command-line.h"
#include "ns3/mobility-model.h"
#include "ns3/position-allocator.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include <iostream>
#include "ns3/tbcm.h"

#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/wifi-module.h"

#include "ns3/yans-wifi-helper.h"
#include "ns3/core-module.h"
#include "ns3/applications-module.h"
#include "ns3/animation-interface.h"
#include "ns3/mobility-module.h"
#include "ns3/aodv-helper.h"
#include "ns3/dsdv-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/energy-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("BlockChain");

void ReceivePacket (Ptr<Socket> socket)
{
	while (socket->Recv ())
	{
		NS_LOG_UNCOND ("Initializing Data Transmission!");
	}
}

static void results(tbcm *device,int n_nodes)
{
	double	pdr = double(device->recv_count) / device->send_count * 100;
	double	delay = 0;
	double	jitter_sum = 0;
	for(int i=0;i<device->recv_count;i++)
	{
		delay += device->txDelay[i];

		if(i > 0)
		{
			jitter_sum += (device->rxTime[i] - device->rxTime[i-1]);
		}		
	}

	double	avgDelay = delay / device->recv_count;
	double	jitter = jitter_sum / (device->recv_count-1);

	double	throughput = device->bytes*8 / (device->pend - device->pinit);
	double	goodput = device->bytes*8 / avgDelay;

	cout<<"Simulation Results"<<endl;
	cout<<"Results_Pkt_send:\t\t"<<device->send_count<<endl;
	cout<<"Results_Pkt_recv:\t\t"<<device->recv_count<<endl;
	cout<<"Results_PDR:\t\t\t"<<pdr<<endl;
	cout<<"Results_avgDelay:\t\t"<<avgDelay<<endl;
	cout<<"Results_Jitter:\t\t\t"<<jitter<<endl;
	cout<<"Results_Throughput:\t\t"<<throughput<<endl;
	cout<<"Results_Goodput:\t\t"<<goodput<<endl;
	cout<<"Results_Overheads:\t\t"<<device->routingOverhead<<endl;
	cout<<"Results_NRO:\t\t\t"<<(double(device->routingOverhead) / device->recv_count)<<endl;
}
static void consensusResults(tbcm *device)
{
	double simTime = CURRENT_TIME;
	double consensusThroughput = (simTime > 0) ? (tbcm::blockCount / simTime) : 0;

	double avgBlockInterval = 0;
	for (size_t i = 0; i < tbcm::blockIntervals.size(); i++)
		avgBlockInterval += tbcm::blockIntervals[i];
	if (tbcm::blockIntervals.size() > 0)
		avgBlockInterval /= tbcm::blockIntervals.size();

	double blockSuccessRate = (tbcm::blockRoundsAttempted > 0)
		? (100.0 * tbcm::blockRoundsSucceeded / tbcm::blockRoundsAttempted) : 0;

	double avgMsgSize = (tbcm::consensusMsgCount > 0)
		? ((double)tbcm::consensusBytes / tbcm::consensusMsgCount) : 0;

	double avgEstLatency = (tbcm::latencySamples > 0)
		? (tbcm::sumEstLatency / tbcm::latencySamples) : 0;

	cout<<"Consensus Results"<<endl;
	cout<<"Results_BlocksCreated:\t\t"<<tbcm::blockCount<<endl;
	cout<<"Results_ConsensusThroughput:\t"<<consensusThroughput<<endl;
	cout<<"Results_AvgBlockInterval:\t"<<avgBlockInterval<<endl;
	cout<<"Results_BlockRoundsAttempted:\t"<<tbcm::blockRoundsAttempted<<endl;
	cout<<"Results_BlockSuccessRate:\t"<<blockSuccessRate<<endl;
	cout<<"Results_ConsensusMsgCount:\t"<<tbcm::consensusMsgCount<<endl;
	cout<<"Results_ConsensusBytes:\t\t"<<tbcm::consensusBytes<<endl;
	cout<<"Results_AvgConsensusMsgSize:\t"<<avgMsgSize<<endl;
	cout<<"Results_ConsensusEnergyProxy:\t"<<tbcm::consensusEnergyConsumed<<endl;
	cout<<"Results_EstConsensusLatency:\t"<<avgEstLatency<<endl;
	cout<<"Results_FaultyVoteCount:\t"<<tbcm::faultyVoteCount<<endl;
	cout<<"Results_FaultyNodePercent:\t"<<(tbcm::faultyNodePercent*100)<<endl;
cout<<"Results_InitialCollisions:\t"<<GERE::initialCollisionsDetected<<endl;
cout<<"Results_ReworkAttempts:\t\t"<<GERE::reworkAttemptsTotal<<endl;
cout<<"Results_ReworkSuccess:\t\t"<<GERE::reworkSuccessTotal<<endl;

long tp = tbcm::truePositives, fp = tbcm::falsePositives, tn = tbcm::trueNegatives, fn = tbcm::falseNegatives;
double precision = 0, recall = 0, specificity = 0, f1 = 0;

if ((tp+fp) > 0) precision = (double)tp / (tp+fp);
if ((tp+fn) > 0) recall = (double)tp / (tp+fn);        // recall == sensitivity, same formula
if ((tn+fp) > 0) specificity = (double)tn / (tn+fp);
if ((precision+recall) > 0) f1 = 2*precision*recall/(precision+recall);

cout<<"Results_TP:\t\t\t"<<tp<<endl;
cout<<"Results_FP:\t\t\t"<<fp<<endl;
cout<<"Results_TN:\t\t\t"<<tn<<endl;
cout<<"Results_FN:\t\t\t"<<fn<<endl;
cout<<"Results_Precision:\t\t"<<(precision*100)<<endl;
cout<<"Results_Recall:\t\t\t"<<(recall*100)<<endl;
cout<<"Results_Sensitivity:\t\t"<<(recall*100)<<endl;
cout<<"Results_Specificity:\t\t"<<(specificity*100)<<endl;
cout<<"Results_F1Score:\t\t"<<(f1*100)<<endl;


}

static void energyResults(tbcm **device_, int n_nodes)
{
	double totalEnergy = 0;
	int aliveCount = 0;
	for (int i = 0; i < n_nodes; i++)
	{
		totalEnergy += device_[i]->energy;
		if (device_[i]->energy > 0) aliveCount++;
	}
	double avgEnergy = totalEnergy / n_nodes;
	double aliveP = 100.0 * aliveCount / n_nodes;

	cout<<"Energy Results"<<endl;
	cout<<"Results_AvgResidualEnergy:\t"<<avgEnergy<<endl;
	cout<<"Results_PercentNodesAlive:\t"<<aliveP<<endl;
	cout<<"Results_DeadNodes:\t\t"<<(n_nodes - aliveCount)<<endl;
}
static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize,
	              uint32_t pktCount, Time pktInterval )
{
	if (pktCount > 0)
	{
	socket->Send (Create<Packet> (pktSize));
	Simulator::Schedule (pktInterval, &GenerateTraffic,
	            socket, pktSize,pktCount - 1, pktInterval);
	}
	else
	{
	socket->Close ();
	}
}

double   RandomDist(double a,double b)
{
    if (b == a) return a;
    else if (a < b) return (b - a) * ((float)rand() / RAND_MAX) + a;
    return 0;
}

int main (int argc, char *argv[])
{
	int	proposed = 1;
	int	stop_time =200;
	int    useROCMode = 0;
	double detectionThreshold = 0.5;

	std::string phyMode ("DsssRate1Mbps");
	uint32_t packetSize = 1150;
	uint32_t numPackets = 1;
	double	pkt_ivl = 0.1;
	bool	verbose = false;
	int	n_nodes = 200;

	double	rss = -80;
	double	coverage = 100.0;
	double	topo_x = 500;
	double	topo_y = 500;
	double	init_energy = 100;
double	faultyPercent = 0.0;
	double	threshold = 0.60;
	double	bcInterval = 5.0;

	double	interval = pkt_ivl * (n_nodes - 1);
	cerr<<"Simulation Started..."<<endl;

	double	Eelec = 5e-9;
	double	Efs = 5e-12;
	double	Emp= 0.0013e-12;

	double	freq = 2.3e6;
	double	speed = 0;

	double	gain = 1;
	double	Pt = 0.1;


	CommandLine cmd;
cmd.AddValue ("useROCMode", "1 = ROC-curve detector, 0 = existing fixed-probability detector", useROCMode);
cmd.AddValue ("detectionThreshold", "Detection score threshold for ROC mode (0-1)", detectionThreshold);

	cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
	cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
	cmd.AddValue ("numPackets", "number of packets generated", numPackets);
	cmd.AddValue ("interval", "interval (seconds) between packets", interval);
	cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
	cmd.AddValue ("proposed", "0", proposed);
cmd.AddValue ("n_nodes", "Number of nodes", n_nodes);
	cmd.AddValue ("coverage", "Transmission range (m)", coverage);
	cmd.AddValue ("topo_x", "Topology width (m)", topo_x);
	cmd.AddValue ("topo_y", "Topology height (m)", topo_y);
	cmd.AddValue ("init_energy", "Initial node energy (J)", init_energy);
	cmd.AddValue ("speed", "Node movement speed (m/s)", speed);
	cmd.AddValue ("faultyPercent", "Fraction of faulty validators (0-1)", faultyPercent);
	cmd.AddValue ("threshold", "Consensus vote threshold (0-1)", threshold);
	cmd.AddValue ("bcInterval", "Blockchain round interval (s)", bcInterval);
	cmd.Parse (argc, argv);
	tbcm::useROCMode = useROCMode;
	tbcm::detectionThreshold = detectionThreshold;
tbcm::faultyNodePercent = faultyPercent;
	tbcm::consensusThreshold = threshold;
tbcm::consensusMode = proposed;
	Time interPacketInterval = Seconds (interval);

	NodeContainer node_;
	node_.Create (n_nodes);

	WifiHelper wifi;
	if (verbose)
	{
		wifi.EnableLogComponents ();
	}
	wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

	YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
	wifiPhy.Set ("RxGain", DoubleValue (0) );
	wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);  

	YansWifiChannelHelper wifiChannel;
	wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss", DoubleValue (rss));
	wifiChannel.AddPropagationLoss ("ns3::RangePropagationLossModel", "MaxRange", DoubleValue (coverage));
	wifiPhy.SetChannel (wifiChannel.Create ());

	WifiMacHelper deviceMac;
	wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode",StringValue (phyMode),"ControlMode",StringValue (phyMode));
	deviceMac.SetType ("ns3::AdhocWifiMac");
	NetDeviceContainer devices = wifi.Install (wifiPhy, deviceMac, node_);

	MobilityHelper mobility;
	Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

	positionAlloc->Add (Vector (topo_x/2,topo_y/2,0.0));

	for(int i=0;i<n_nodes;i++)
	{
		double x = RandomDist(0,topo_x);
		double y = RandomDist(0,topo_y);

		positionAlloc->Add (Vector (x,y,0.0));
	}

	cout<<"Coverage: "<<coverage<<endl;
	tbcm *device_[n_nodes];
	nsaddr_t bs_id = 0;

	for (int i=0;i<n_nodes;i++)
	{
		NodeContainer mn = NodeContainer (node_.Get (i));
		Ptr <Node> n = node_.Get (i);
		n->coverage = coverage;
		n->proposed = proposed;

		tbcm *device = new tbcm();
		device->index = i;
		device->freq = freq;
		device->node_ = n;
		device->linkLayerTarget = (void*)(deviceMac.Get(i));
		device_[i] = device;
		device->node_->speed = speed;
		device->topo_x = topo_x;
		device->topo_y = topo_y;
		device->Eelec = Eelec;
		device->Efs = Efs;
		device->Emp = Emp;
		device->gain = gain;
		device->Pt = Pt;
		device->setEnergy(init_energy);
device->bcTimer.interval = bcInterval;
	}

	device_[bs_id]->BS = 1;

	for(int i=0;i<n_nodes;i++)
	{
		device_[i]->initializeMovement();
		device_[i]->initializeBeaconTimer();
	}
	
	mobility.SetPositionAllocator (positionAlloc);
	mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	mobility.Install (node_);

	InternetStackHelper internet;

	BasicEnergySourceHelper basicSourceHelper;
	EnergySourceContainer sources = basicSourceHelper.Install (node_);
	WifiRadioEnergyModelHelper radioEnergyHelper;
	radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.0174));
	DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install (devices, sources);

	Ptr<BasicEnergySource> basicSourcePtr_[1000];
	Ptr<DeviceEnergyModel> basicRadioModelPtr_[1000];

	Ipv4StaticRoutingHelper staticRouting;
	Ipv4ListRoutingHelper list;
	list.Add (staticRouting, 0);
	internet.SetRoutingHelper (list);
	internet.Install (node_);

	Ipv4AddressHelper ipv4;
	NS_LOG_INFO ("Assign IP Addresses.");
	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer i = ipv4.Assign (devices);

	double	min_interval = 0.1;
	double	start_time = 10;
	
	int flag = 0;

	for(int i=1;i<n_nodes;i++)
	{
		nsaddr_t src = i;
		nsaddr_t dst = bs_id;
		double	startTime = start_time + i * min_interval;

		TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
		Ptr<Socket> recvSink = Socket::CreateSocket (node_.Get (src), tid);
		InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
		recvSink->Bind (local);
		recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

		Ptr<Socket> source = Socket::CreateSocket (node_.Get (dst), tid);
		InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
		source->SetAllowBroadcast (true);
		source->Connect (remote);

		if(flag == 0)
		{
			flag = 1;
			Simulator::ScheduleWithContext(source->GetNode ()->GetId(), Seconds(startTime), &GenerateTraffic, source, packetSize, numPackets, interPacketInterval);
		}
		device_[src]->initializeDataTransmission(dst, Create<Packet>(), packetSize, interval, startTime, stop_time);
	}

	Simulator::Schedule(Seconds(100), &results, device_[0], n_nodes);
	Simulator::Schedule(Seconds(125), &results, device_[0], n_nodes);
	Simulator::Schedule(Seconds(150), &results, device_[0], n_nodes);
	Simulator::Schedule(Seconds(175), &results, device_[0], n_nodes);
	Simulator::Schedule(Seconds(stop_time), &results, device_[0], n_nodes);
	Simulator::Schedule(Seconds(100), &consensusResults, device_[0]);
	Simulator::Schedule(Seconds(125), &consensusResults, device_[0]);
	Simulator::Schedule(Seconds(150), &consensusResults, device_[0]);
	Simulator::Schedule(Seconds(175), &consensusResults, device_[0]);
	Simulator::Schedule(Seconds(stop_time), &consensusResults, device_[0]);

	Simulator::Schedule(Seconds(100), &energyResults, device_, n_nodes);
	Simulator::Schedule(Seconds(125), &energyResults, device_, n_nodes);
	Simulator::Schedule(Seconds(150), &energyResults, device_, n_nodes);
	Simulator::Schedule(Seconds(175), &energyResults, device_, n_nodes);
	Simulator::Schedule(Seconds(stop_time), &energyResults, device_, n_nodes);

	AnimationInterface *anim = new AnimationInterface("animation.xml");
	anim->EnablePacketMetadata (true);
	anim->SetStopTime(Seconds(stop_time));

	for(int i=0;i<n_nodes;i++)
	{
		device_[i]->anim = anim;
	}

	Simulator::Stop(Seconds(stop_time));
	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
