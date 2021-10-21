#include <bits/stdc++.h>
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/flow-monitor-module.h>
#include <ns3/gnuplot.h>
#include <ns3/ipv4-global-routing-helper.h>
#include <ns3/applications-module.h>
#include <ns3/internet-module.h>
using namespace std;
using namespace ns3;

double ERROR = 0.000001;

const string MAX_BYTES = "Maximum number of bytes the application can send";
const string TCP_PROTOCOL = "TCP agent type to use at H2 : TcpHighSpeed, TcpVegas, TcpScalable";
const string PACKET_SIZE = "Packet size in bytes";
const string SIMULTANEOUSLY = "Simultaneously start flows for TCP and UDP";
const string RUN_TIME = "Run time as a factor of 5s";
const string OFFSET = "Offset for different time intervlals";
const string LOOP_RUNS = "Number of for loop runs";

void flowMonCalc(map<FlowId, FlowMonitor::FlowStats>::const_iterator);
void printValuesFromCmd(uint, string);
void makeGNUPltFiles(bool, uint, string, Gnuplot2dDataset, Gnuplot2dDataset, Gnuplot2dDataset, Gnuplot2dDataset);