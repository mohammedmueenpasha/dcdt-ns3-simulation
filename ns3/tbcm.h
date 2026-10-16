#ifndef TBCM_H
#define TBCM_H

#include "ns3/node.h"
#include "ns3/random-variable-stream.h"
#include "ns3/output-stream-wrapper.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/ipv4-interface.h"
#include "ns3/ipv4-l3-protocol.h"
#include <map>
#include "ns3/node-list.h"
#include <ns3/mobility-model.h>
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include <vector>

#include "ns3/timer.h"
#include "ns3/callback.h"
#include "mylist.h"
#include "ns3/animation-interface.h"

#include <string>

namespace ns3 {

class	tbcm;
using namespace std;

#define	TBCM			tbcm

#define	AggrInterval		10.0

#define	minPH			0
#define	maxPH			14

#define	ms_speed		5
#define	SNR			delta
#define	gainJ			alpha
#define CONSENSUS_MSG_ENERGY_COST   0.002   
#define	MIN_DATA_COUNT		10
#define	tow			2
#define SPEED_OF_LIGHT		300000000		// 3 * 10^8 m/s
#define	BS_INTERVAL		25.0
#define	movementArea		20
#define	CLEN			10
#define FALSE_ALARM_RATE_PCT 5.0   // % chance DCDT incorrectly flags an honest vote
#define	MEM_LIMIT		20
#define	Txp			0.1
#define	MIN_DELAY		Random(0,PROP_DELAY)
#define	RSS_BOUND(x)		Random(0,x)
#define	BOUND(x)		Random(0,x)

#define	tdel			PROP_DELAY
#define	NS_LOG_COMPONENT	NS_COMPONENT_DEFINE

enum {INITIAL,HEAD,MEMBER};

LIST_HEAD(device_head, tbcm);

struct Point {
	double x, y;
};

struct Card {
	int rows;
	int cols;
	int filledCount;
	int* cells;
	int* strikes;
};

class Player {
	public:
		unsigned long long id;
		Card card;
		Player() { }
		Player(unsigned long long pid, const Card& c);
};

struct LiveEvent {
	int number;
	int index;
	unsigned long long timestamp;
};

class PRNG {
	private:
		unsigned long long state;
	public:
		PRNG(unsigned long long seed);
		unsigned long long next();
};

/*class GERE {
	private:
		int maxN;
		int counts[1001];
		unsigned long long totalObserved;
		int rounds;
		bool biased;
		int biasedNumber;
	public:
		GERE();
		void observeSequence(int *seq, int n);
		void analyze(int n);
		bool isBiased() const;
		int getBiasedNumber() const;
		void perturbSequence(int *seq, int n, unsigned long long gid, unsigned long long theta);
		void resetIfNeeded(int n);
};
*/
class GERE {
	private:
		int maxN;
		int counts[1001];
		unsigned long long totalObserved;
		int rounds;
		bool biased;
		int biasedNumber;

		static vector<vector<int>> winningPatternHistory;
	public:
		static const int MAX_HISTORY = 50;
		static const int MAX_REWORK_ATTEMPTS = 10;
		static const int CHECK_WINDOW = 1;

		static long initialCollisionsDetected;
		static long reworkAttemptsTotal;
		static long reworkSuccessTotal;

		GERE();
		void observeSequence(int *seq, int n);
		void analyze(int n);
		bool isBiased() const;
		int getBiasedNumber() const;
		void perturbSequence(int *seq, int n, unsigned long long gid, unsigned long long theta);
		void resetIfNeeded(int n);

