<div id="table-of-contents">
<h2>Table of Contents</h2>
<div id="text-table-of-contents">
<ul>
<li><a href="#org6ef7c9b">1. Porting efforts of NLSR to ndnSIM for the 3rd NDN Hackathon</a>
<ul>
<li><a href="#orgf78f419">1.1. Code rebasing</a></li>
</ul>
</li>
<li><a href="#org14f98d4">2. Getting ndnSIM with NLSR</a>
<ul>
<li><a href="#orgb3f4504">2.1. Downloading ndnSIM with NLSR source</a></li>
<li><a href="#org7d33506">2.2. Building</a></li>
<li><a href="#org9c46d88">2.3. Configuring NLSR to run on ndnSIM</a>
<ul>
<li><a href="#org0248abf">2.3.1. Generating the topology</a></li>
<li><a href="#orgabb4bb4">2.3.2. Selecting LS or HR</a></li>
</ul>
</li>
<li><a href="#org2d20f35">2.4. Running an experiment</a></li>
</ul>
</li>
</ul>
</div>
</div>

<a id="org6ef7c9b"></a>

# Porting efforts of NLSR to ndnSIM for the 3rd NDN Hackathon


<a id="orgf78f419"></a>

## Code rebasing

[Anil Jangam](https://github.com/anilj1) had made several changes to NLSR and some to ndnSIM itself, including building NLSR as a submodule under ndnSIM. As a result, he wrote several ndnSIM tools to automate config generation for NLSR experiments. As the first order of business, we rebased the code from:

-   NLSR v0.2.1 to v0.3.0
-   ndn-cxx v0.3.4 to v0.4.1
-   NFD v0.3.4 to v0.4.1


<a id="org14f98d4"></a>

# Getting ndnSIM with NLSR


<a id="orgb3f4504"></a>

## Downloading ndnSIM with NLSR source

Downloading and building ndnSIM with NLSR is not hard:

1.  `mkdir ndnSIM-NLSR`
2.  `cd ndnSIM-NLSR`
3.  `git clone https://github.com/named-data-ndnSIM/ns-3-dev.git ns-3`
4.  `git clone https://github.com/named-data-ndnSIM/pybindgen.git pybindgen`
5.  `git clone --recursive https://github.com/3rd-ndn-hackathon/ndnSIM-NLSR.git ndnSIM`


<a id="org7d33506"></a>

## Building

The instructions to build from this point onward are exactly the same as listed here: <http://ndnsim.net/2.1/getting-started.html#compiling-and-running-ndnsim>   


<a id="org9c46d88"></a>

## Configuring NLSR to run on ndnSIM

In order to use NLSR in experiments, you need topology files, and then configuration files for each node running NLSR in that topology.


<a id="org0248abf"></a>

### Generating the topology

Assuming that all paths are from the base ns-3 directory (e.g. `/home/nmg/ns-3/`):

1.  The topology files were generated using BRITE (<http://www.cs.bu.edu/brite/>). This file must describe the whole graph, including nodes and links. Example topologies are given in `src/ndnSIM/examples/ndn-nlsr-conf/` as .brite files
2.  The chosen topology file must be linked to `src/ndnSIM/examples/ndn-nlsr-conf/nlsr_router_topo.brite`
3.  The BRITE topology file must be consumed by a custom class, ndn-nlsr-confgen, like: `./waf --run=ndn-nfd-confgen`.


<a id="orgabb4bb4"></a>

### Selecting LS or HR

After ndn-nfd-confgen generates the config files (`nlsr_node_NX.conf`), it is important to note two things: 

1.  All link costs are assumed to be 25.
2.  The mode is assumed to be LS.

If running an experiment in HR mode, several changes under `hyperbolic` must be made for each node:

1.  Change state to `on`
2.  Set `radius`
3.  Set `angle` to a value between 0 and 2π (0≤θ<360 degrees)


<a id="org2d20f35"></a>

## Running an experiment

The experiments used follow the standard ndnSIM experiment format. E.g. `./waf --run=ndn-nlsr-simple`

