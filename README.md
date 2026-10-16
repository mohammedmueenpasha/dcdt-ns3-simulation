# DCDT NS-3 Simulation

NS-3 simulation code for the paper: *A Scalable Tambola Consensus Mechanism for Blockchain Networks with GAN-Entropy Reinforced Bias Correction*
Repository contents
- `project.cc`: main simulation script
- `ns3/`: C++ header (`.h`) and source (`.cc`) files implementing the consensus mechanism and supporting classes

 Requirements
- NS-3 version: **3.30**
- Language: C++ (C++11 or later)
- OS: Ubuntu [18.04 / 20.04]
- Compiler: g++ [version]
- Python 3 (required by waf)

 Setup
1. Download NS-3.30 from https://www.nsnam.org/releases/ns-3-30/ and extract it.
2. Copy `project.cc` from this repository into the `scratch/` folder of your NS-3.30 installation.
3. C++ header (`.h`) and source (`.cc`) files implementing the consensus mechanism and supporting classes
4. Configure and build from the NS-3.30 root folder:

       ./waf configure --enable-examples
       ./waf build

 Running the simulation
From the NS-3.30 root folder:

    ./waf --run scratch/project

With parameters:

    ./waf --run "scratch/project --[param1]=[value] --[param2]=[value]"


 Outputs
- [output file or console output]: reproduces [Figure X / Table Y] in the paper

 License
Released under the GNU GPL v2 License, consistent with NS-3. See `LICENSE`.

 Citation
If you use this code, please cite: [paper citation or DOI]