		bool matchesPreviousWinningPattern(const int* seq, int checkCount);
		static void recordWinningPattern(const int* trueList, int trueCount);
		void reworkSequenceAvoidingHistory(int* seq, int n, unsigned long long gid, unsigned long long theta);
};

string sha256(const string& input);
string aes_encrypt(const string& msg, int key);
string aes_decrypt(const string& msg, int key);

int CellCount(int totalCells, unsigned long long gid,
		unsigned long long uid, unsigned long long theta);

void CellPosition(int totalCells, int C, unsigned long long gid,
		unsigned long long uid, const string &theta,
		int *outPositions);

void CellValues(unsigned long long uid,
		unsigned long long gid,
		unsigned long long theta,
		int Vmin, int Vmax,
		int C,
		int *positions,
		int *outCells);

void GenerateMasterSequence(int N,
		unsigned long long gid,
		unsigned long long theta,
		int *outSequence,
		int &outCount,
		string &masterHash);

void BroadcastToAll(int number, int index);

void StrikeNumberOnCard(Card& c, int number);

void LiveDraw(const int* drawSeq,
		int count,
		Player* players,
		int playerCount,
		LiveEvent* outEvents);


enum NodeType {
	TN,
	VN,
	MN
};

class BCNode {
	public:
		NodeType type;
		unsigned long long nodeID;
		unsigned long long thetaConst;
		BCNode(NodeType t, unsigned long long id, unsigned long long theta);
};

class TransactionNode : public BCNode {
	public:
		TransactionNode(unsigned long long id, unsigned long long theta);
		Card generateCard(unsigned long long gameID, int r, int c);
		string sendCardToVN(const Card& card, unsigned long long gameID);
};

class ValidationNode : public BCNode {
	public:
		ValidationNode(unsigned long long id, unsigned long long theta);
		bool validateCard(const Card& card);
		string sendValidationToMN(const Card& card);
};

class MasterNode : public BCNode {
	public:
		MasterNode(unsigned long long id, unsigned long long theta);
		string storeBlock(const string& blockData);
};

struct WinBroadcastMessage {
	unsigned long long uid;
	unsigned long long gid;
	unsigned long long theta;
	int* drawSeq;
	int drawCount;
	int* trueList;
	int trueCount;
	int lastIndex;
	string claimPattern;
	string integrityHash;
};

void WinBroadcast(unsigned long long uid,
		unsigned long long gid,
		unsigned long long theta,
		int* drawSeq,
		int drawCount,
		int* trueList,
		int trueCount,
		int lastIndex,
		const string& winPattern,
		WinBroadcastMessage* outMsg);

enum ClaimResultType {
	CLAIM_WIN,
	CLAIM_LOSE,
	CLAIM_UNDECIDED
};

struct ClaimResult {
	ClaimResultType result;
	int* matchedCells;
	int matchedCount;
	int usedIndex;
	string reason;
};

ClaimResult VerifyClaim(unsigned long long gid,
		unsigned long long uid,
		unsigned long long theta,
		int totalCells,
		int rows,
		int cols,
		int* drawSeq,
		int drawCount,
		int* trueList,
		int trueCount,
		int lastIndex,
		const string& winPattern,
		const string& masterHash);

struct WinnerRecord {
	unsigned long long uid;
	int lastIndex;
	unsigned long long timestamp;
	string pattern;
	bool exists;
};

struct ClaimObject {
	unsigned long long uid;
	unsigned long long gid;
	unsigned long long theta;
	int* drawSeq;
	int drawCount;
	int* trueList;
	int trueCount;
	int lastIndex;
	string pattern;
	unsigned long long timestamp;
};

void BroadcastWinner(const WinnerRecord& w);
void BroadcastWinnerUpdate(const WinnerRecord& w);
void ClaimRejected(unsigned long long uid, const string& reason);

void ProcessClaim(const ClaimObject& C,
		WinnerRecord& CurrentWinner,
		int totalCells,
		int rows,
		int cols,
		const string& masterHash);

struct WinningClaim {
	unsigned long long uid;
	unsigned long long gid;
	unsigned long long theta;
	int* drawSeq;
	int drawCount;
	int* trueList;
	int trueCount;
	int lastIndex;
	string pattern;
	unsigned long long timestamp;
	string masterHash;
};

struct BlockData {
	int index;
	unsigned long long timestamp;
	string claimSerialized;
	string previousHash;
	string hash;
};

void ProposeWinnerBlock(WinningClaim* winClaims,
		int claimCount,
		unsigned long long* networkNodes,
		int nodeCount,
		const string& previousBlockHash,
		int previousBlockIndex,
		double threshold,
		BlockData* outBlock);

#define	CURRENT_TIME		Simulator::Now().GetSeconds ()
#define	myindex			(node_->GetId())
#define	nsaddr_t		int32_t

#define	BEACON_INTERVAL		10.0
#define	DEVICE			1
#define	DATA_PACKET		2

#define	devicePort		12345
#define	dataPort		01
#define	PROP_DELAY		0.001

#define	IP_BROADCAST		-1

class	BlockChainTimer : public Timer
{
	public:
	double		interval;
	tbcm		*agent;
	int		run;

