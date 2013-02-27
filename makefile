CC=g++
all:
	$(CC) DemandSideMicroGrid.cpp -o DemandSideMicroGrid -g
resi:
	$(CC) DemandSideMicroGrid.cpp -o ResiDemandSideMicroGrid -g
indu:
	$(CC) DemandSideMicroGrid.cpp -o InduDemandSideMicroGrid -g
comm:
	$(CC) DemandSideMicroGrid.cpp -o CommDemandSideMicroGrid -g
