<div id="table-of-contents">
<h2>Table of Contents</h2>
<div id="text-table-of-contents">
<ul>
<li><a href="#orga1a7afb">1. Porting efforts of NLSR to ndnSIM for the 3rd NDN Hackathon</a>
<ul>
<li><a href="#orgbeb84fd">1.1. Code rebasing</a></li>
</ul>
</li>
<li><a href="#orgea224d3">2. Getting ndnSIM with NLSR</a>
<ul>
<li><a href="#org2f35b0a">2.1. Downloading ndnSIM with NLSR source</a></li>
<li><a href="#org9351d8a">2.2. Building</a></li>
<li><a href="#orgb231061">2.3. Configuring NLSR to run on ndnSIM</a>
<ul>
<li><a href="#orga037a71">2.3.1. Generating the topology</a></li>
<li><a href="#org50da3b9">2.3.2. Selecting LS or HR</a></li>
<li><a href="#orgcf10299">2.3.3. Convenient script</a></li>
</ul>
</li>
<li><a href="#org02bf806">2.4. Running an experiment</a></li>
</ul>
</li>
</ul>
</div>
</div>

<a id="orga1a7afb"></a>

# Porting efforts of NLSR to ndnSIM for the 3rd NDN Hackathon


<a id="orgbeb84fd"></a>

## Code rebasing

[Anil Jangam](https://github.com/anilj1) had made several changes to NLSR and some to ndnSIM itself, including building NLSR as a submodule under ndnSIM. As a result, he wrote several ndnSIM tools to automate config generation for NLSR experiments. As the first order of business, we rebased the code from:

-   NLSR v0.2.1 to v0.3.0
-   ndn-cxx v0.3.4 to v0.4.1
-   NFD v0.3.4 to v0.4.1


<a id="orgea224d3"></a>

# Getting ndnSIM with NLSR


<a id="org2f35b0a"></a>

## Downloading ndnSIM with NLSR source

Downloading and building ndnSIM with NLSR is not hard:

1.  `mkdir ndnSIM-NLSR`
2.  `cd ndnSIM-NLSR`
3.  `git clone https://github.com/named-data-ndnSIM/ns-3-dev.git ns-3`
4.  `git clone https://github.com/named-data-ndnSIM/pybindgen.git pybindgen`
5.  `git clone --recursive https://github.com/3rd-ndn-hackathon/ndnSIM-NLSR.git ns-3/src/ndnSIM`


<a id="org9351d8a"></a>

## Building

The instructions to build from this point onward are exactly the same as listed here: <http://ndnsim.net/2.1/getting-started.html#compiling-and-running-ndnsim>   


<a id="orgb231061"></a>

## Configuring NLSR to run on ndnSIM

In order to use NLSR in experiments, you need topology files, and then configuration files for each node running NLSR in that topology.


<a id="orga037a71"></a>

### Generating the topology

Assuming that all paths are from the base ns-3 directory (e.g. `/home/nmg/ns-3/`):

1.  The topology files were generated using BRITE (<http://www.cs.bu.edu/brite/>). This file must describe the whole graph, including nodes and links. Example topologies are given in `src/ndnSIM/examples/ndn-nlsr-conf/` as .brite files
2.  The chosen topology file must be linked to `src/ndnSIM/examples/ndn-nlsr-conf/nlsr_router_topo.brite`
3.  The BRITE topology file must be consumed by a custom class, ndn-nlsr-confgen, like: `./waf --run=ndn-nfd-confgen`.
    1.  To generate configs for LS mode, use `./waf --run=​"ndn-nlsr-confgen -t ls"`
    2.  For HR mode, use `./waf --run=​"ndn-nlsr-confgen -t hb"`


<a id="org50da3b9"></a>

### Selecting LS or HR

ndn-nlsr-confgen does not know what values you want to set. In LS mode the link costs are assumed to be 25.
If the configs were generated for HR mode, the defaults are:

1.  `radius = 100+nodeId`
2.  `angle = nodeId`

E.g. if `nodeId = 1`, then `radius = 101`, `angle = 1`.


<a id="orgcf10299"></a>

### Convenient script

For your convenience, a perl script has been written that wraps these commands into a simple format. From within the ndnSIM directory:

`perl nlsr-experiment.pl <absolute-path-to-topology-file> <hr/ls>`


<a id="org02bf806"></a>

## Running an experiment

The experiments used follow the standard ndnSIM experiment format. E.g. `./waf --run=ndn-nlsr-simple`
To enable logging for your experiment, follow the standard ndnSIM procedure: `NS_​LOG=​...`