	BlockChainTimer(ns3::Timer::DestroyPolicy dp) : Timer(dp)
	{
		interval = 0;
		run = 0;
	}

};

class	DataTimer : public Timer
{
	public:
	double		interval;
	int		packetSize;
	double		startTime;
	double		stopTime;
	int		run;
	Ptr<Packet>	dataPacket;
	nsaddr_t	dst;
	int		init;

	DataTimer(ns3::Timer::DestroyPolicy dp) : Timer(dp)
	{
		packetSize = 0;
		interval = 0;
		stopTime = 0;
		run = 0;
	}
};

class	ResultTimer : public Timer
{
	public:
		int	run;
		ResultTimer(ns3::Timer::DestroyPolicy dp) : Timer(dp)
	{
		run = 0;
	}
};

class	InfoTimer : public Timer
{
	public:
		int	run;
		InfoTimer(ns3::Timer::DestroyPolicy dp) : Timer(dp)
	{
		run = 0;
	}
};

class	ClusterTimer : public Timer
{
	public:
		int	run;
		ClusterTimer(ns3::Timer::DestroyPolicy dp) : Timer(dp)
	{
		run = 0;
	}
};

class	nMovementTimer : public Timer
{
	public:
		nMovementTimer(ns3::Timer::DestroyPolicy dp) : Timer(dp)
	{
		run = 0;
		vertical = 0;
	}
		int	run;
		int	vertical;
};

class	PacketTimer : public Timer
{
	public:
		PacketTimer(ns3::Timer::DestroyPolicy dp) : Timer(dp)
	{
		pcount = 0;
		run = 0;
	}

		void	sort()
		{
			for(int i=0;i<pcount;i++)
			{
				for(int j=0;j<pcount;j++)
				{
					if(delay[i] < delay[j])
					{
						Ptr<Packet>	tmp = p[i];
						p[i] = p[j];
						p[j] = tmp;

						double	d = delay[i];
						delay[i] = delay[j];
						delay[j] = d;
					}
				}
			}
		}

		Ptr<Packet>	p[10000];
		double		delay[10000];
		int		pcount;
		int		run;
};

enum PacketType{BEACON=1,BS_MSG,DEVICEINFO};

class	tbcm
{
	public:
		tbcm();
		nsaddr_t			index;
		Ptr <Node>			node_;

		Timer				beaconTimer;
		Timer				bsTimer;
		Timer				minitTimer;
		int				Lane;

		void				initializeBeaconTimer();
		void				initializeMovement();
		nMovementTimer			movementTimer;
static double         lastBlockTime;
static int            blockCount;
static vector<double> blockIntervals;
static int            roundAttempts;
static int consensusMode; 
static long            consensusMsgCount;
static long            consensusBytes;
static double          consensusEnergyConsumed;

static double           faultyNodePercent;
static long              faultyVoteCount;
static double            consensusThreshold;
static int                blockRoundsAttempted;
static int                blockRoundsSucceeded;
static int                useROCMode;
static double             detectionThreshold;
static long initialCollisionsDetected;   // symmetric check - same for Base and DCDT
static long reworkAttemptsTotal;         // DCDT only
static long reworkSuccessTotal;          // DCDT only

static double             sumEstLatency;
static long                latencySamples;

