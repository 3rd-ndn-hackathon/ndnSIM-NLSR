#!/bin/env perl
use strict; use warnings;

if (scalar @ARGV != 2) {
    print "Usage: `perl ./nlsr-experiment.pl <topology.brite> <ls/hb>`\n";
    die;
}

my $topFile = shift(@ARGV);
my $lsOpt = shift(@ARGV);

unless (-f $topFile) {
    print "Topology file not found at: $topFile.\n";
}

unless ($lsOpt eq "ls" or $lsOpt eq "hb") {
    print "Invalid config file type!\n";
}

if (-f "examples/ndn-nlsr-conf/nlsr_router_topo.brite") {
    system("rm examples/ndn-nlsr-conf/nlsr_router_topo.brite");
}

system("ln -s $topFile examples/ndn-nlsr-conf/nlsr_router_topo.brite");
system("../../waf --run=\"ndn-nlsr-confgen -t $lsOpt\"");

system("NS_LOG=\"NdnNlsrSimple=level_all|prefix_func|prefix_time:NexthopList=level_all|prefix_func|prefix_time:RoutingTableCalculator=level_all|prefix_func|prefix_time:NamePrefixTableEntry=level_all|prefix_func|prefix_time:FaceMap=level_all|prefix_func|prefix_time:Fib=level_all|prefix_func|prefix_time:RoutingTable=level_all|prefix_func|prefix_time:NamePrefixTable=level_all|prefix_func|prefix_time:Map=level_all|prefix_func|prefix_time:FibEntry=level_all|prefix_func|prefix_time:nlsr=level_all|prefix_func|prefix_time:AdjacencyList=level_all|prefix_func|prefix_time:LsdbDatasetInterestHandler=level_all|prefix_func|prefix_time:ConfParameter=level_all|prefix_func|prefix_time:Lsdb=level_all|prefix_func|prefix_time:PrefixUpdateProcessor=level_all|prefix_func|prefix_time:SequencingManager=level_all|prefix_func|prefix_time:NamePrefixList=level_all|prefix_func|prefix_time:Lsa=level_all|prefix_func|prefix_time:SyncLogicHandler=level_all|prefix_func|prefix_time:HelloProtocol=level_all|prefix_func|prefix_time:Adjacent=level_all|prefix_func|prefix_time:ndn.AppHelper=level_all|prefix_func|prefix_time:ndn.StackHelper=level_all|prefix_func|prefix_time:ndn.FibHelper=level_all|prefix_func|prefix_time:ndn.StrategyChoiceHelper=level_all|prefix_func|prefix_time:ndn.GlobalRoutingHelper=level_all|prefix_func|prefix_time:ndn.LinkControlHelper=level_all|prefix_func|prefix_time:ndn.cs.Nocache=level_all|prefix_func|prefix_time:ndn.cs.ContentStore=level_all|prefix_func|prefix_time:ndn.L3Protocol=level_all|prefix_func|prefix_time:ndn.RttEstimator=level_all|prefix_func|prefix_time:ndn.RttMeanDeviation=level_all|prefix_func|prefix_time:L2RateTracer=level_all|prefix_func|prefix_time:ndn.L3RateTracer=level_all|prefix_func|prefix_time:ndn.CsTracer=level_all|prefix_func|prefix_time:ndn.AppDelayTracer=level_all|prefix_func|prefix_time:RocketfuelMapReader=level_all|prefix_func|prefix_time:RocketfuelWeightsReader=level_all|prefix_func|prefix_time:AnnotatedTopologyReader=level_all|prefix_func|prefix_time:ndn.Consumer=level_all|prefix_func|prefix_time:ndn.ConsumerZipfMandelbrot=level_all|prefix_func|prefix_time:ndn.ConsumerCbr=level_all|prefix_func|prefix_time:ndn.ConsumerWindow=level_all|prefix_func|prefix_time:ndn.App=level_all|prefix_func|prefix_time:ndn.Producer=level_all|prefix_func|prefix_time:ndn.ConsumerBatches=level_all|prefix_func|prefix_time:NlsrApp=level_all|prefix_func|prefix_time:NlsrExec=level_all|prefix_func|prefix_time:ndn.NlsrConfReader=level_all|prefix_func|prefix_time\" ../../waf --run=ndn-nlsr-simple");
