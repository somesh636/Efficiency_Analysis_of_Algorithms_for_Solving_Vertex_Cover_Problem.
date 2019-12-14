# Vertex_Cover_Using_Multihread
The Problem of Vertex Cover is Solved using different Algorithms by using Multithread.
There are three Algorithms running simultaneously to solve the Vertex Cover Problem.
For More detailed information on the Algorithms and results from the code kindly check the
report.pdf file.

* To Run this code follow the steps below
You need Minisat folder in your working folder to run this code. Copy the Minisat
folder and generate the library using cmake. The CMakeList.txt is present in the Minisat
folder. Then use the following command to generate the execuatable.
command is
* cmake ../
* make
* ./prj

* Sample Output
* V 5
* E {<1,4>,<4,0>,<3,1>,<3,2>,<3,0>,<4,3>,<2,1>}
* CNF-SAT-VC: 0,1,3
* APPROX-VC-1: 0,1,3
* APPROX-VC-2: 1,2,3,4