		static struct	device_head devicehead_;  // static head of list of devices
		inline void insert(struct device_head* head) {
			LIST_INSERT_HEAD(head, this, entry);
		}
		inline tbcm* nextdevice() { return entry.le_next; }
		LIST_ENTRY(tbcm) entry;  // declare list entry structure

		void		beaconTransmission();
		double		Random(double a,double b);
		void		ScheduleNextBeacon();

		double		get_y(Ptr<Node> mn);
		double		get_x(Ptr<Node> mn);

		double		get_x(int32_t n);
		double		get_y(int32_t n);

		double		get_speed(int32_t n);
		double		get_speed(Ptr<Node> mn);

		Ipv4Address	GetNodeAddress (nsaddr_t nid);
		Ptr<Node>	GetNodeWithAddress (Ipv4Address ipv4Address);

		void		SchedulePacketWithoutContext(Ptr<Packet> packet,void* target);
		void		SchedulePacket(tbcm *device,Ptr<Packet> packet,double delay);

		static 	tbcm* get_device_by_address(nsaddr_t);
		double		get_x();
		double		get_y();
		double		get_speed();

		double		distance(double x1,double y1,double x2,double y2);

		void		recv(Ptr<Packet> packet);
		PacketTimer	ptimer;
		void		BufferPacket(Ptr<Packet> packet,double delay);
		void		handlePacket();
		void		recvBeacon(Ptr<Packet> p);
		void*		linkLayerTarget;


		class		nodeList
		{
			public:
				nsaddr_t	nodeid[1000];
				double		x[1000];
				double		y[1000];
				double		s[1000];
				double		p[1000];
				double		ra_dist[1000];
				int		cellId[1000];
				int		count;

				nodeList()
				{
					count = 0;
				}

				void	add(nsaddr_t nid,double x_,double y_,double s_)
				{
					int pos = check(nid);
					if(pos == -1)
					{
						nodeid[count] = nid;
						x[count] = x_;
						y[count] = y_;
						s[count] = s_;
						p[count] = 0;
						count++;
					}
					else
					{
						x[pos] = x_;
						y[pos] = y_;
						s[pos] = s_;
					}
				}

				void	add(nsaddr_t nid,double x_,double y_,double s_,double p_)
				{
					int pos = check(nid);
					if(pos == -1)
					{
						nodeid[count] = nid;
						x[count] = x_;
						y[count] = y_;
						s[count] = s_;
						p[count] = p_;
						count++;
					}
					else
					{
						x[pos] = x_;
						y[pos] = y_;
						s[pos] = s_;
						p[pos] = p_;
					}
				}

				void	add_prob(nsaddr_t nid,double p_)
				{
					int pos = check(nid);
					if(pos != -1)
					{
						p[pos] = p_;
					}
				}

				int	check(nsaddr_t nid)
				{
					for(int i=0;i<count;i++)
					{
						if(nodeid[i] == nid)
							return	i;
					}
					return	-1;
				}

				void	remove(nsaddr_t nid)
				{
					int pos = check(nid);
					if(pos != -1)
					{
						for(int i=pos;i<(count-1);i++)
						{
							nodeid[i] = nodeid[i+1];
							x[i] = x[i+1];
							y[i] = y[i+1];
							s[i] = s[i+1];
							p[i] = p[i+1];
						}
						count--;
					}
				}

		}nbList_,infoTable_;

		void		updateNBRList();
		void		copy(Ptr<Packet> p,Ptr<Packet> packet);

		double		Random(double b);

		int		txCount;
		int		rxCount;

		class		myList
		{
			public:
				nsaddr_t	nodeid[1000];
				int		count;

				myList()
				{
					count = 0;
				}

				void	add(nsaddr_t nid)
				{
					int pos = check(nid);
					if(pos == -1)
					{
						nodeid[count] = nid;
						count++;
					}
				}

