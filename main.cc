#include <fstream>
#include <bits/stdc++.h>

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/point-to-point-module.h>
#include <ns3/flow-monitor-module.h>
#include <ns3/gnuplot.h>
#include <ns3/ipv4-global-routing-helper.h>
#include <ns3/applications-module.h>
#include <ns3/internet-module.h>

#include "main.h"

using namespace std;
using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Main");

void flowMonCalc(map<FlowId, FlowMonitor::FlowStats>::const_iterator i)
{
    cout << "Tx Packets: " << i->second.txPackets << "\n";
    cout << "Tx Bytes:" << i->second.txBytes << "\n";
    cout << "Rx Packets: " << i->second.rxPackets << "\n";
    cout << "Rx Bytes:" << i->second.rxBytes << "\n";
    cout << "Net Packet Lost: " << i->second.lostPackets << "\n";
    cout << "Lost due to droppackets: " << i->second.packetsDropped.size() << "\n";
    cout << "Total Delay(in seconds): " << i->second.delaySum.GetSeconds() << endl;
    cout << "Mean Delay(in seconds): " << (double)i->second.delaySum.GetSeconds() / (i->second.rxPackets) << endl;
    cout << "Offered Load: " << (double)i->second.txBytes * 8.0 / (i->second.timeLastTxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds()) / (double)1000 << " Kbps" << endl;
    cout << "Throughput: " << (double)i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstRxPacket.GetSeconds()) / (double)1000 << " Kbps" << endl;
    cout << "Mean jitter:" << (double)i->second.jitterSum.GetSeconds() / (i->second.rxPackets - 1) << endl;
    cout << endl;
}

void printValuesFromCmd(uint arr[], string tcpProtocol)
{
    cout << "--------------------------------------------------------" << endl;
    cout << "The command line arguments input are:" << endl;
    cout << "maxBytes: " << arr[0] << endl;
    cout << "tcpProtocol: " << tcpProtocol << endl;
    cout << "packetSize: " << arr[1] << endl;
    cout << "simultaneously: " << arr[2] << endl;
    cout << "runTime: " << arr[3] << endl;
    cout << "offset: " << arr[4] << endl;
    cout << "loopRuns: " << arr[5] << endl;
    cout << "--------------------------------------------------------" << endl
         << endl;
}

void makeGNUPltFiles(bool simultaneously, uint offset, string tcpProtocol, Gnuplot2dDataset datasetTcpThroughput,
                     Gnuplot2dDataset datasetTcpDelay, Gnuplot2dDataset datasetUdpThroughput, Gnuplot2dDataset datasetUdpDelay)
{
    string simultaneouslyFlag = "Seperate";
    if (simultaneously && offset == 0)
        simultaneouslyFlag = "Simultaneous_Same_Start";
    else if (simultaneously && offset != 0)
        simultaneouslyFlag = "Simultaneous_Different_Start";

    string fileNameWithNoExtension = "Udp_and_" + tcpProtocol + "_throughput_" + simultaneouslyFlag;
    string graphicsFileName = fileNameWithNoExtension + ".pdf";
    string plotFileName = fileNameWithNoExtension + ".plt";
    string plotTitle = tcpProtocol + " vs UDP throughput";
    string fileNameWithNoExtensionDelay = "Udp_and_" + tcpProtocol + "_delay_" + simultaneouslyFlag;
    string graphicsFileNameDelay = fileNameWithNoExtensionDelay + ".pdf";
    string plotFileNameDelay = fileNameWithNoExtensionDelay + ".plt";
    string plotTitleDelay = tcpProtocol + " vs UDP delay";

    Gnuplot plot(graphicsFileName);
    Gnuplot plot_delay(graphicsFileNameDelay);

    plot.SetTitle(plotTitle);
    plot_delay.SetTitle(plotTitleDelay);

    plot.SetTerminal("pdf");
    plot_delay.SetTerminal("pdf");

    plot.SetLegend("Packet Size(in Bytes)", "Throughput Values(in Kbps)");
    plot_delay.SetLegend("Packet Size(in Bytes)", "Delay(in s)");

    datasetTcpThroughput.SetTitle("Throughput FTP over TCP");
    datasetTcpThroughput.SetStyle(Gnuplot2dDataset::LINES_POINTS);
    datasetTcpThroughput.SetExtra("lw 2");
    datasetUdpThroughput.SetTitle("Throughput CBR over UDP");
    datasetUdpThroughput.SetStyle(Gnuplot2dDataset::LINES_POINTS);
    datasetUdpThroughput.SetExtra("lw 2");

    datasetTcpDelay.SetTitle("Delay FTP over TCP");
    datasetTcpDelay.SetStyle(Gnuplot2dDataset::LINES_POINTS);
    datasetTcpDelay.SetExtra("lw 2");
    datasetUdpDelay.SetTitle("Delay CBR over UDP");
    datasetUdpDelay.SetStyle(Gnuplot2dDataset::LINES_POINTS);
    datasetUdpDelay.SetExtra("lw 2");

    plot.AddDataset(datasetTcpThroughput);
    plot.AddDataset(datasetUdpThroughput);

    plot_delay.AddDataset(datasetUdpDelay);
    plot_delay.AddDataset(datasetTcpDelay);

    ofstream plotFile(plotFileName.c_str());

    plot.GenerateOutput(plotFile);

    plotFile.close();

    ofstream plotFile_delay(plotFileNameDelay.c_str());
    plot_delay.GenerateOutput(plotFile_delay);
    plotFile_delay.close();
}