				int	check(nsaddr_t nid)
				{
					for(int i=0;i<count;i++)
					{
						if(nodeid[i] == nid)
							return	i;
					}
					return	-1;
				}

				void	remove(nsaddr_t nid)
				{
					int pos = check(nid);
					if(pos != -1)
					{
						for(int i=pos;i<(count-1);i++)
						{
							nodeid[i] = nodeid[i+1];
						}
						count--;
					}
				}
		};

		static	myList	list_;
		static	myList	CNlist_;
		static	myList	Jlist_;

		void		sendCHAnnouncement();
		int		status;
		int		NotConnected;
		double		getDistance(nsaddr_t nid);

		nsaddr_t	headId;

		class	rss_table
		{
			public:
				nsaddr_t nodeid[1000];
				double	rss[1000];
				double	rss2[1000];
				int	count;

				rss_table()
				{
					count = 0;
				}

				void	add(nsaddr_t nid,double r)
				{
					int pos = check(nid);
					if(pos == -1)
					{
						nodeid[count] = nid;
						rss[count] = r;
						rss2[count] = 0;
						count++;
					}
					else
					{
						rss2[pos] = rss[pos];
						rss[pos] = r;
					}
				}

				int	check(nsaddr_t nid)
				{
					for(int i=0;i<count;i++)
					{
						if(nodeid[i] == nid)
							return	i;
					}
					return	-1;
				}
		}rss_table_;

		static	int	send_count;
		static	int	recv_count;
		static	int	bytes;
		//static	double	txDelay[10000];
		//static	double	rxTime[10000];
static	vector<double>	txDelay;
static	vector<double>	rxTime;
		static	double	pinit;
		static	double	pend;
		static	int	routingOverhead;
static long truePositives;
static long falseNegatives;
static long falsePositives;
static long trueNegatives;
		double		topo_x;
		double		topo_y;
		int		BS;

		double		dest_x;
		double		dest_y;
		void		assignMovement();
		void		NodeMovement();
		void		assign_position(double x,double y);
		AnimationInterface	*anim;
		void		trace(Ptr<Packet> p,nsaddr_t src,nsaddr_t dst,double txc);
		ResultTimer	rtimer;

		void		bsMsg();
		void		recvBSMsg(Ptr<Packet> p);
		InfoTimer	itimer;
		ClusterTimer	ctimer;
		nsaddr_t	bs_id;
		double		bs_x;
		double		bs_y;

		void		sendInfo();
		double		get_energy();
		double		energy;
		double		init_energy;
		void		setEnergy(double);

		double		Eelec;
		double		Emp;
		double		Efs;
		double		e_TX(double dist,double psize);
		double		e_RX(double psize);
		void		energyTxLoss(nsaddr_t n1,nsaddr_t n2,double psize);
		void		energyRxLoss(double psize);
		void		recvDeviceInfo(Ptr<Packet> p);
		void		energyConsumption(double e);
		double		gain;
		double		Pt;
		double		freq;

		DataTimer	dataTimer;
		Ptr<Packet>	dataPacket;
		static	int	uid;
		void		initializeDataTransmission(nsaddr_t dst,Ptr<Packet> p,int packetSize,double pkt_interval,double startTime,double stopTime);
		void		dataTransmission();
		nsaddr_t	getForwarder(Ptr<Packet> p,nsaddr_t src,nsaddr_t dst);
		Ptr<Packet>	PacketList[1000];
		int		dpktcount;
		void		EnquePacket(Ptr<Packet> p);
		nsaddr_t	available_in_forwarding_route(nsaddr_t n1,nsaddr_t n2);
		int		is_nbr(nsaddr_t nbr);
		void		handleRoutingData(Ptr<Packet> p);
		double		get_dist(nsaddr_t nbr);
		nsaddr_t	check_for_multihop_forwarding(Ptr<Packet> p);
		int		min_distBS(nsaddr_t nid);
		void		assign_color(nsaddr_t nid, int cid);
		BlockChainTimer	bcTimer;
		void		BlockChain();
};
}

#endif