int main(int argc, char *argv[])
{

    uint maxBytes = 0;
    string tcpProtocol = "TcpHighSpeed";
    uint packetSize = 1024;
    uint runTime = 1;
    uint offset = 0;
    bool simultaneously = false;
    uint loopRuns = 1;

    CommandLine cmd;
    cmd.AddValue("maxBytes", MAX_BYTES, maxBytes);
    cmd.AddValue("tcpProtocol", TCP_PROTOCOL, tcpProtocol);
    cmd.AddValue("packetSize", PACKET_SIZE, packetSize);
    cmd.AddValue("simultaneously", SIMULTANEOUSLY, simultaneously);
    cmd.AddValue("runTime", RUN_TIME, runTime);
    cmd.AddValue("offset", OFFSET, offset);
    cmd.AddValue("loopRuns", LOOP_RUNS, loopRuns);
    cmd.Parse(argc, argv);

    uint commandLineArray[] = {maxBytes, packetSize, simultaneously, runTime, offset, loopRuns};

    printValuesFromCmd(commandLineArray, tcpProtocol);

    if (tcpProtocol == "TcpScalable")
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpScalable::GetTypeId()));
    else if (tcpProtocol == "TcpHighSpeed")
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpHighSpeed::GetTypeId()));
    else
        Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue(TcpVegas::GetTypeId()));

    uint portNumber;

    Gnuplot2dDataset datasetUdpThroughput;
    Gnuplot2dDataset datasetTcpThroughput;
    Gnuplot2dDataset datasetUdpDelay;
    Gnuplot2dDataset datasetTcpDelay;

    for (uint i = 0; i < loopRuns; i++)
    {
        uint udpPacketSize, tcpPacketSize;
        udpPacketSize = tcpPacketSize = packetSize + 100 * i;

        NodeContainer c;
        c.Create(6);
        NodeContainer n0n2 = NodeContainer(c.Get(0), c.Get(2));
        NodeContainer n1n2 = NodeContainer(c.Get(1), c.Get(2));
        NodeContainer n2n3 = NodeContainer(c.Get(2), c.Get(3));
        NodeContainer n3n4 = NodeContainer(c.Get(3), c.Get(4));
        NodeContainer n3n5 = NodeContainer(c.Get(3), c.Get(5));

        InternetStackHelper internet;
        internet.Install(c);

        uint queueSizeHR = (80000 * 20) / (tcpPacketSize * 8);
        uint queueSizeRR = (30000 * 100) / (tcpPacketSize * 8);
        string queueSizeHR2 = to_string(queueSizeHR) + "p";
        string queueSizeRR2 = to_string(queueSizeRR) + "p";

        PointToPointHelper point2Point;

        point2Point.SetDeviceAttribute("DataRate", StringValue("80Mbps"));
        point2Point.SetChannelAttribute("Delay", StringValue("20ms"));
        point2Point.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue(QueueSize(queueSizeHR2)));

        NetDeviceContainer d0d2 = point2Point.Install(n0n2);
        NetDeviceContainer d1d2 = point2Point.Install(n1n2);
        NetDeviceContainer d3d4 = point2Point.Install(n3n4);
        NetDeviceContainer d3d5 = point2Point.Install(n3n5);

        point2Point.SetDeviceAttribute("DataRate", StringValue("30Mbps"));
        point2Point.SetChannelAttribute("Delay", StringValue("100ms"));
        point2Point.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue(QueueSize(queueSizeRR2)));

        NetDeviceContainer d2d3 = point2Point.Install(n2n3);

        Ptr<RateErrorModel> em = CreateObject<RateErrorModel>();
        em->SetAttribute("ErrorRate", DoubleValue(ERROR));
        d3d4.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));
        d3d5.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

        Ipv4AddressHelper ipv4;
        ipv4.SetBase("10.1.0.0", "255.255.255.0");
        Ipv4InterfaceContainer i0i2 = ipv4.Assign(d0d2);

        ipv4.SetBase("10.1.1.0", "255.255.255.0");
        Ipv4InterfaceContainer i1i2 = ipv4.Assign(d1d2);

        ipv4.SetBase("10.1.3.0", "255.255.255.0");
        Ipv4InterfaceContainer i2i3 = ipv4.Assign(d2d3);

        ipv4.SetBase("10.1.4.0", "255.255.255.0");
        Ipv4InterfaceContainer i3i4 = ipv4.Assign(d3d4);

        ipv4.SetBase("10.1.5.0", "255.255.255.0");
        Ipv4InterfaceContainer i3i5 = ipv4.Assign(d3d5);

        Ipv4GlobalRoutingHelper::PopulateRoutingTables();

        cout << "The IP addresses assigned to Senders:" << endl;
        cout << "H1: " << i0i2.GetAddress(0) << endl;
        cout << "H2: " << i1i2.GetAddress(0) << endl;

        cout << "The IP addresses assigned to Recievers:" << endl;
        cout << "H3: " << i3i4.GetAddress(1) << endl;
        cout << "H4: " << i3i5.GetAddress(1) << endl;

        cout << "The IP addresses assigned to Routers:" << endl;
        cout << "R1<--H1: " << i0i2.GetAddress(1) << endl;
        cout << "R1<--H2: " << i1i2.GetAddress(1) << endl;
        cout << "R2<--H3: " << i3i4.GetAddress(0) << endl;
        cout << "R2<--H4: " << i3i5.GetAddress(0) << endl;
        cout << "R1<--R2: " << i2i3.GetAddress(0) << endl;
        cout << "R1-->R2: " << i2i3.GetAddress(1) << endl;
        cout << endl;

        Ipv4GlobalRoutingHelper g;
        Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("routing.routes", ios::out);
        g.PrintRoutingTableAllAt(Seconds(2), routingStream);

        portNumber = 9;

        OnOffHelper onoff("ns3::UdpSocketFactory", Address(InetSocketAddress(i3i4.GetAddress(1), portNumber)));
        onoff.SetAttribute("PacketSize", UintegerValue(udpPacketSize));

        ApplicationContainer udpAppsSource = onoff.Install(n0n2.Get(0));

        if (simultaneously == false)
        {
            udpAppsSource.Start(Seconds((0.0 + (10 * i)) * runTime));
            udpAppsSource.Stop(Seconds((5.0 + (10 * i)) * runTime));
        }
        else
        {
            udpAppsSource.Start(Seconds((0.0 + (10 * i)) * runTime));
            udpAppsSource.Stop(Seconds((10.0 + (10 * i)) * runTime));
        }

        PacketSinkHelper sinkUdp("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address::GetAny(), portNumber)));

        ApplicationContainer udpAppsDest = sinkUdp.Install(n3n4.Get(1));

        if (simultaneously == false)
        {
            udpAppsDest.Start(Seconds((0.0 + (10 * i)) * runTime));
            udpAppsDest.Stop(Seconds((5.0 + (10 * i)) * runTime));
        }
        else
        {
            udpAppsDest.Start(Seconds((0.0 + (10 * i)) * runTime));
            udpAppsDest.Stop(Seconds((10.0 + (10 * i)) * runTime));
        }

        portNumber = 13356;
        BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(i3i5.GetAddress(1), portNumber));

        source.SetAttribute("MaxBytes", UintegerValue(maxBytes));
        source.SetAttribute("SendSize", UintegerValue(tcpPacketSize));
        ApplicationContainer tcpAppsSource = source.Install(n1n2.Get(0));

        if (simultaneously == false)
        {
            tcpAppsSource.Start(Seconds((5.0 + (10 * i)) * runTime));
            tcpAppsSource.Stop(Seconds((10.0 + (10 * i)) * runTime));
        }
        else
        {
            tcpAppsSource.Start(Seconds((0.0 + (10 * i)) * runTime));
            tcpAppsSource.Stop(Seconds((10.0 + (10 * i)) * runTime));
        }

        PacketSinkHelper sink_tcp("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), portNumber));
        ApplicationContainer tcpAppsDest = sink_tcp.Install(n3n5.Get(1));

        if (simultaneously == false)
        {
            tcpAppsDest.Start(Seconds((5.0 + (10 * i)) * runTime));
            tcpAppsDest.Stop(Seconds((10.0 + (10 * i)) * runTime));
        }
        else
        {
            tcpAppsDest.Start(Seconds((0.0 + (10 * i)) * runTime + offset));
            tcpAppsDest.Stop(Seconds((10.0 + (10 * i)) * runTime + offset));
        }

        Ptr<FlowMonitor> flowMon;
        FlowMonitorHelper flowMonHelper;
        flowMon = flowMonHelper.InstallAll();
        if (!simultaneously)
            Simulator::Stop(Seconds((10 + (10 * i)) * runTime));
        else
            Simulator::Stop(Seconds((10 + (10 * i)) * runTime + offset));
        Simulator::Run();
        flowMon->CheckForLostPackets();

        Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowMonHelper.GetClassifier());
        map<FlowId, FlowMonitor::FlowStats> stats = flowMon->GetFlowStats();

        double throughput_udp;
        double throughput_tcp;
        double delay_udp;
        double delay_tcp;

        for (map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin(); i != stats.end(); ++i)
        {
            Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow(i->first);
            cout << "Flow: " << i->first << "\n";
            cout << "Source: " << t.sourceAddress << "\n";
            cout << "Destination: " << t.destinationAddress << "\n";

            if (t.sourceAddress == "10.1.0.1")
            {
                throughput_udp = (double)i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstRxPacket.GetSeconds()) / (double)1000;
                delay_udp = (double)i->second.delaySum.GetSeconds() / (i->second.rxPackets);

                datasetUdpThroughput.Add(udpPacketSize, throughput_udp);
                datasetUdpDelay.Add(udpPacketSize, delay_udp);

                cout << "UDP Flow over CBR " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
                flowMonCalc(i);
            }
            else if (t.sourceAddress == "10.1.1.1")
            {
                throughput_tcp = (double)i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstRxPacket.GetSeconds()) / 1000;
                delay_tcp = (double)i->second.delaySum.GetSeconds() / (i->second.rxPackets);

                datasetTcpThroughput.Add(tcpPacketSize, throughput_tcp);
                datasetTcpDelay.Add(tcpPacketSize, delay_tcp);

                cout << tcpProtocol << " Flow over FTP " << i->first << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
                flowMonCalc(i);
            }
        }

        cout << "Finished Loop Run Number: " << i << endl
             << endl;
        cout << "--------------------------------------------------------" << endl
             << endl;
        Simulator::Destroy();
    }
    makeGNUPltFiles(simultaneously, offset, tcpProtocol, datasetTcpThroughput, datasetTcpDelay, datasetUdpThroughput, datasetUdpDelay);
    return 0;
}